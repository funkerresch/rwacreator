#include "rwaassetlist.h"



RwaAssetList::RwaAssetList(RwaBackend *backend, QWidget *parent) :
    QListWidget(parent)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    this->backend = backend;

    connect(backend, SIGNAL(sendAddTts2CurrentState(QString)),
              this, SLOT(addTts2CurrentState(QString)));
}

void RwaAssetList::mousePressEvent(QMouseEvent *event)
{
    qint32 index = 0;
    QListWidget::mousePressEvent(event);
    //qDebug() << currentItem()->text();
    index = currentIndex().row();
    qDebug() << index;

}

void RwaAssetList::keyPressEvent(QKeyEvent *event)
{
    QListWidget::keyPressEvent(event);
    switch (event->key())
      {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          qDebug() << "Enter";
          break;
        case Qt::Key_Escape:
          qDebug() << "Escape";
          break;
        case Qt::Key_Insert:
          qDebug() << "Insert";
          break;
        case 16777219:
          //qDebug() << currentItem()->text();
          emit deleteAsset(currentItem()->text());
          takeItem(getSelectedIndex());

          break;
          default:
          qDebug() << "other" << event->key();
          break;
      }
}

qint32 RwaAssetList::getSelectedIndex()
{
    if(selectedIndexes().isEmpty())
        return -1;
    else
        return this->selectedIndexes().first().row();
}

void RwaAssetList::mouseMoveEvent(QMouseEvent *event)
{

    //QListWidget::mouseMoveEvent(event);
}

QStringList RwaAssetList::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

void RwaAssetList::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug("dragEnterEvent");

    event->acceptProposedAction();

}

void RwaAssetList::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug("dragMoveEvent");
    //event->acceptProposedAction();
}

void RwaAssetList::add2ListAndCopy(QString fullpath)
{
    bool fileExists = false;
    QString strippedPath;
    QString assetPath = backend->getCompleteAssetPath();
    QFileInfo info(fullpath);

    QStringList list = fullpath.split("//");
    strippedPath = list.last();

    QString fullAssetPath(QString("%1/%2").arg(assetPath).arg(RwaUtilities::getFileName(fullpath)));
    QFileInfo assetInfo(fullAssetPath);
    fileExists = assetInfo.exists();

    if(!info.completeSuffix().compare("pd") )
    {
        addItem(RwaUtilities::getFileName(fullAssetPath));
        if(!fileExists)
            QFile::copy(strippedPath, fullAssetPath);

        emit newAsset(fullAssetPath, RWAASSETTYPE_PD);
    }

    else if(!info.completeSuffix().compare("wav") )
    {
        addItem(RwaUtilities::getFileName(fullAssetPath));
        if(!fileExists)
            QFile::copy(strippedPath, fullAssetPath);
        emit newAsset(fullAssetPath, RWAASSETTYPE_WAV);
        qDebug("ASSETLIST: dropEvent(); wav");
    }
    else if(!info.completeSuffix().compare("aif") )
    {
        qDebug("ASSETLIST: dropEvent(); aif");
        addItem(RwaUtilities::getFileName(fullAssetPath));
        if(!fileExists)
            QFile::copy(strippedPath, fullAssetPath);
        emit newAsset(fullAssetPath, RWAASSETTYPE_AIF);
    }

    else
    {
        ;

    }

}

void RwaAssetList::addTts2CurrentState(QString fullpath)
{
     qDebug("add to asset list");
     add2ListAndCopy(fullpath);
}

void RwaAssetList::dropEvent(QDropEvent *event)
{


    QString str = event->mimeData()->text();
    QStringList newItems = str.split( "\n", QString::SkipEmptyParts );

    foreach( const QString fullpath, newItems )
        add2ListAndCopy(fullpath);

    event->acceptProposedAction();

    if(this->count() > 0)
        setCurrentRow(this->count()-1);

}
