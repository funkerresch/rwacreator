#include "rwamapview.h"

RwaMapView::RwaMapView(QWidget* parent, RwaScene *scene, QString name)
: RwaGraphicsView(parent, scene, name)
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
    assetStartPointsVisible = false;
    assetReflectionsVisible = false;
    sceneRadiusVisible = true;
    statesVisible = true;
    scenesVisible = true;

    tool = RWATOOL_ARROW;

    layout = new QBoxLayout(QBoxLayout::TopToBottom,this);
    layout->setSpacing(1);
    layout->setContentsMargins(QMargins(1,1,1,1));

    currentScene = nullptr;
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

    // Backend Notifications

    connect(this, SIGNAL(sendEntityPosition(QPointF)),
              backend, SLOT(receiveEntityPosition(QPointF)));

    connect(this, SIGNAL(sendMapPosition(QPointF)),
              backend, SLOT(receiveMapCoordinates(QPointF)));

    connect(this, SIGNAL(sendStateCoordinate(QPointF)),
              backend, SLOT(receiveStatePosition(QPointF)));

//    connect(this, SIGNAL(sendStartStopSimulator(bool)),
//              backend, SLOT(startStopSimulator(bool)));

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
              backend, SLOT(receiveLastTouchedState(RwaState*)));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
              backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    connect(this, SIGNAL(sendMoveCurrentState1(double, double)),
              backend, SLOT(receiveMoveCurrentState1(double, double)));

    connect(this, SIGNAL(sendMoveCurrentScene()),
              backend, SLOT(receiveMoveCurrentScene()));

    connect(this, SIGNAL(sendSelectedStates(QStringList)),
              backend, SLOT(receiveSelectedStates(QStringList)));

    connect(backend, SIGNAL(sendMovePixmapsOfCurrentAsset1(double, double)),
              this, SLOT(movePixmapsOfCurrentAsset(double,double)));

    connect(backend, SIGNAL(updateScene(RwaScene *)),
              this, SLOT(setCurrentScene(RwaScene *)));

    connect(backend, SIGNAL(sendMovePixmapsOfCurrentState1(double, double)),
              this, SLOT(movePixmapsOfCurrentState(double,double)));

    connect(backend, SIGNAL(sendMoveCurrentAssetChannel(double, double, int)),
              this, SLOT(movePixmapsOfCurrentAssetChannel(double,double, int)));

    connect(backend, SIGNAL(sendMoveCurrentAssetReflection(double, double, int)),
              this, SLOT(movePixmapsOfCurrentAssetReflection(double,double, int)));

    connect(this, SIGNAL(sendCurrentStateRadiusEdited()),
            backend, SLOT(receiveCurrentStateRadiusEdited()));

    connect(this, SIGNAL(sendCurrentSceneRadiusEdited()),
            backend, SLOT(receiveCurrentSceneRadiusEdited()));

    connect(backend, SIGNAL(sendRedrawAssets()),
              this, SLOT(redrawAssets()));

    connect(backend, SIGNAL(sendCurrentStateRadiusEdited()),
              this, SLOT(receiveUpdateCurrentStateRadius()));

    connect(backend, SIGNAL(sendCurrentSceneRadiusEdited()),
              this, SLOT(receiveUpdateCurrentSceneRadius()));

    connect(backend, SIGNAL(sendMoveHero2CurrentState()),
              this, SLOT(setEntityCoordinates2CurrentState()));

    connect(backend, SIGNAL(sendMoveHero2CurrentScene()),
              this, SLOT(setEntityCoordinates2CurrentScene()));

    connect(backend, SIGNAL(sendHeroPositionEdited()),
              this, SLOT(receiveHeroPositionEdited()));

    layout->addWidget(setupToolbar(toolbarFlags));
    layout->addWidget(mc);
    addZoomButtons();

    // Toolbox Notifications

    connect(toolbar,    SIGNAL(sendMapCoordinates(double,double)),
        this, SLOT(setMapCoordinates(double,double)));

    connect(this,    SIGNAL(sendMapCoordinates(double,double)),
        toolbar, SLOT(receiveMapCoordinates(double,double)));

    readSettings();
}

