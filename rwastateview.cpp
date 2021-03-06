#include "rwastateview.h"
#include <QScrollArea>

RwaStateView::RwaStateView(QWidget* parent, RwaScene *scene, QString name)
: RwaGraphicsView(parent, scene, name)
{
    setAcceptDrops(true);
    setAlignment(Qt::AlignTop);
    windowSplitter = new QSplitter(this);
    connect(windowSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(handleSplitter(int, int)));
    layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    layout->setContentsMargins(0,0,0,0);
    assetList = new RwaAssetList(this, scene);
    assetAttributes = new RwaAssetAttributeView(this, scene);
    assetAttributes->scrollArea->setMaximumWidth(250);
    currentPoint = nullptr;
    editStateRadius = false;
    editStateHeight = false;
    editStateWidth = false;
    editStatePosition = false;
    stateRadiusVisible = true;
    assetReflectionsVisible = true;
    assetsVisible = true;
    onlyAssetsOfCurrentStateVisible = true;

    connect(backend, SIGNAL(sendMoveCurrentState1(double, double)),
              this, SLOT(movePixmapsOfCurrentState(double,double)));

    connect(backend, SIGNAL(sendMoveCurrentScene()),
              this, SLOT(moveCurrentState()));

    connect(this, SIGNAL(sendMoveCurrentState1(double, double)),
              backend, SLOT(receiveMoveCurrentState1(double,double)));

    connect(backend, SIGNAL(sendMoveCurrentAsset1(double, double)),
              this, SLOT(movePixmapsOfCurrentAsset(double,double)));

    connect(this, SIGNAL(sendMoveCurrentAsset1(double, double)),
              backend, SLOT(receiveMoveCurrentAsset1(double,double)));

    connect(this, SIGNAL(sendMoveCurrentAssetChannel(double, double, int)),
              backend, SLOT(receiveMoveCurrentAssetChannel(double,double, int)));

    connect(this, SIGNAL(sendMoveCurrentAssetReflection(double, double, int)),
              backend, SLOT(receiveMoveCurrentAssetReflection(double,double, int)));

    connect(backend, SIGNAL(sendStatePosition(QPointF)),
              this, SLOT(setMapCoordinates(QPointF)));

    connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
              this, SLOT(receiveMouseMoveEvent(const QMouseEvent*, const QPointF)));

    connect(mc, SIGNAL(sendMouseDownEvent(const QMouseEvent*, const QPointF)),
              this, SLOT(receiveMouseDownEvent(const QMouseEvent*, const QPointF)));

    connect(mc, SIGNAL(sendMouseReleaseEvent()),
              this, SLOT(receiveMouseReleaseEvent()));

    connect(assetList, SIGNAL(newAsset(const QString &, qint32)),
              this, SLOT(addAssetItem(const QString &, qint32)));

    connect(assetList, SIGNAL(deleteAsset(const QString &)),
              this, SLOT(deleteAssetItem(const QString &)));

    connect(this, SIGNAL(sendCurrentAsset(RwaAsset1*)),
            backend, SLOT(receiveLastTouchedAsset(RwaAsset1*)));

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
            backend, SLOT(receiveLastTouchedState(RwaState*)));

    connect(this, SIGNAL(sendCurrentStateRadiusEdited()),
            backend, SLOT(receiveCurrentStateRadiusEdited()));

    connect(backend, SIGNAL(sendCurrentStateRadiusEdited()),
              this, SLOT(receiveUpdateCurrentStateRadius()));

    addZoomButtons();
    windowSplitter->addWidget(assetAttributes->scrollArea);
    windowSplitter->addWidget(assetList);
    windowSplitter->addWidget(mc);
    layout->addWidget(windowSplitter);

    readSplitterLayout();

    QTimer::singleShot(1000, this, SLOT(correctMapView()));
}

void RwaStateView::keyPressEvent(QKeyEvent *event)
{
    QGraphicsView::keyPressEvent(event);
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          //qDebug() << "Enter";
          break;
        case Qt::Key_Escape:
          //qDebug() << "Escape";
          break;
        case Qt::Key_Insert:
          //qDebug() << "Insert";
          break;

          default:
        //qDebug() << "RwaStateView::keyPressEvent" << event->key();
          break;
     }
}

