#include "rwagraphicsview.h"
#include "rwastyles.h"
#include <math.h>

RwaGraphicsView::RwaGraphicsView(QWidget *parent, RwaScene *scene, RwaBackend *backend) :
    RwaView(parent, scene, backend)
{
    QString path = backend->getCompleteBundlePath();

    stateLineEditVisible = false;
    entityLineEditVisible = false;
    assetLineEditVisible = false;

    currentEntityPoint = NULL;
    currentStatePoint = NULL;
    currentScenePoint = NULL;
    currentAssetPoint = NULL;

    entityVisible = false;
    assetsVisible = false;
    statesVisible = false;
    stateRadiusVisible = false;

    rightLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    leftLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    innerLayout = new QBoxLayout(QBoxLayout::LeftToRight);

    mapName = new QLineEdit();
    mapName->setFixedWidth(120);
    mapName->setFixedHeight(20);
    zoomInButton = new QPushButton("+");
    zoomOutButton = new QPushButton("-");
    zoomInButton->setMinimumWidth(50);
    zoomInButton->setFixedHeight(20);
    zoomOutButton->setFixedWidth(50);
    zoomOutButton->setFixedHeight(20);

    zoomInButton->setStyleSheet(rwaButtonStyleSheet);
    zoomOutButton->setStyleSheet(rwaButtonStyleSheet);
    mapName->setStyleSheet(rwaButtonStyleSheet);

    mc = new MapControl(QSize(250,250));

    mapadapter = new OSMMapAdapter();
    l = new MapLayer("Custom Layer", mapadapter);
    mc->addLayer(l);

    scenesLayer = new GeometryLayer("Scene Layer", mapadapter);
    scenesLayer->setActivePixmap(QPixmap(path+"images/sceneselected.png"));
    scenesLayer->setPassivePixmap(QPixmap(path+"images/scene.png"));
    mc->addLayer(scenesLayer);

    sceneRadiusLayer = new GeometryLayer("Scene Area Layer", mapadapter);
    mc->addLayer(sceneRadiusLayer);

    statesLayer = new GeometryLayer("Geometry Layer", mapadapter);
    statesLayer->setActivePixmap(QPixmap(path+"images/stateselected.png"));
    statesLayer->setPassivePixmap(QPixmap(path+"images/state.png"));
    mc->addLayer(statesLayer);

    stateRadiusLayer = new GeometryLayer("Radius Layer", mapadapter);
    mc->addLayer(stateRadiusLayer);

    assetLayer = new GeometryLayer("Asset Layer", mapadapter);
    assetLayer->setActivePixmap(QPixmap(path+"images/audiosourceselected.png"));
    assetLayer->setPassivePixmap(QPixmap(path+"images/audiosource.png"));
    assetLayer->setPixmap3(QPixmap(path+"images/audiochannelsource.png"));
    assetLayer->setPixmap4(QPixmap(path+"images/audiosourcestartpoint.png"));
    assetLayer->setPixmap5(QPixmap(path+"images/audiosourcestartpoint1.png"));
    mc->addLayer(assetLayer);

    entityLayer = new GeometryLayer("Client Layer", mapadapter);
    entityLayer->setActivePixmap(QPixmap(path+"images/hero4.png"));
    mc->addLayer(entityLayer);

    mc->setParent(this);
    mc->showScale(true);
    mc->setView(QPointF(8.26,50));
    mc->setZoom(18);
    mapCoordinates = (QPointF(8.26,50));
    connect(backend, SIGNAL(newGameLoaded()), this, SLOT(initNewGame()));
    connect(this, SIGNAL(sendStateName(RwaState*,QString)), backend, SLOT(receiveStateName(RwaState*,QString)));
}

void RwaGraphicsView::initNewGame()
{
    QDir tilecache(backend->getCompleteProjectPath() + "/tilecache");
    mc->enablePersistentCache(tilecache);
    //qDebug() << backend->getCompleteProjectPath() + "/tilecache";
}

