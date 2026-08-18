#include "rwahistory.h"
#include "QtGui/qevent.h"
#include <QDir>

RwaHistory::RwaHistory(QWidget *parent):
    QListView(parent)
{
    backend = RwaBackend::getInstance();

    // One model for the lifetime of the view; only its root path changes per game.
    listModel = new QFileSystemModel(this);
    listModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    listModel->setNameFilters(QStringList() << "*.rwa");
    listModel->setNameFilterDisables(false); // hide non-matching entries instead of greying them out
    connect (listModel, SIGNAL(rowsInserted(const QModelIndex &, int , int )), this, SLOT(update()));
    setSelectionMode(SelectionMode::SingleSelection);

    connect (backend, SIGNAL(newGameLoaded()),
             this, SLOT(receiveNewGameSignal()));
    connect (backend, SIGNAL(projectPathsChanged()),
             this, SLOT(receiveNewGameSignal()));

    connect (this, SIGNAL(readUndoFile(QString)),
             backend, SLOT(receiveReadUndoFile(QString)));
}

void RwaHistory::receiveNewGameSignal()
{
    const QString undoPath = backend->completeUndoPath;
    QDir undoDir(undoPath);

    // QFileSystemModel::setRootPath() falls back to "." (the working directory)
    // for a nonexistent directory and to the drives list for an empty one.
    // Neither is a place a click may load a project from, so refuse to show
    // anything unless the undo folder really exists.
    if (undoPath.isEmpty() || !undoDir.isAbsolute() || (!undoDir.exists() && !undoDir.mkpath(".")))
    {
        qWarning() << "History View: no usable undo folder" << undoPath << "- undo history disabled";
        setModel(nullptr);
        return;
    }

    setModel(listModel);
    setRootIndex(listModel->setRootPath(undoDir.absolutePath()));
    setSelectionMode(SelectionMode::SingleSelection);
}

void RwaHistory::loadCurrentEntry()
{
    if (model() != listModel)
        return;

    QModelIndex index = currentIndex();
    if (!index.isValid() || index.parent() != rootIndex())
        return;

    QFileInfo info = listModel->fileInfo(index);
    if (!info.isFile() || info.suffix() != "rwa")
        return;

    emit readUndoFile(info.fileName());
}

void RwaHistory::mousePressEvent(QMouseEvent *event)
{
    QListView::mousePressEvent(event);
    loadCurrentEntry();
}

void RwaHistory::update()
{
    if (model() != listModel)
        return;

    QModelIndex parentIndex = rootIndex();
    int32_t count = listModel->rowCount(parentIndex);
    if (count <= 0)
        return;

    // Newest snapshot becomes current and selected, so that Up/Down step back from it.
    QModelIndex indexOfTheCellIWant = listModel->index(count-1, 0, parentIndex);
    selectionModel()->setCurrentIndex(indexOfTheCellIWant, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
    scrollToBottom();
}

void RwaHistory::keyPressEvent(QKeyEvent *event)
{
    QListView::keyPressEvent(event);
    switch (event->key())
    {
        case Qt::Key_Up:
        case Qt::Key_Down:
            loadCurrentEntry();
            break;
        default:
            break;
    }
}