void RwaStateView::correctMapView()
{
     mc->resize(QSize(windowSplitter->sizes().at(2), this->height()));
}

void RwaStateView::receiveUpdateCurrentStateRadius()
{
    redrawRadiusOfCurrentState();
}

void RwaStateView::zoomIn()
{
    mc->zoomIn();
    if(currentState)
    {
        currentState->setZoom( mc->currentZoom());
        if(stateRadiusVisible)
            redrawStateRadii();
    }
}

void RwaStateView::zoomOut()
{
    mc->zoomOut();
    if(currentState)
    {
        currentState->setZoom( mc->currentZoom());
        if(stateRadiusVisible)
            redrawStateRadii();
    }
}

void RwaStateView::setZoomLevel(qint32 zoomLevel)
{
    if(!currentState)
        return;

    currentState->setZoom(zoomLevel);
    mc->setZoom(zoomLevel);
    if(stateRadiusVisible)
        redrawStateRadii();
}

void RwaStateView::setCurrentAsset(RwaAsset1 *asset)
{
    if(!asset)
        return;

    RwaGraphicsView::setCurrentAsset(asset);

//    currentAsset = asset;
//    currentState->setLastTouchedAsset(this->currentAsset);
//    updateAssetPixmaps();
//    updateReflectionPixmaps();
//    if(!(QObject::sender() == backend))
//        emit sendCurrentAsset(currentAsset);

}

void RwaStateView::redrawStateRadii()
{
    stateRadiusLayer->clearGeometries();
    RwaState *state;
    foreach (state, currentScene->getStates())
    {
        if(state->isGpsState && (state == currentState) )
             drawArea(state, stateRadiusLayer);
    }
}

void RwaStateView::setMapCoordinates(double lon, double lat)
{
    mc->setView(QPointF(lon,lat));
}

void RwaStateView::setMapCoordinates(QPointF coordinates)
{
    mc->setView(coordinates);
}

void RwaStateView::adaptSize(qint32 width, qint32 height)
{
    mc->resize(QSize(windowSplitter->sizes().at(2), this->height()));
//    QSettings settings("Intrinsic Audio", "Rwa Creator");
//    settings.setValue("stateViewGeometry", windowSplitter->saveGeometry());
//    settings.setValue("stateViewState", windowSplitter->saveState());
}

void RwaStateView::handleSplitter(int pos, int index)
{
     mc->resize(QSize(windowSplitter->sizes().at(2), this->height()));
//    QSettings settings("Intrinsic Audio", "Rwa Creator");
//    settings.setValue("stateViewGeometry", windowSplitter->saveGeometry());
//    settings.setValue("stateViewState", windowSplitter->saveState());
}