void RwaMapView::readSettings()
{
    QSettings settings("Intrinsic Audio", "Rwa Creator");
    setAssetsVisible(settings.value("mapviewassetsvisible").toBool());
    toolbar->assetsVisibleButton->setChecked(settings.value("mapviewassetsvisible").toBool());
    setRadiiVisible(settings.value("mapviewradiivisible").toBool());
    toolbar->radiiVisibleButton->setChecked(settings.value("mapviewradiivisible").toBool());

    // This is not so nice but much less code then putting it in backend and add more slots and signals for very basic functionality
    backend->heroFollowsSceneAndState = settings.value("mapviewherofollows").toBool();
    toolbar->heroFollowsSceneAndStateButton->setChecked(settings.value("mapviewherofollows").toBool());
}

void RwaMapView::writeSettings()
{
    QSettings settings("Intrinsic Audio", "Rwa Creator");
    settings.setValue("mapviewassetsvisible", assetsVisible);
    settings.setValue("mapviewradiivisible", stateRadiusVisible);
    settings.setValue("mapviewherofollows", backend->heroFollowsSceneAndState);
}

void RwaMapView::setMapCoordinates(double lon, double lat)
{
    qDebug();
    mc->setView(QPointF(lon,lat));
    if(currentScene)
    {
        redrawStates();
        emit sendMapPosition(mc->currentCoordinate());
        emit sendMapCoordinates(mc->currentCoordinate().x(), mc->currentCoordinate().y());
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
    redrawSceneRadii();
}

void RwaMapView::zoomOut()
{
    mc->zoomOut();
    if(currentScene)
        currentScene->setZoom( mc->currentZoom());
    if(stateRadiusVisible)
         redrawStateRadii();

    redrawStates();
    redrawSceneRadii();
}

void RwaMapView::adaptSize(qint32 width, qint32 height)
{
    //resize(QSize(width, height));
    mc->resize(QSize(width-20, height-70));
}

void RwaMapView::moveCurrentScene(const QPointF myPoint)
{
    double dx, dy;
    std::vector<double> lastCoordinate(2, 0.0);
    std::vector<double> tmp(2, 0.0);
    QmapPoint *geo = (QmapPoint *)currentScenePoint;

    if(geo)
    {
        if(currentScene->positionIsLocked())
            return;

        geo->setCoordinate(myPoint);

        lastCoordinate = currentScene->getCoordinates();
        tmp[0] = myPoint.x();
        tmp[1] = myPoint.y();
        currentScene->setCoordinates(tmp);

        dx = currentScene->getCoordinates()[0] - lastCoordinate[0];
        dy = currentScene->getCoordinates()[1] - lastCoordinate[1];
        geo->move(dx, dy);

        if(currentScene->childrenDoFollowMe())
        {
            currentScene->moveMyChildren(dx, dy);
            movePixmapsOfCurrentScene(dx,dy);
            emit sendMoveCurrentScene();
        }

        currentScene->moveCorners(dx, dy);
        redrawSceneRadii();
        return;
    }
}

void RwaMapView::moveCurrentState1(const QPointF myPoint)
{
    double dx, dy;
    std::vector<double> lastCoordinate(2, 0.0);
    std::vector<double> tmp(2, 0.0);
    QmapPoint *geo = (QmapPoint *)currentStatePoint;

    if(geo)
    {
        if(currentState->positionIsLocked())
            return;

        lastCoordinate = currentState->getCoordinates();
        tmp[0] = myPoint.x();
        tmp[1] = myPoint.y();
        currentState->setCoordinates(tmp);

        dx = currentState->getCoordinates()[0] - lastCoordinate[0];
        dy = currentState->getCoordinates()[1] - lastCoordinate[1];
        if(currentState->childrenDoFollowMe())      
            currentState->moveMyChildren(dx, dy);

        emit sendMoveCurrentState1(dx, dy);
        emit sendStateCoordinate(myPoint);

        return;
    }
}

void RwaMapView::receiveMouseMoveEvent(const QMouseEvent*, const QPointF myPoint)
{
    if(tool != RWATOOL_ARROW)
        return;

    QmapPoint *geo;
    if(!backend->isSimulationRunning())
    {
        geo = (QmapPoint *)currentStatePoint;
        if(geo)
        {
            moveCurrentState1(myPoint);
            if(!currentState->positionIsLocked())
                setUndoAction("Move State");
            return;
        }

        geo = (QmapPoint *)currentScenePoint;
        if(geo)
        {
            moveCurrentScene(myPoint);
            if(!currentScene->positionIsLocked())
                setUndoAction("Move Scene");
            return;
        }

        if(editArea)
        {
            if(editSceneArea)
            {
                resizeArea(myPoint, currentScene);
                setUndoAction("Resize Scene Area");
                emit sendCurrentSceneRadiusEdited();
                sceneRadiusLayer->setVisible(true);
            }
            if(editStateArea)
            {
                resizeArea(myPoint, currentState);
                setUndoAction("Resize State Area");
                emit sendCurrentStateRadiusEdited();
                stateRadiusLayer->setVisible(true);
            }
            return;
        }
    }

    geo = (QmapPoint *)currentEntityPoint;
    if(geo)
    {
        geo->setCoordinate(myPoint);
        currentEntity = (RwaEntity *)geo->data;
        std::vector<double> tmp {myPoint.x(), myPoint.y()};
        currentEntity->setCoordinates(tmp);
        emit sendEntityPosition(myPoint);
        return;
    }
    if(tool == RWATOOL_PEN)
    {
        mc->setSelectSize(myPoint.x()-selectRectX, myPoint.y()-selectRectY);
        mc->updateRequestNew();
        return;
    }

    emit sendMapPosition(mc->currentCoordinate());
    emit sendMapCoordinates(mc->currentCoordinate().x(), mc->currentCoordinate().y());
    RwaUtilities::copyLocationCoordinates2Clipboard(mc->currentCoordinate());

    if(backend->logCoordinates)
        RwaUtilities::logLocationCoordinates(mc->currentCoordinate());
}

bool RwaMapView::mouseDownEntities(const QPointF myPoint)
{
    currentEntityPoint = nullptr;
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
    currentScenePoint = nullptr;
    QmapPoint* tmppoint = new QmapPoint(myPoint.x(), myPoint.y());
    for (int i=0; i<scenesLayer->geometries.count(); i++)
    {
        if (scenesLayer->geometries.at(i)->isVisible() && scenesLayer->geometries.at(i)->Touches(tmppoint, mapadapter))
        {
             currentScenePoint = (QmapPoint *)scenesLayer->geometries.at(i);
             currentScene = (RwaScene *)currentScenePoint->data;
             mc->setMouseMode(MapControl::None);
             scenesLayer->setVisible(true);
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
    currentStatePoint = nullptr;
    QmapPoint* tmppoint = new QmapPoint(myPoint.x(), myPoint.y());
    for (int i=0; i<statesLayer->geometries.count(); i++)
    {
        if (statesLayer->geometries.at(i)->isVisible() && statesLayer->geometries.at(i)->Touches(tmppoint, mapadapter))
        {
             currentStatePoint = (QmapPoint *)statesLayer->geometries.at(i);
             currentState = (RwaState *)currentStatePoint->data;
             mc->setMouseMode(MapControl::None);

             tmpObjectName = QString::fromStdString( currentState->objectName());
             emit sendCurrentState(currentState);
             statesLayer->setVisible(true);
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
        tmp.setX(state->getCoordinates()[0]);
        tmp.setY(state->getCoordinates()[1]);
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
                return;

            if(mouseDownScenes(myPoint))
                return;

            if(mouseDownArea(myPoint, currentScene))
                return;

            if(mouseDownArea(myPoint, currentState))
                return;
        }
    }

    if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonDblClick && !backend->isSimulationRunning())
    {
        if(mouseDoubleClickArea(myPoint, currentScene))
        {
            setUndoAction("Edit Scene area.");
            sceneRadiusLayer->setVisible(true);
            return;
        }

        if(mouseDoubleClickArea(myPoint, currentState))
        {
            setUndoAction("Edit State area.");
            sceneRadiusLayer->setVisible(true);
            emit sendCurrentStateRadiusEdited();
            return;
        }

        std::vector<double> tmp(2, 0.0);
        tmp[0] = myPoint.x();
        tmp[1] = myPoint.y();

        std::string stateName("State "+ std::to_string( RwaBackend::getStateNameCounter(currentScene->getStates())));
        RwaState *newState = currentScene->addState(stateName, tmp);
        emit sendCurrentScene(currentScene);
        emit sendCurrentState(newState);
        mc->setMouseMode(MapControl::None);
        setUndoAction("New State");
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
                 currentStatePoint = (QmapPoint *)statesLayer->geometries.at(i);
                 RwaState *state = (RwaState *)currentStatePoint->data;
                 currentScene->removeState(state);
                 emit sendCurrentScene(currentScene);
                 emit sendCurrentState(currentScene->lastTouchedState);
                 setUndoAction("Remove State");
                 delete tmppoint;
                 return;
            }
        }
    }
    delete tmppoint;
}

