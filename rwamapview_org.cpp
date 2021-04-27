#include "rwamapview.h"

RwaMapView::RwaMapView(QWidget* parent, RwaScene *scene, RwaBackend *backend)
: RwaGraphicsView(parent, scene, backend)
{
    mc->setParent(this);
    qint32 toolbarFlags = (  RWATOOLBAR_MAPEDITTOOLS
                           | RWATOOLBAR_SCENEMENU
                           | RWATOOLBAR_SIMULATORTOOLS
                           | RWATOOLBAR_SCENEMENU
                           | RWATOOLBAR_SELECTSCENEMENU
                           | RWATOOLBAR_STATEMENU
                           | RWATOOLBAR_SELECTSTATEMENU
                           | RWATOOLBAR_MAINVOLUME
                           | RWATOOLBAR_SUGGESTPLACES);

    stateLineEditVisible = true;
    assetsVisible = false;
    stateRadiusVisible = false;
    editSceneRadius = false;
    editSceneHeight = false;
    editSceneWidth = false;
    editSceneCorner = false;
    cornerIndex2Edit = 0;
    stateIndex2Delete = 0;

    tool = RWATOOL_ARROW;

    layout = new QBoxLayout(QBoxLayout::TopToBottom,this);
    layout->setSpacing(1);
    layout->setContentsMargins(QMargins(1,1,1,1));

    currentScene = NULL;

    connect(mapName, SIGNAL(textEdited(QString)), this, SLOT(updateSceneMenus()));

    mc->setZoom(13);

    // MapControl Notifications

    connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
              this, SLOT(receiveMouseMoveEvent(const QMouseEvent*, const QPointF)));

    connect(mc, SIGNAL(sendMouseDownEvent(const QMouseEvent*, const QPointF)),
              this, SLOT(receiveMouseDownEvent(const QMouseEvent*, const QPointF)));

    connect(mc, SIGNAL(sendSelectRect(QRectF)),
              this, SLOT(receiveSelectRect(QRectF)));

    connect(mc, SIGNAL(sendMouseReleaseEvent()),
              this, SLOT(receiveMouseReleaseEvent()));

    connect(mc, SIGNAL(viewChanged(QPointF,int)),
            this, SLOT(receiveViewChanged(QPointF, int)));

    // Backend Notifications

    connect(this, SIGNAL(sendEntityPosition(QPointF)),
              backend, SLOT(receiveEntityPosition(QPointF)));

    connect(this, SIGNAL(sendMapPosition(QPointF)),
              backend, SLOT(receiveMapCoordinates(QPointF)));

    connect(this, SIGNAL(sendStartStopSimulator(bool)),
              backend, SLOT(startStopSimulator(bool)));

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
              backend, SLOT(receiveLastTouchedState(RwaState*)));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
              backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    connect(this, SIGNAL(sendMoveCurrentState()),
              backend, SLOT(receiveMoveCurrentState()));

    connect(this, SIGNAL(sendAppendScene()),
              backend, SLOT(appendScene()));

    connect(this, SIGNAL(sendClearScene(RwaScene *)),
              backend, SLOT(clearScene(RwaScene *)));

    connect(this, SIGNAL(sendDuplicateScene(RwaScene *)),
              backend, SLOT(duplicateScene(RwaScene *)));

    connect(this, SIGNAL(sendRemoveScene(RwaScene *)),
              backend, SLOT(removeScene(RwaScene *)));

    connect(this, SIGNAL(sendSceneName(RwaScene*, QString)),
              backend, SLOT(receiveSceneName(RwaScene *, QString)));

    connect(backend, SIGNAL(updateScene(RwaScene *)),
              this, SLOT(setCurrentScene(RwaScene *)));

    connect(backend, SIGNAL(sendMoveCurrentAsset()),
              this, SLOT(moveCurrentAsset()));

    connect(backend, SIGNAL(sendMoveCurrentState()),
              this, SLOT(updateState()));

    connect(backend, SIGNAL(sendRedrawAssets()),
              this, SLOT(redrawAssets()));

    connect(backend, SIGNAL(sendCurrentStateRadiusEdited()),
              this, SLOT(receiveUpdateCurrentStateRadius()));

    connect(backend, SIGNAL(sendCurrentSceneRadiusEdited()),
              this, SLOT(receiveUpdateCurrentSceneRadius()));

    connect(this, SIGNAL(sendWriteUndo()), parent, SLOT(writeUndo()));

    layout->addWidget(setupToolbar(toolbarFlags));
    layout->addWidget(mc);
    addZoomButtons();

    // Toolbox Notifications

    connect(toolbar,    SIGNAL(sendMapCoordinates(double,double)),
        this, SLOT(setMapCoordinates(double,double)));

    setCurrentScene(scene);
}