void RwaGraphicsView::addZoomButtons()
{
    innerLayout->setAlignment(Qt::AlignTop);
    mc->setLayout(innerLayout);
    connect(zoomInButton, SIGNAL(clicked(bool)),
              this, SLOT(zoomIn()));
    connect(zoomOutButton, SIGNAL(clicked(bool)),
              this, SLOT(zoomOut()));

    leftLayout->addWidget(zoomInButton);
    leftLayout->addWidget(zoomOutButton);

    rightLayout->addWidget(mapName);
    rightLayout->setAlignment(Qt::AlignRight);
    leftLayout->setAlignment(Qt::AlignLeft);
    innerLayout->addLayout(leftLayout);
    innerLayout->addLayout(rightLayout);
}

double RwaGraphicsView::getDistanceInPxFromGps(QPointF p1, QPointF p2)
{
    QPointF point1;
    QPointF endCoordinate;
    point1 = mapadapter->coordinateToDisplay(p1);
    endCoordinate = mapadapter->coordinateToDisplay(p2);
    return sqrt(pow(point1.x()-endCoordinate.x(),2) + pow(point1.y()-endCoordinate.y(),2)) * 2; // corrected from diameter to radius with factor 2
}

double RwaGraphicsView::getDistanceInPxFromScreen(QPointF p1, QPointF p2)
{
    return sqrt(pow(p1.x()-p1.x(),2) + pow(p1.y()-p2.y(),2));
}

void RwaGraphicsView::setTool(qint32 tool)
{
    this->tool = tool;
    switch(tool)
    {
        case(RWATOOL_ARROW):
        {
            setCursor(Qt::ArrowCursor);
            break;
        }
        case(RWATOOL_PEN):
        {
            setCursor(Qt::CrossCursor);
            break;
        }
        case(RWATOOL_RUBBER):
        {
            setCursor(Qt::OpenHandCursor);
            break;
        }
        case(RWATOOL_MARKEE):
        {
            setCursor(Qt::IBeamCursor);
            break;
        }
        default:break;
    }
}

void RwaGraphicsView::setAssetsVisible(bool assetsVisible)
{
    this->assetsVisible = assetsVisible;
    if(assetsVisible)
        redrawAssets();
    else
        assetLayer->clearGeometries();
}

void RwaGraphicsView::setEntitiesVisible(bool entitiesVisible)
{
    this->entityVisible = entitiesVisible;
}

void RwaGraphicsView::setStatesVisible(bool statesVisible)
{
    this->statesVisible = statesVisible;
}

void RwaGraphicsView::setStateRadiusVisible(bool radiusVisible)
{
    stateRadiusVisible = radiusVisible;
    if(stateRadiusVisible)
        redrawStateRadii();
    else
        stateRadiusLayer->clearGeometries();
}

void RwaGraphicsView::updatePixmaps(QObject *active, GeometryLayer *layer)
{
    for (int i=0; i<layer->geometries.count(); i++)
    {
        QmapPoint *point;
        QLineEdit *lineEdit;
        point = (QmapPoint *)layer->geometries.at(i);
        if(point->data == active)
        {
            point->setPixmap(layer->getActivePixmap());
            lineEdit = point->lineEdit();
            if(!backend->isSimulationRunning() && stateLineEditVisible)
            {
                point->setLineEditVisible(true);
                point->setLabelVisible(false);
            }

        }
        else
        {
            point->setPixmap(layer->getPassivePixmap());
            lineEdit = point->lineEdit();
            point->setLineEditVisible(false);
            point->setLabelVisible(true);

        }
        //layer->setVisible(true);
    }
}

void RwaGraphicsView::updatePixmaps(QmapPoint *active, GeometryLayer *layer)
{
    for (int i=0; i<layer->geometries.count(); i++)
    {
        QmapPoint *point;
        QLineEdit *lineEdit;
        point = (QmapPoint *)layer->geometries.at(i);
        if(point == active)
        {
            point->setPixmap(layer->getActivePixmap());
            lineEdit = point->lineEdit();
            if(!backend->isSimulationRunning() && stateLineEditVisible)
            {
                point->setLineEditVisible(true);
                point->setLabelVisible(false);
            }
        }
        else
        {
            point->setPixmap(layer->getPassivePixmap());
            lineEdit = point->lineEdit();
            point->setLineEditVisible(false);
            point->setLabelVisible(true);

        }
    }
}

void RwaGraphicsView::receiveStateName(QString name)
{
    emit sendStateName(currentState, name);
}

void RwaGraphicsView::updateCurrentState()
{
    updatePixmaps(currentStatePoint, statesLayer);
    if(stateRadiusVisible)
        drawArea(currentState, stateRadiusLayer);

    if(!(QObject::sender() == this->backend))
    {
          emit sendCurrentState((RwaState *)currentState);
    }
}