void RwaMapView::receiveMouseDownEvent(const QMouseEvent *event, const QPointF myPoint)
{
    RwaUtilities::copyLocationCoordinates2Clipboard(myPoint);
    if(backend->logCoordinates)
        RwaUtilities::logLocationCoordinates(myPoint);

    currentStatePoint = nullptr;
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
        case RWATOOL_PEN:
        {
            mouseDownMarkee(event, myPoint);
            break;
        }
        default:break;
    }
}

void RwaMapView::receiveMouseReleaseEvent()
{
    if(tool == RWATOOL_PEN)
    {
        QStringList states;
        for (int i=0; i<statesLayer->geometries.count(); i++)
        {
            currentStatePoint = (QmapPoint *)statesLayer->geometries.at(i);
            std::vector<double> position = std::vector<double>(2, 0.0);
            position[0] = currentStatePoint->coordinate().x();
            position[1] = currentStatePoint->coordinate().y();
            std::vector<double> topLeft = std::vector<double>(2, 0.0);
            topLeft[0] = mc->getSelectRect().topLeft().x();
            topLeft[1] = mc->getSelectRect().topLeft().y();
            std::vector<double> bottomRight = std::vector<double>(2, 0.0);
            bottomRight[0] = mc->getSelectRect().bottomRight().x();
            bottomRight[1] = mc->getSelectRect().bottomRight().y();

            if(RwaUtilities::coordinateWithinRectangle1(position, topLeft, bottomRight))
            {
                RwaState *state = (RwaState *)currentStatePoint->data;
                states << QString::fromStdString(state->objectName());
            }
        }

        mc->updateRequestNew();
        if(!(QObject::sender() == this->backend))
        {
            emit sendSelectedStates(states);
        }
        return;
    }

    mc->setMouseMode(MapControl::Panning);
    if(currentScene)
    {
        currentScene->deselectAllStates();
        if(editArea)
            currentScene->setAreaType(currentScene->getAreaType());
    }

    currentEntityPoint = nullptr;
    currentStatePoint = nullptr;
    currentScenePoint = nullptr;
    editAreaRadius = false;
    editAreaHeight = false;
    editAreaWidth = false;
    editAreaCorner = false;
    editArea = false;
    editSceneArea = false;
    editStateArea = false;
    areaCornerIndex2Edit = -1;   
    writeUndo();
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

void RwaMapView::receiveHeroPositionEdited()
{
    redrawEntities();
}

void RwaMapView::receiveUpdateCurrentSceneRadius()
{
    if(sceneRadiusVisible)
        redrawSceneRadii();
}

void RwaMapView::setCurrentAsset(RwaAsset1 *asset)
{
    if(!asset)
        return;

    RwaGraphicsView::setCurrentAsset(asset);
}

void RwaMapView::setCurrentState(qint32 stateNumber)
{
    if(!currentScene)
        return;
    if(currentScene->getStates().empty())
        return;

    auto statesList = currentScene->getStates().begin();
    std::advance(statesList, stateNumber);

    RwaState *state = *statesList;
    setCurrentState(state);
}

void RwaMapView::setCurrentState(RwaState *state)
{
    RwaGraphicsView::setCurrentState(state);
}

void RwaMapView::setCurrentScene(RwaScene *scene)
{
    if(!scene)
        return;

    QDockWidget *window = static_cast<QDockWidget *>(parent());
    window->setWindowTitle("Map View - "+ QString::fromStdString(scene->objectName()));

    if(!backend->isSimulationRunning())
    {
        setMapCoordinates(scene->getCoordinates()[0], scene->getCoordinates()[1]);
        setMap2AreaZoomLevel(scene);
    }

    RwaGraphicsView::setCurrentScene(scene);
}

void RwaMapView::moveCurrentAsset()
{
    if(assetsVisible)
        redrawAssets();
}