void RwaMapView::receiveStartStopSimulator(bool startStop)
{
    emit sendStartStopSimulator(startStop);
}



void RwaMapView::receiveAppendScene()
{
    emit sendAppendScene();
}

void RwaMapView::receiveClearScene()
{
    emit sendClearScene(currentScene);
}

void RwaMapView::receiveDuplicateScene()
{
    emit sendDuplicateScene(currentScene);
}

void RwaMapView::receiveRemoveScene()
{
    emit sendRemoveScene(currentScene);
}

void RwaMapView::updateAssets(RwaState *, RwaAssetItem *)
{
    //if(assetLayer)
        //assetLayer->clearGeometries();
    //redrawAssets();
}

void RwaMapView::setMapCoordinates(double lon, double lat)
{
    mc->setView(QPointF(lon,lat));
    if(currentScene)
    {
       // currentScene->setCoordinates(QPointF(lon,lat));
        redrawStates();
    }
}

void RwaMapView::setMapCoordinates(QPointF coordinates)
{
    mc->setView(coordinates);
    if(currentScene)
    {
        //currentScene->setCoordinates( coordinates);
        redrawStates();
    }
}

void RwaMapView::zoomIn()
{
    mc->zoomIn();
    if(currentScene)
        currentScene->setZoom( mc->currentZoom());
    if(stateRadiusVisible)
         redrawStateRadii();

    redrawStates();
    //redrawSceneRadii();
}

void RwaMapView::zoomOut()
{
    mc->zoomOut();
    if(currentScene)
        currentScene->setZoom( mc->currentZoom());
    if(stateRadiusVisible)
         redrawStateRadii();

    redrawStates();
    //redrawSceneRadii();
}

void RwaMapView::setZoomLevel(qint32 zoomLevel)
{
    mc->setZoom(zoomLevel);
    //qDebug() << "zoomlevel ";
    if(currentScene)
        currentScene->setZoom(zoomLevel);
    if(stateRadiusVisible)
        redrawStateRadii();
}

void RwaMapView::adaptSize(qint32 width, qint32 height)
{
    //resize(QSize(width, height));
    mc->resize(QSize(width-20, height-70));
}

void RwaMapView::updateState()
{
    if(!(QObject::sender() == this->backend))
    {
        emit sendMoveCurrentState();
    }

    redrawStates();
    if(assetsVisible)
        redrawAssets();
    if(stateRadiusVisible)
        redrawStateRadii();
}

