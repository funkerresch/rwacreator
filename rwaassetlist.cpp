#include "rwaassetlist.h"

RwaAssetList::RwaAssetList(QWidget* parent, RwaScene *scene) :
    RwaListView(parent, scene)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, SIGNAL(sendSelectedAssets(QStringList)),
              backend, SLOT(receiveSelectedAssets(QStringList)));

    connect(this, SIGNAL(sendCurrentAsset(RwaAsset1 *)),
              backend, SLOT(receiveLastTouchedAsset(RwaAsset1 *)));
}

void RwaAssetList::ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    (void) hint;
    QString newName = reinterpret_cast<QLineEdit*>(editor)->text();
    if(currentAsset->objectName() != newName.toStdString())
    {
        QString path = backend->completeAssetPath;
        QString newFullPath = path+"/"+newName;
        QStringList tmp = QString::fromStdString(currentAsset->getFileName()).split(".");
        QString newEnd = tmp.last();
        tmp = newFullPath.split(".");
        QString end = tmp.last();

        if(end != newEnd)
            currentItem()->setText(QString::fromStdString(currentAsset->getFileName()));
        else
        {
            QFile::rename(QString::fromStdString(currentAsset->getFullPath()), newFullPath);
            currentAsset->setObjectName(newName.toStdString());
            currentAsset->setFullPath(newFullPath.toStdString());
            currentAsset->setFileName(newName.toStdString());

            emit sendCurrentState(currentState);
        }
    }
}

void RwaAssetList::setCurrentState(RwaState *state)
{
    if(!state)
        return;

    currentState = state;

    clear();

    foreach(RwaAsset1 *asset , state->getAssets() )
    {
        QListWidgetItem *item = new QListWidgetItem(RwaUtilities::getFileName(QString::fromStdString(asset->getFullPath())), this);
        item->setFlags( item->flags() | Qt::ItemIsEditable );
        addItem(item);
    }

    setCurrentAsset(state->getLastTouchedAsset());
}

void RwaAssetList::setCurrentScene(RwaScene *scene)
{
    if(!scene)
        return;

    currentScene = scene;

    if(currentScene->lastTouchedState)
        setCurrentState(currentScene->lastTouchedState);
    else
        setCurrentState(currentScene->getStates().front());
}

int RwaAssetList::getNumberOfSelectedAssets()
{
    QList<QListWidgetItem *> items = this->selectedItems();
    return items.count();
}

QStringList RwaAssetList::getSelectedAssets()
{
    QList<QListWidgetItem *> items = this->selectedItems();
    QStringList assets;
    foreach(QListWidgetItem *item, items)
        assets << item->text();

    return assets;
}

void RwaAssetList::setCurrentAsset(RwaAsset1 *asset)
{
    if(!asset)
        return;

    if(!(QObject::sender() == this->backend))
        emit sendCurrentAsset(asset);

    else
    {
        QList<QListWidgetItem *> items = findItems(QString::fromStdString(asset->getFileName()), Qt::MatchExactly);

        if(!items.empty())
        {
            currentAsset = asset;
            setCurrentItem(items.at(0));
            int row = QListWidget::row(currentItem());
            setCurrentRow(row);
            emit sendSelectedAssets( getSelectedAssets() );
        }
    }
}

void RwaAssetList::setCurrentAssetFromCurrentItem()
{
    if(currentItem())
    {
        RwaAsset1 *asset = currentState->getAsset(currentItem()->text().toStdString());
        setCurrentAsset(asset);
    }
}

void RwaAssetList::mouseReleaseEvent(QMouseEvent *event)
{
    QListWidget::mouseReleaseEvent(event);
    if(currentState)
        emit sendSelectedAssets( getSelectedAssets() );
}

void RwaAssetList::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);
    setCurrentAssetFromCurrentItem();
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
          qDebug() << "ITEM TEXT" << currentItem()->text();
          emit deleteAsset(currentItem()->text());
          takeItem(getSelectedIndex());
          break;
        case 16777237:
            setCurrentAssetFromCurrentItem();
            break;
        case 16777235:
            setCurrentAssetFromCurrentItem();
            break;
        default:
          ;//qDebug() << "other" << event->key();
          break;
    }
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
    QString assetPath = backend->completeAssetPath;
    QFileInfo info(fullpath);

    QStringList list = fullpath.split("//");
    strippedPath = list.last();

    QString fullAssetPath(QString("%1/%2").arg(assetPath).arg(RwaUtilities::getFileName(fullpath)));
    QFileInfo assetInfo(fullAssetPath);
    fileExists = assetInfo.exists();

    if(!info.completeSuffix().compare("pd") )
    {
        QListWidgetItem *item = new QListWidgetItem(RwaUtilities::getFileName(fullAssetPath), this);
        item->setFlags( item->flags() | Qt::ItemIsEditable );
        addItem(item);
        if(!fileExists)
            QFile::copy(strippedPath, fullAssetPath);

        emit newAsset(fullAssetPath, RWAASSETTYPE_PD);
    }

    else if(!info.completeSuffix().compare("wav") )
    {
        QListWidgetItem *item = new QListWidgetItem(RwaUtilities::getFileName(fullAssetPath), this);
        item->setFlags( item->flags() | Qt::ItemIsEditable );
        addItem(item);
        if(!fileExists)
            QFile::copy(strippedPath, fullAssetPath);
        emit newAsset(fullAssetPath, RWAASSETTYPE_WAV);
        qDebug("ASSETLIST: dropEvent(); wav");
    }
    else if(!info.completeSuffix().compare("aif") )
    {
        QListWidgetItem *item = new QListWidgetItem(RwaUtilities::getFileName(fullAssetPath), this);
        item->setFlags( item->flags() | Qt::ItemIsEditable );
        addItem(item);
        if(!fileExists)
            QFile::copy(strippedPath, fullAssetPath);
        emit newAsset(fullAssetPath, RWAASSETTYPE_AIF);
    }

    else
    {      

    }
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