void RwaGraphicsView::workAroundLineEditBug()
{
    QTimer::singleShot(100, this, SLOT(updateCurrentState()));
}

void RwaGraphicsView::drawArea(RwaArea *area, GeometryLayer *layer)
{
    float radius;
    float width;
    float height;

    if(area->getAreaType() == RWAAREATYPE_CIRCLE)
    {
        radius = getDistanceInPxFromGps(area->getCoordinates(),
                    RwaUtilities::calculatePointOnCircle(area->getCoordinates(), area->getRadius()));

        CirclePoint* stateArea = new CirclePoint(area->getCoordinates().x(), area->getCoordinates().y(), radius);
        stateArea->setData((QObject *) area);
        layer->addGeometry(stateArea);
    }

    if(area->getAreaType() == RWAAREATYPE_RECTANGLE || area->getAreaType() == RWAAREATYPE_SQUARE)
    {
        width = getDistanceInPxFromGps(area->getCoordinates(),
                    RwaUtilities::calculatePointOnCircle(area->getCoordinates(), area->getWidth()/2));

        height = getDistanceInPxFromGps(area->getCoordinates(),
                    RwaUtilities::calculatePointOnCircle(area->getCoordinates(), area->getHeight()/2));

        RectPoint *stateArea = new RectPoint(area->getCoordinates().x(), area->getCoordinates().y(), width, height);

        stateArea->setData((QObject *) area);
        layer->addGeometry(stateArea);
    }

    if(area->getAreaType() == RWAAREATYPE_POLYGON)
    {
        if(area->corners->isEmpty())
        {
            QVector <QPointF> *corners = new QVector<QPointF>;
            QPointF corner = QPoint(0,0);
            QPointF tmpWest = QPoint(0,0);
            QPointF tmpEast = QPoint(0,0);

            width = getDistanceInPxFromGps(area->getCoordinates(),
                        RwaUtilities::calculatePointOnCircle(area->getCoordinates(), area->getWidth()/2));

            height = getDistanceInPxFromGps(area->getCoordinates(),
                        RwaUtilities::calculatePointOnCircle(area->getCoordinates(), area->getHeight()/2));

            tmpWest = RwaUtilities::calculateDestination(area->getCoordinates(), area->getWidth()/2, 270);
            tmpEast = RwaUtilities::calculateDestination(area->getCoordinates(), area->getWidth()/2, 90);

            corner = RwaUtilities::calculateDestination(tmpWest, area->getHeight()/2, 0);
            corners->append(corner);

            corner = RwaUtilities::calculateDestination(tmpEast, area->getHeight()/2, 0);
            corners->append(corner);

            corner = RwaUtilities::calculateDestination(tmpEast, area->getHeight()/2, 180);
            corners->append(corner);

            corner = RwaUtilities::calculateDestination(tmpWest, area->getHeight()/2, 180);
            corners->append(corner);

            area->corners = corners;

            PolygonPoint *stateArea = new PolygonPoint(area->getCoordinates().x(), area->getCoordinates().y(), corners);

            stateArea->setData((QObject *) area);
            layer->addGeometry(stateArea);
        }
        else
        {
            PolygonPoint *stateArea = new PolygonPoint(area->getCoordinates().x(), area->getCoordinates().y(), area->corners);
            stateArea->setData((QObject *) area);
            layer->addGeometry(stateArea);
        }
    }
}

void RwaGraphicsView::drawScene(RwaScene *scene, bool isActive)
{
    QmapPoint* newPlace;

    if(isActive)
        newPlace = new QmapPoint(scene->getCoordinates().x(), scene->getCoordinates().y(), scenesLayer->getActivePixmap(), scene);
    else
        newPlace = new QmapPoint(scene->getCoordinates().x(), scene->getCoordinates().y(), scenesLayer->getPassivePixmap(), scene);

    scenesLayer->addGeometry(newPlace);
}