void RwaMapView::moveCurrentScene(const QPointF myPoint)
{
    float dx, dy;
    QPointF lastCoordinate;
    QmapPoint *geo = (QmapPoint *)currentScenePoint;

    if(geo)
    {
        if(currentScene->positionIsLocked())
            return;

        lastCoordinate = currentScene->getCoordinates();
        currentScene->setCoordinates(myPoint);

        QClipboard *clipboard = QApplication::clipboard();
        QString text = QString("%1 %2 %3 %4 %5").arg(currentScene->getCoordinates().x()).arg(currentScene->getCoordinates().y()).arg(currentScene->getRadius()).arg(currentScene->getWidth()).arg(currentScene->getHeight());
        clipboard->setText(text);

        if(backend->getLogCoordinates())
            qDebug() << "Lon: "<< QString::number(currentScene->getCoordinates().x(), 'f', 8) << " Lat: " << QString::number(currentScene->getCoordinates().y(), 'f', 8);

        dx = currentScene->getCoordinates().x() - lastCoordinate.x();
        dy = currentScene->getCoordinates().y() - lastCoordinate.y();

        if(!currentScene->corners->isEmpty())
        {
            for(int i = 0; i< currentScene->corners->count();i++)
            {
               currentScene->corners->data()[i].setX(currentScene->corners->at(i).x() + dx);
               currentScene->corners->data()[i].setY(currentScene->corners->at(i).y() + dy);
            }
        }

        if(currentScene->childrenDoFollowMe())
        {
            currentScene->moveMyChildren(dx, dy);
            redrawStates();
            if(stateRadiusVisible)
                redrawStateRadii();
            if(assetsVisible)
                redrawAssets();
        }
        //redrawScenes();
        //redrawSceneRadii();

    }
}

void RwaMapView::moveCurrentState1(const QPointF myPoint)
{
    float dx, dy;
    QPointF lastCoordinate;
    QmapPoint *geo = (QmapPoint *)currentStatePoint;

    if(geo)
    {
        if(currentState->positionIsLocked())
            return;

        lastCoordinate = currentState->getCoordinates();
        currentState->setCoordinates(myPoint);

        QClipboard *clipboard = QApplication::clipboard();
        QString text = QString("%1 %2 %3 %4 %5").arg(currentState->getCoordinates().x()).arg(currentState->getCoordinates().y()).arg(currentState->getRadius()).arg(currentState->getWidth()).arg(currentState->getHeight());
        clipboard->setText(text);

        if(backend->getLogCoordinates())
            qDebug() << "Lon: "<< QString::number(currentState->getCoordinates().x(), 'f', 8) << " Lat: " << QString::number(currentState->getCoordinates().y(), 'f', 8);

        dx = currentState->getCoordinates().x() - lastCoordinate.x();
        dy = currentState->getCoordinates().y() - lastCoordinate.y();
        if(currentState->childrenDoFollowMe())
            currentState->moveMyChildren(dx, dy);

        redrawState(currentStatePoint);
        if(stateRadiusVisible)
            redrawStateRadii();
        if(assetsVisible)
            redrawAssets();

        emit updateCurrentState();

        return;
    }
}

void RwaMapView::receiveMouseMoveEvent(const QMouseEvent*, const QPointF myPoint)
{
    //qDebug("%s %s %d",  __FILE__, __FUNCTION__ ,__LINE__);
    //qDebug("%f %f",  myPoint.x(), myPoint.y());

    QmapPoint *geo;
    if(!backend->isSimulationRunning())
    {
        geo = (QmapPoint *)currentStatePoint;
        if(geo)
        {
            moveCurrentState1(myPoint);
            return;
        }

        geo = (QmapPoint *)currentScenePoint;
        if(geo)
        {
            moveCurrentScene(myPoint);
            return;
        }

        double newRadius = RwaUtilities::calculateDistance(currentScene->getCoordinates(), myPoint);

        if(editSceneRadius)
        {
            currentScene->setRadius(newRadius*1000);
            //redrawSceneRadii();
           // emit sendCurrentSceneRadiusEdited();
        }

        if( (editSceneWidth || editSceneHeight) && (currentScene->getAreaType() == RWAAREATYPE_SQUARE) )
        {
            currentScene->setWidth(newRadius*2000);
            currentScene->setHeight(newRadius*2000);
           // redrawSceneRadii();
            //emit sendCurrentSceneRadiusEdited();
        }
        if( editSceneWidth && (currentScene->getAreaType() == RWAAREATYPE_RECTANGLE) )
        {
            currentScene->setWidth(newRadius*2000);
           // redrawSceneRadii();
           // emit sendCurrentStateRadiusEdited();
        }

        if( editSceneHeight && (currentScene->getAreaType() == RWAAREATYPE_RECTANGLE) )
        {
            currentScene->setHeight(newRadius*2000);
           // redrawSceneRadii();
           // emit sendCurrentStateRadiusEdited();
        }

        if( editSceneCorner && (currentScene->getAreaType() == RWAAREATYPE_POLYGON) )
        {
            currentScene->corners->data()[cornerIndex2Edit] = myPoint;
           // redrawSceneRadii();
           // emit sendCurrentSceneRadiusEdited();
        }
    }

    geo = (QmapPoint *)currentEntityPoint;
    if(geo)
    {
        geo->setCoordinate(myPoint);
        currentEntity->setGpsCoordinates(myPoint);
        emit sendEntityPosition(currentEntity->getCoordinates());
        return;
    }
    if(tool == RWATOOL_MARKEE)
    {
        mc->setSelectSize(myPoint.x()-selectRectX, myPoint.y()-selectRectY);
        mc->updateRequestNew();
        return;
    }
    emit sendMapPosition(myPoint);
}

