#include "rwahistory.h"


RwaHistory::RwaHistory(QWidget *parent):
    QListView(parent)
{
    backend = RwaBackend::getInstance();

    connect (backend, SIGNAL(newGameLoaded()),
             this, SLOT(receiveNewGameSignal()));

    connect (this, SIGNAL(readUndoFile(QString)),
             backend, SLOT(receiveReadUndoFile(QString)));
}

void RwaHistory::receiveNewGameSignal()
{
    listModel = new QFileSystemModel(this);
    listModel->setRootPath(QString(backend->completeUndoPath));
    this->setModel(listModel);
    this->setRootIndex(listModel->setRootPath(backend->completeUndoPath));
    connect (listModel, SIGNAL(rowsInserted(const QModelIndex &, int , int )), this, SLOT(update()));
    setSelectionMode(SelectionMode::SingleSelection);
}

void RwaHistory::mousePressEvent(QMouseEvent *event)
{
    QListView::mousePressEvent(event);
    emit readUndoFile(this->currentIndex().data( Qt::DisplayRole ).toString());
}

void RwaHistory::update()
{
    QDir dir( backend->completeUndoPath );
    dir.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot );
    int32_t count = dir.count(); // rowcount of model is always 1, no matter what..

    QModelIndex parentIndex = listModel->index(backend->completeUndoPath);
    QModelIndex indexOfTheCellIWant = listModel->index(count-1, 0, parentIndex);
    selectionModel()->select(indexOfTheCellIWant, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
    scrollToBottom();
}

void RwaHistory::keyPressEvent(QKeyEvent *event)
{
    QListView::keyPressEvent(event);
    switch (event->key())
    {
        case Qt::Key_Up:
            emit readUndoFile(this->currentIndex().data( Qt::DisplayRole ).toString());
            break;
        case Qt::Key_Down:
            emit readUndoFile(this->currentIndex().data( Qt::DisplayRole ).toString());
            break;
        default:
            break;
    }
}