void RwaGraphicsView::drawState(RwaState *state, bool isActive)
{
    if(state->getType() != RWASTATETYPE_GPS)
        return;

    QmapPoint* newPlace;

    if(isActive)
        newPlace = new QmapPoint(state->getCoordinates().x(), state->getCoordinates().y(), statesLayer->getActivePixmap(), state);
    else
        newPlace = new QmapPoint(state->getCoordinates().x(), state->getCoordinates().y(), statesLayer->getPassivePixmap(), state);

    QLineEdit *stateName = new QLineEdit(mc);
    QLabel *stateNameLabel = new QLabel(mc);

    stateName->setText(state->objectName());
    stateName->setFixedWidth(80);
    stateName->setFixedHeight(21);
    stateNameLabel->setText(state->objectName());
    stateNameLabel->setFixedWidth(80);
    stateNameLabel->setFixedHeight(21);

    newPlace->addLineEditWidget(stateName);
    newPlace->addLabelWidget(stateNameLabel);

    connect(stateName, SIGNAL(textEdited(QString)), this, SLOT(receiveStateName(QString)));
    connect(stateName, SIGNAL(editingFinished()), this, SLOT(workAroundLineEditBug()));
    //connect(stateName, SIGNAL(returnPressed()), this, SLOT(updateCurrentState()));

    if(isActive && stateLineEditVisible && !backend->isSimulationRunning())
         newPlace->setLineEditVisible(true);
    else if(isActive && stateLineEditVisible && backend->isSimulationRunning())
    {
        stateNameLabel->setStyleSheet(rwaSimulationOnLabelActive);
        newPlace->setLabelVisible(true);
    }

    else if(!isActive && stateLineEditVisible && backend->isSimulationRunning())
    {
        stateNameLabel->setStyleSheet(rwaSimulationOnLabelNotActive);
        newPlace->setLabelVisible(true);
    }
    else
        newPlace->setLabelVisible(true);

    statesLayer->addGeometry(newPlace);
    currentStatePoint=newPlace;

    //qDebug() << "COORDINATES: " << newPlace->coordinate().x();
    //statesLayer->addGeometry(radiusCircle);
}