bool RwaMapView::mouseDownEntities(const QPointF myPoint)
{
    currentEntityPoint = NULL;
    QmapPoint* tmppoint = new QmapPoint(myPoint.x(), myPoint.y());
    for (int i=0; i<entityLayer->geometries.count(); i++)
    {
        if (entityLayer->geometries.at(i)->isVisible() && entityLayer->geometries.at(i)->Touches(tmppoint, mapadapter))
        {
             currentEntityPoint = (QmapPoint *)entityLayer->geometries.at(i);
             currentEntity= (RwaEntity *)currentEntityPoint->data;
             mc->setMouseMode(MapControl::None);
             break;
        }
    }

    if(currentEntityPoint)
    {
        delete tmppoint;
        return true;
    }
    else
        return false;
}

bool RwaMapView::mouseDownScenes(const QPointF myPoint)
{
    currentScenePoint = NULL;
    QmapPoint* tmppoint = new QmapPoint(myPoint.x(), myPoint.y());
    for (int i=0; i<scenesLayer->geometries.count(); i++)
    {
        if (scenesLayer->geometries.at(i)->isVisible() && scenesLayer->geometries.at(i)->Touches(tmppoint, mapadapter))
        {
             currentScenePoint = (QmapPoint *)scenesLayer->geometries.at(i);
             currentScene = (RwaScene *)currentScenePoint->data;
             mc->setMouseMode(MapControl::None);
             setCurrentScene(currentScene);
             qDebug() << "Mouse Down Scenes";
             //QClipboard *clipboard = QApplication::clipboard();
             //QString text = QString("%1 %2 %3 %4 %5").arg(currentState->getCoordinates().x()).arg(currentState->getCoordinates().y()).arg(currentState->getRadius()).arg(currentState->getWidth()).arg(currentState->getHeight());
             //clipboard->setText(text);
             //scenesLayer->setVisible(true);
             emit sendWriteUndo();

        }
    }

    if(currentScenePoint)
    {
        delete tmppoint;
        return true;
    }
    else
        return false;

}

bool RwaMapView::mouseDownStates(const QPointF myPoint)
{
    currentStatePoint = NULL;
    currentRadiusPoint = NULL;

    QmapPoint* tmppoint = new QmapPoint(myPoint.x(), myPoint.y());
    for (int i=0; i<statesLayer->geometries.count(); i++)
    {
        if (statesLayer->geometries.at(i)->isVisible() && statesLayer->geometries.at(i)->Touches(tmppoint, mapadapter))
        {
             currentStatePoint = (QmapPoint *)statesLayer->geometries.at(i);
             currentState = (RwaState *)currentStatePoint->data;
             mc->setMouseMode(MapControl::None);
             setCurrentState(currentState);
             QClipboard *clipboard = QApplication::clipboard();
             QString text = QString("%1 %2 %3 %4 %5").arg(currentState->getCoordinates().x()).arg(currentState->getCoordinates().y()).arg(currentState->getRadius()).arg(currentState->getWidth()).arg(currentState->getHeight());
             clipboard->setText(text);

             emit sendWriteUndo();
        }
    }


    if(currentStatePoint)
    {
        delete tmppoint;
        return true;
    }
    else
        return false;

}