void RwaStateView::receiveMouseMoveEvent(const QMouseEvent*, const QPointF myPoint)
{
    if(!currentState)
        return;

    RwaMapItem *geo = (RwaMapItem *)currentMapItem;
    QPointF lastCoordinate;
    double dx, dy;
    std::vector<double> tmp(2, 0.0);

    RwaUtilities::copyLocationCoordinates2Clipboard(myPoint);
    if(backend->logCoordinates)
        RwaUtilities::logLocationCoordinates(myPoint);

    if(backend->isSimulationRunning())
        return;

    if(editStatePosition)
    {
        lastCoordinate = QPointF(currentState->getCoordinates()[0], currentState->getCoordinates()[1]) ;
        tmp[0] = mc->currentCoordinate().x();
        tmp[1] = mc->currentCoordinate().y();
        currentState->setCoordinates(tmp);
        dx = currentState->getCoordinates()[0] - lastCoordinate.x();
        dy = currentState->getCoordinates()[1] - lastCoordinate.y();
        if(currentState->childrenDoFollowMe())
            currentState->moveMyChildren(dx, dy);

        emit sendMoveCurrentState1(dx, dy);
        setUndoAction("Move State");

        return;
    }

    if(geo)
    {
        if(currentAsset->getLockPosition())
            return;

         if(geo->getRwaType() == RWAPOSITIONTYPE_ASSET)
         {
            lastCoordinate = QPointF(currentAsset->getCoordinates()[0], currentAsset->getCoordinates()[1]);
            tmp[0] = myPoint.x();
            tmp[1] = myPoint.y();
            currentAsset->setCoordinates(tmp);
            dx = myPoint.x() - lastCoordinate.x();
            dy = myPoint.y() - lastCoordinate.y();
            currentAsset->moveMyChildren(dx, dy);
            emit sendMoveCurrentAsset1(dx, dy);
            setUndoAction("Move Asset");
         }

         if(geo->getRwaType() == RWAPOSITIONTYPE_ASSETSTARTPOINT)
         {
             tmp[0] = myPoint.x();
             tmp[1] = myPoint.y();
             currentAsset->setStartPosition(tmp);
             geo->setCoordinate(myPoint);
             setUndoAction("Move Asset start location.");
         }

         if(geo->getRwaType() == RWAPOSITIONTYPE_ASSETCHANNEL &&
                 currentAsset->individuellChannelPositionsAllowed() )
         {
             lastCoordinate = QPointF(currentAsset->channelcoordinates[geo->getChannel()][0], currentAsset->channelcoordinates[geo->getChannel()][1]);
             dx = myPoint.x() - lastCoordinate.x();
             dy = myPoint.y() - lastCoordinate.y();
             tmp[0] = myPoint.x();
             tmp[1] = myPoint.y();
             currentAsset->setChannelCoordinate(geo->getChannel(), tmp);
             currentAsset->individuellChannelPosition[geo->getChannel()] = true;
             geo->setCoordinate(myPoint);
             emit sendMoveCurrentAssetChannel(dx, dy, geo->getChannel());
             setUndoAction("Move Asset channel position.");
         }

         if(geo->getRwaType() == RWAPOSITIONTYPE_REFLECTIONPOSITION)
         {
             lastCoordinate = QPointF(currentAsset->reflectioncoordinates[geo->getChannel()][0], currentAsset->reflectioncoordinates[geo->getChannel()][1]);
             dx = myPoint.x() - lastCoordinate.x();
             dy = myPoint.y() - lastCoordinate.y();
             tmp[0] = myPoint.x();
             tmp[1] = myPoint.y();
             currentAsset->setReflectionCoordinate(geo->getChannel(), tmp);
             geo->setCoordinate(myPoint);
             emit sendMoveCurrentAssetReflection(dx, dy, geo->getChannel());
             setUndoAction("Move Asset reflection position.");
         }

         return;
    }

    if(editArea)
    {
        resizeArea(myPoint, currentState);
        emit sendCurrentStateRadiusEdited();
        setUndoAction("Edit State area.");
        if(stateRadiusVisible)
            stateRadiusLayer->setVisible(true);
    }
}

void RwaStateView::receiveMouseDownEvent(const QMouseEvent *event, const QPointF myPoint)
{
    if(!currentState)
        return;

    currentPoint = nullptr;
    RwaAsset1 *asset;

    RwaUtilities::copyLocationCoordinates2Clipboard(myPoint);
    if(backend->logCoordinates)
        RwaUtilities::logLocationCoordinates(myPoint);

    if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonDblClick)
    {
        if(currentState->getAreaType() == RWAAREATYPE_POLYGON)
        {
            if(mouseDoubleClickArea(myPoint, currentState))
            {
                redrawStateRadii();
                setUndoAction("Edit State area.");
                return;
            }
        }
    }

    if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonPress)
    {
        if(mouseDownArea(myPoint, currentState))
            return;

        QmapPoint tmppoint(myPoint.x(), myPoint.y());

        for (int i=0; i<assetLayer->geometries.count(); i++)
        {
            if (assetLayer->geometries.at(i)->isVisible() && assetLayer->geometries.at(i)->Touches(&tmppoint, mapadapter))
            {
                 currentMapItem = static_cast<RwaMapItem *>(assetLayer->geometries.at(i));
                 asset = static_cast<RwaAsset1 *>(currentMapItem->data);

                 if(currentMapItem->getRwaType() != RWAPOSITIONTYPE_ASSETCHANNEL)
                 {
                     emit sendCurrentAsset(asset);
                     mc->setMouseMode(MapControl::None);
                     return;
                 }
                 else
                 {
                     if(asset->individuellChannelPositionsAllowed())
                     {
                         emit sendCurrentAsset(asset);
                         mc->setMouseMode(MapControl::None);
                         return;
                     }
                 }
             }
         }

        for (int i=0; i<assetReflectionLayer->geometries.count(); i++)
        {
            if (assetReflectionLayer->geometries.at(i)->isVisible() && assetReflectionLayer->geometries.at(i)->Touches(&tmppoint, mapadapter))
            {
                 currentMapItem = static_cast<RwaMapItem *>(assetReflectionLayer->geometries.at(i));
                 asset = static_cast<RwaAsset1 *>(currentMapItem->data);
                 asset->currentReflection = currentMapItem->getChannel();
                 emit sendCurrentAsset(asset);
                 mc->setMouseMode(MapControl::None);
                 return;
             }
         }
     }

     if(!currentState->positionIsLocked())
        editStatePosition = true;
}