void RwaGraphicsView::drawAsset(RwaAssetItem *item, bool isActive)
{
    QmapPoint* newPlace;

    if(isActive)
        newPlace = new QmapPoint(item->getCoordinates().x(), item->getCoordinates().y(), assetLayer->getActivePixmap(), item);
    else
        newPlace = new QmapPoint(item->getCoordinates().x(), item->getCoordinates().y(), assetLayer->getPassivePixmap(), item);

    newPlace->setType(RWAPOSITIONTYPE_ASSET);
    currentAssetPoint=newPlace;

    assetLayer->addGeometry(newPlace);

    if(!backend->isSimulationRunning())
        item->calculateChannelPositions(); // else they are calculated in the gameloop for visualisation purposes

    if(item->getMoveFromStartPosition())
    {
        newPlace = new QmapPoint(item->getStartPosition().x(), item->getStartPosition().y(), assetLayer->getPixmap5(), item);
        newPlace->setAllowTouches(true);
        newPlace->setType(RWAPOSITIONTYPE_ASSETSTARTPOINT);
        assetLayer->addGeometry(newPlace);

        if(backend->isSimulationRunning())
        {
            newPlace = new QmapPoint(item->getCurrentPosition().x(), item->getCurrentPosition().y(), assetLayer->getPixmap4(), item);
            newPlace->setAllowTouches(false);
            newPlace->setType(RWAPOSITIONTYPE_CURRENTASSETPOSITION);
            assetLayer->addGeometry(newPlace);
        }
    }

    if(item->getMultichannelsourceradius() > 0)
    {
        if(item->playbackType == RWAPLAYBACKTYPE_BINAURALMONO)
        {
            newPlace = new QmapPoint(item->channelcoordinates[0].x(), item->channelcoordinates[0].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
        }

        if(item->playbackType == RWAPLAYBACKTYPE_BINAURALSTEREO)
        {
            newPlace = new QmapPoint(item->channelcoordinates[0].x(), item->channelcoordinates[0].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
            newPlace = new QmapPoint(item->channelcoordinates[1].x(), item->channelcoordinates[1].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
        }

        if(item->playbackType == RWAPLAYBACKTYPE_BINAURAL5CHANNEL)
        {
            newPlace = new QmapPoint(item->channelcoordinates[0].x(), item->channelcoordinates[0].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
            newPlace = new QmapPoint(item->channelcoordinates[1].x(), item->channelcoordinates[1].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
            newPlace = new QmapPoint(item->channelcoordinates[2].x(), item->channelcoordinates[2].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
            newPlace = new QmapPoint(item->channelcoordinates[3].x(), item->channelcoordinates[3].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
            newPlace = new QmapPoint(item->channelcoordinates[4].x(), item->channelcoordinates[4].y(), assetLayer->getPixmap3(), item);
            newPlace->setAllowTouches(false);
            assetLayer->addGeometry(newPlace);
        }
    }
}

void RwaGraphicsView::drawEntity(RwaEntity *entity, bool isActive)
{
    QmapPoint* newPlace;

    if(isActive)
        newPlace = new QmapPoint(entity->getCoordinates().x(), entity->getCoordinates().y(), entityLayer->getActivePixmap(), entity);
    else
        newPlace = new QmapPoint(entity->getCoordinates().x(), entity->getCoordinates().y(), entityLayer->getPassivePixmap(), entity);

    entityLayer->addGeometry(newPlace);
}


void RwaGraphicsView::redrawAssets()
{
    assetLayer->clearGeometries();
    RwaState *state;
    RwaAssetItem *item;

    foreach (state, currentScene->getStates())
    {
        foreach (item, state->getAssets())
        {
            if(item == currentAsset)
                drawAsset(item, 1);
            else
                drawAsset(item, 0);
        }
    }
}

void RwaGraphicsView::redrawRadiusOfCurrentState()
{
    if(!currentState)
        return;

    stateRadiusLayer->clearGeometries();
    if(currentState->isGpsState)
        drawArea(currentState, stateRadiusLayer);
}

void RwaGraphicsView::redrawAssetsOfCurrentState()
{
    assetLayer->clearGeometries();
    RwaAssetItem *item;

    if(!currentState)
        return;

    foreach (item, currentState->getAssets())
    {
        if(item == currentAsset)
            drawAsset(item, 1);
        else
            drawAsset(item, 0);
    }
}

void RwaGraphicsView::redrawScenes()
{
    scenesLayer->clearGeometries();
    int currentLevel = currentScene->getLevel();
    RwaScene *scene;
    foreach (scene, backend->getScenes())
    {
        if(scene->getLevel() == currentLevel)
        {
            if(scene == currentScene)
                drawScene(scene, 1);
            else
                drawScene(scene);
        }
    }
}

void RwaGraphicsView::redrawArea(QmapPoint *area)
{

}

void RwaGraphicsView::redrawAsset(QmapPoint *assetPoint)
{
    assetLayer->removeGeometry(assetPoint);
    RwaAssetItem *asset;
    foreach (asset, currentState->getAssets())
    {
          if(asset == currentAsset)
         {
              drawAsset(asset,1);
               break;
         }
    }
}

void RwaGraphicsView::redrawState(QmapPoint *statePoint)
{
    statesLayer->removeGeometry(statePoint);
    RwaState *state;
    foreach (state, currentScene->getStates())
    {
          if(state == currentState)
         {
              drawState(state,1);
               break;
         }
    }
}

void RwaGraphicsView::redrawStates()
{
    statesLayer->clearGeometries();
    RwaState *state;
    foreach (state, currentScene->getStates())
    {
        if(state->isGpsState)
        {
             if(state != currentState)
                drawState(state);
             else
                 drawState(state, 1);
        }
    }
}

void RwaGraphicsView::redrawSceneRadii()
{
    sceneRadiusLayer->clearGeometries();
    RwaScene *scene;
    int currentLevel = currentScene->getLevel();
    foreach (scene, backend->getScenes())
    {
         if(scene->getLevel() == currentLevel)
            drawArea(scene, sceneRadiusLayer);
    }
}

void RwaGraphicsView::redrawStateRadii()
{
    //qDebug() << "DRAWWWW STATE RADII";
    stateRadiusLayer->clearGeometries();
    RwaState *state;
    foreach (state, currentScene->getStates())
    {
        if(state->isGpsState)
             drawArea(state, stateRadiusLayer);
    }
}

void RwaGraphicsView::redrawEntities()
{
    entityLayer->clearGeometries();
    for (int i=0; i<backend->simulator->entities.count(); i++)
    {
        RwaEntity *entity = backend->simulator->entities.at(i);
        if(!entity)
            return;

        drawEntity(entity, true);
    }
}