void RwaMapView::receiveSelectRect(QRectF selectRect)
{
    RwaState *state;
    foreach (state, currentScene->getStates())
    {
        QRectF tmp;
        tmp.setX(state->getCoordinates().x());
        tmp.setY(state->getCoordinates().y());
        tmp.setWidth(0.0000001);tmp.setHeight(0.0000001);
        if(selectRect.contains(tmp))
        {
            state->select(true);
            //qDebug("%d", state->getSta);
        }
    }
}

void RwaMapView::mouseDownMarkee(const QMouseEvent *event, const QPointF myPoint)
{
    if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonPress)
    {

        mc->setMouseMode(MapControl::Dragging);
        /*mc->setSelectOrigin(myPoint.x(), myPoint.y());
        selectRectX = myPoint.x();
        selectRectY = myPoint.y();
        qDebug("%f", myPoint.x());*/

    }
}

void RwaMapView::mouseDownArrow(const QMouseEvent *event, const QPointF myPoint)
{
    if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonPress)
    {
        if(mouseDownEntities(myPoint)) // entities have priority
           return;

        if(!backend->isSimulationRunning())
        {
            if(mouseDownStates(myPoint))
                return;// mouse down is allowed, though no drag for states in simulation

            if(mouseDownScenes(myPoint))
                return;

            double clickDistance;
            double radiusInPx;
            double bearing;
            QPointF somePoint;
            QPointF east, west, north, south;

            clickDistance = getDistanceInPxFromGps(currentScene->getCoordinates(), myPoint);
            bearing = RwaUtilities::calculateBearing(currentScene->getCoordinates(), myPoint);

            north = RwaUtilities::calculateDestination(currentScene->getCoordinates(), currentScene->getHeight()/2, 0);
            west = RwaUtilities::calculateDestination(currentScene->getCoordinates(), currentScene->getWidth()/2, 90);
            south = RwaUtilities::calculateDestination(currentScene->getCoordinates(), currentScene->getHeight()/2, 180);
            east = RwaUtilities::calculateDestination(currentScene->getCoordinates(), currentScene->getWidth()/2, 270);

            north = mapadapter->coordinateToDisplay(north);
            east = mapadapter->coordinateToDisplay(east);
            south = mapadapter->coordinateToDisplay(south);
            west = mapadapter->coordinateToDisplay(west);
            somePoint = mapadapter->coordinateToDisplay(myPoint);

            if(currentScene->getAreaType() == RWAAREATYPE_CIRCLE)
            {
                radiusInPx = getDistanceInPxFromGps(currentScene->getCoordinates(),
                            RwaUtilities::calculatePointOnCircle(currentScene->getCoordinates(), currentScene->getRadius()));// * 0.5;

                if(fabs(clickDistance-radiusInPx) < 10)
                {
                    mc->setMouseMode(MapControl::None);
                    editSceneRadius = true;
                    return;
                }
            }

            if(currentScene->getAreaType() == RWAAREATYPE_SQUARE || currentScene->getAreaType() == RWAAREATYPE_RECTANGLE)
            {
                if( (fabs(somePoint.x() - east.x()) < 10)
                        && ( (somePoint.y() > north.y() && somePoint.y() < south.y() ) ) )
                    editSceneWidth = true;

                if( (fabs(somePoint.x() - west.x()) < 10)
                        && ( (somePoint.y() > north.y() && somePoint.y() < south.y() ) ) )
                    editSceneWidth = true;

                if( (fabs(somePoint.y() - north.y()) < 10)
                        && ( (somePoint.x() > west.x() && somePoint.x() < east.x() ) ) )
                    editSceneHeight = true;

                if( (fabs(somePoint.y() - south.y()) < 10)
                        && ( (somePoint.x() > west.x() && somePoint.x() < east.x() ) ) )
                    editSceneHeight = true;

                if(editSceneWidth || editSceneHeight)
                {
                    mc->setMouseMode(MapControl::None);
                    return;
                }
            }

            if(currentScene->getAreaType() == RWAAREATYPE_POLYGON)
            {
                QPoint corner = QPoint();

                for(int i = 0; i < currentScene->corners->count(); i++)
                {
                    corner = mapadapter->coordinateToDisplay(currentScene->corners->at(i));
                    if( (fabs(somePoint.x() - corner.x()) < 10)
                            && (fabs(somePoint.y() - corner.y()) < 10) )
                    {
                        editSceneCorner = true;
                        mc->setMouseMode(MapControl::None);
                        cornerIndex2Edit = i;

                        return;
                    }
                }
            }
        }
    }

    if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonDblClick && !backend->isSimulationRunning())
    {
        if(currentScene->getAreaType() == RWAAREATYPE_POLYGON)
        {
            QPointF corner = QPointF();

            for(int i = 0; i < currentScene->corners->count(); i++)
            {
                corner = currentScene->corners->at(i);
                if( (fabs(myPoint.x() - corner.x()) < 0.00015)
                        && (fabs(myPoint.y() - corner.y()) < 0.00015) )
                {
                    currentScene->corners->remove(i);
                    //redrawSceneRadii();
                    return;
                }
            }

            QPointF *lastCorner = &currentScene->corners->last();
            for(int i = 0; i < currentScene->corners->count(); i++)
            {
                if(RwaUtilities::mouseDownOnPolygonVertex(*lastCorner, currentScene->corners->at(i), myPoint))
                {
                    mc->setMouseMode(MapControl::None);
                    currentScene->corners->insert(i, myPoint);
                    //redrawSceneRadii();

                    return;
                }
                lastCorner = &currentScene->corners->data()[i];
            }
        }


        RwaState *newState = currentScene->addState(QString("State %1").arg(currentScene->getStateNameCounter()), myPoint);
        currentState = newState;
        updateCurrentState();
        mc->setMouseMode(MapControl::None);
        drawState(newState,1);
        emit sendWriteUndo();

        //redrawSceneRadii();
    }
}