void RwaStateView::receiveMouseReleaseEvent()
{
    if(currentState)
    {
        if(editArea)
            currentState->setAreaType(currentState->getAreaType());
    }

    editAreaRadius = false;
    editAreaHeight = false;
    editAreaWidth = false;
    editStatePosition = false;
    editAreaCorner = false;
    editArea = false;
    mc->setMouseMode(MapControl::Panning);
    currentPoint = nullptr;
    currentMapItem = nullptr;
    writeUndo();
}

void RwaStateView::dropEvent(QDropEvent *event)
{
    if(backend->logOther)
        qDebug();
}

void RwaStateView::moveCurrentState()
{
    mc->setView(QPointF(currentState->getCoordinates()[0], currentState->getCoordinates()[1]));
    if(currentState->childrenDoFollowMe())
        redrawAssetsOfCurrentState();

    if(stateRadiusVisible)
        redrawStateRadii();
}

void RwaStateView::setCurrentState(RwaState *state)
{
    if(!state)
        return;

    QDockWidget *window = static_cast<QDockWidget *>(parent());
    window->setWindowTitle("State View - "+ QString::fromStdString(state->objectName()));
    mc->setView(QPointF(state->getCoordinates()[0], state->getCoordinates()[1]));
    setMap2AreaZoomLevel(state);
    RwaGraphicsView::setCurrentState(state);    

    if(QObject::sender() != this->backend)
    {
        qDebug();
        emit sendCurrentAsset(state->lastTouchedAsset);
    }

}

void RwaStateView::setCurrentScene(RwaScene *scene)
{
    if(!scene)
        return;

    if(scene == currentScene)
        return;

    RwaGraphicsView::setCurrentScene(scene);

    if(QObject::sender() != this->backend)
    {
        qDebug();
        emit sendCurrentState(scene->lastTouchedState);
    }
}

int RwaStateView::getNumberOfSelectedAssets()
{
    return assetList->getNumberOfSelectedAssets();
}

void RwaStateView::addAssetItem(const QString &path, qint32 type)
{
    if(currentState)
    {
        string uid = std::string(QUuid::createUuid().toString().toLatin1());
        RwaAsset1 *newItem = new RwaAsset1(path.toStdString(), currentState->getCoordinates(), type, uid);
        backend->simulator->runtime->openFile4MetaData(path.toLatin1());
        newItem->setDuration(((RwaRuntime::assetDuration/RwaRuntime::assetSampleRate)*1000.0));
        newItem->setNumberOfChannels(RwaRuntime::assetChannelCount);
        currentState->addAsset(newItem);
        currentState->setLastTouchedAsset(newItem);
        emit sendCurrentState(currentState);
        //setCurrentState(currentState);
    }
}

void RwaStateView::deleteAssetItem(const QString &path)
{
    if(currentState)
    {
        RwaAsset1 *item = currentState->getAsset(path.toStdString());
        QFile file(QString::fromStdString(item->getFullPath()));

        if(backend->trashAsset && !backend->fileUsedByAnotherAsset(item))
        {
            if(file.exists())
                file.remove();
        }

        currentState->deleteAsset(path.toStdString());
        emit sendCurrentState(currentState);
    }
}