void RwaMapView::mouseDownRubber(const QMouseEvent *event, const QPointF myPoint)
{
    QmapPoint* tmppoint = new QmapPoint(myPoint.x(), myPoint.y());
    if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonPress)
    {
        for (int i=0; i<statesLayer->geometries.count(); i++)
        {
            if (statesLayer->geometries.at(i)->isVisible() && statesLayer->geometries.at(i)->Touches(tmppoint, mapadapter))
            {
                 emit sendWriteUndo();
                 currentStatePoint = (QmapPoint *)statesLayer->geometries.at(i);
                 RwaState *state = (RwaState *)currentStatePoint->data;
                 currentScene->removeState(state);
                 emit sendCurrentScene(currentScene);
                 delete tmppoint;
                 return;
            }
        }
    }
    delete tmppoint;
}

void RwaMapView::receiveMouseDownEvent(const QMouseEvent *event, const QPointF myPoint)
{
    currentStatePoint = NULL;
    switch(tool)
    {
        case RWATOOL_ARROW:
        {
            mouseDownArrow(event, myPoint);
            break;
        }
        case RWATOOL_RUBBER:
        {
            if(!backend->isSimulationRunning())
                mouseDownRubber(event, myPoint);
            break;
        }
        case RWATOOL_MARKEE:
        {
            mouseDownMarkee(event, myPoint);
            break;
        }
        default:break;
    }
}

void RwaMapView::receiveMouseReleaseEvent()
{
    if(writeUndo)
        emit sendWriteUndo();

    mc->setMouseMode(MapControl::Panning);

    currentEntityPoint = NULL;
    currentStatePoint = NULL;
    currentScenePoint = NULL;
    writeUndo = false;
    editSceneRadius = false;
    editSceneHeight = false;
    editSceneWidth = false;
    editSceneCorner = false;
    cornerIndex2Edit = 0;
    currentScene->deselectAllStates();
}

void RwaMapView::receiveSceneName(QString name)
{
    emit sendSceneName(currentScene, name);
}

void RwaMapView::receiveUpdateCurrentStateRadius()
{
    if(stateRadiusVisible)
        redrawStateRadii();
}

void RwaMapView::receiveUpdateCurrentSceneRadius()
{
    redrawSceneRadii();
}

void RwaMapView::setCurrentState(qint32 stateNumber)
{
    if(!currentScene)
        return;
    if(currentScene->getStates().isEmpty())
        return;

    currentState = currentScene->getStates().at(stateNumber);
    this->currentScene->currentState = currentState;

    if(currentStatePoint)
        redrawState(currentStatePoint);
    updatePixmaps(currentStatePoint, statesLayer);

    if(assetsVisible)
        redrawAssets();

    if(stateRadiusVisible)
        redrawStateRadii();
}

void RwaMapView::setCurrentState(RwaState *currentState)
{
    this->currentScene->currentState = currentState;
    this->currentState = currentState;

    if(currentStatePoint)
        redrawState(currentStatePoint);
    updatePixmaps(currentStatePoint, statesLayer);

    if(assetsVisible)
        redrawAssets();

    if(stateRadiusVisible)
        redrawStateRadii();

    if(!(QObject::sender() == this->backend))
    {
        emit sendCurrentState((RwaState *)currentState);
    }
    qDebug() << "SET CURRENT STATE";
}

void RwaMapView::setCurrentScene(RwaScene *scene)
{
    if(scene)
    {
        if(currentScene)
        {
            disconnect(mapName, SIGNAL(textEdited(QString)), this, SLOT(receiveSceneName(QString)));

            if(statesLayer)
                statesLayer->clearGeometries();
            if(assetsVisible)
                assetLayer->clearGeometries();
             if(entityLayer)
                entityLayer->clearGeometries();
             if(stateRadiusVisible)
                 stateRadiusLayer->clearGeometries();

             sceneRadiusLayer->clearGeometries();
         }

        if(currentScene != scene)
            setMapCoordinates(scene->getCoordinates());

        currentScene = scene;

        connect(mapName, SIGNAL(textEdited(QString)), this, SLOT(receiveSceneName(QString)));
        mapName->setText(scene->objectName());

        setZoomLevel(scene->getZoom());

        if(stateRadiusVisible)
           redrawStateRadii();

        if(assetsVisible)
            redrawAssets();

        if(!(QObject::sender() == this->backend))
        {
            //qDebug() << "MapView:" << "emit current Scene";
            emit sendCurrentScene(scene);
        }

        if(currentScene->currentState)
        {
            setCurrentState(currentScene->currentState);
            //emit sendCurrentState((RwaState *)currentState);
        }
        else
        {
            if(!currentScene->getStates().isEmpty())
            {
                setCurrentState(currentScene->getStates().first());
               // emit sendCurrentState((RwaState *)currentState);
            }
        }

        redrawStates();
        //redrawSceneRadii();
        //redrawScenes();
        redrawEntities();
        qDebug() << "SET CURRENT SCENE";
    }
}

void RwaMapView::moveCurrentAsset()
{
    if(assetsVisible)
        redrawAssets();
}

void RwaMapView::setAssetsVisible(bool assetsVisible)
{
    this->assetsVisible = assetsVisible;
    assetLayer->setVisible(assetsVisible);
    redrawAssets();

}

void RwaMapView::setRadiiVisible(bool radiiVisible)
{
    this->stateRadiusVisible = radiiVisible;
    stateRadiusLayer->setVisible(radiiVisible);
    redrawStateRadii();
}
