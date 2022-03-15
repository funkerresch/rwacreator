#include "rwagraphicsview.h"
#include "rwastyles.h"
#include <math.h>

RwaGraphicsView::RwaGraphicsView(QWidget *parent, RwaScene *scene) :
    RwaView(parent, scene)
{
    QString path = backend->completeBundlePath;

    stateLineEditVisible = false;
    entityLineEditVisible = false;
    assetLineEditVisible = false;
    entityVisible = false;
    assetsVisible = false;
    statesVisible = false;
    stateRadiusVisible = false;
    assetStartPointsVisible = true;
    assetReflectionsVisible = true;

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

    assetReflectionLayer = new GeometryLayer("Asset Reflection Layer", mapadapter);
    assetReflectionLayer->setActivePixmap(QPixmap(path+"images/audioreflectionactive.png"));
    assetReflectionLayer->setPassivePixmap(QPixmap(path+"images/audioreflectionpassive.png"));
    mc->addLayer(assetReflectionLayer);

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
    QDir tilecache(backend->completeProjectPath + "/tilecache");
    mc->enablePersistentCache(tilecache);
    entityLayer->clearGeometries();
    entityInitialized = false;
    emit sendCurrentScene(backend->getScenes().first());
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
//        case(RWATOOL_MARKEE):
//        {
//            setCursor(Qt::IBeamCursor);
//            break;
//        }
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

void RwaGraphicsView::movePixmapsOfCurrentAsset(double dx, double dy)
{
    for (int j=0; j<assetLayer->geometries.count(); j++)
    {
        RwaMapItem *point1;
        point1 = (RwaMapItem *)assetLayer->geometries.at(j);

        RwaAsset1 *item = (RwaAsset1 *)point1->data;

        if( (item == currentAsset) && (point1->getRwaType() != RWAPOSITIONTYPE_ASSETSTARTPOINT) )
        {
            point1->move(dx, dy);
        }
    }

    moveReflectionPixmapsOfCurrentAsset(dx, dy);
}

void RwaGraphicsView::movePixmapsOfCurrentAssetChannel(double dx, double dy, int channel)
{
    for (int j=0; j<assetLayer->geometries.count(); j++)
    {
        RwaMapItem *point1;
        point1 = static_cast<RwaMapItem *>(assetLayer->geometries.at(j));

        RwaAsset1 *item = static_cast<RwaAsset1 *>(point1->data);

        if( (item == currentAsset) && (point1->getRwaType() == RWAPOSITIONTYPE_ASSETCHANNEL) && point1->getChannel() == channel)
        {
            point1->move(dx, dy);
        }
    }
}


void RwaGraphicsView::moveReflectionPixmapsOfCurrentAsset(double dx, double dy)
{
    for (int j=0; j<assetReflectionLayer->geometries.count(); j++)
    {
        RwaMapItem *point1;
        point1 = static_cast<RwaMapItem *>(assetReflectionLayer->geometries.at(j));

        RwaAsset1 *item = static_cast<RwaAsset1 *>(point1->data);

        if( (item == currentAsset) && (point1->getRwaType() == RWAPOSITIONTYPE_REFLECTIONPOSITION) )
        {
            point1->move(dx, dy);
        }
    }
}

void RwaGraphicsView::movePixmapsOfCurrentAssetReflection(double dx, double dy, int channel)
{
    for (int j=0; j<assetReflectionLayer->geometries.count(); j++)
    {
        RwaMapItem *point1;
        point1 = static_cast<RwaMapItem *>(assetReflectionLayer->geometries.at(j));

        RwaAsset1 *item = static_cast<RwaAsset1 *>(point1->data);

        if( (item == currentAsset) && (point1->getRwaType() == RWAPOSITIONTYPE_REFLECTIONPOSITION) && point1->getChannel() == channel)
        {
            point1->move(dx, dy);
        }
    }
}

void RwaGraphicsView::movePixmapsOfAssetReflections(double dx, double dy)
{
    for (int j=0; j<assetReflectionLayer->geometries.count(); j++)
    {
        RwaMapItem *point1;
        point1 = static_cast<RwaMapItem *>(assetReflectionLayer->geometries.at(j));

        if( point1->getRwaType() == RWAPOSITIONTYPE_REFLECTIONPOSITION)
            point1->move(dx, dy);
    }
}


void RwaGraphicsView::movePixmapsOfCurrentScene(double dx, double dy)
{
    QmapPoint *point1;


   // if(statesVisible)
    {
        for (int j=0; j<statesLayer->geometries.count(); j++)
        {
            point1 = static_cast<QmapPoint *>(statesLayer->geometries.at(j));

            RwaState *state = static_cast<RwaState *>(point1->data);
            if(state->getScene() == currentScene)
            {
                point1->move(dx, dy);
                if(assetsVisible)
                {
                    for (int j=0; j<assetLayer->geometries.count(); j++)
                    {
                        point1 = (QmapPoint *)assetLayer->geometries.at(j);

                        RwaAsset1 *item = (RwaAsset1 *)point1->data;
                        if(item->myState == state)
                            point1->move(dx, dy);
                    }
                }

                if(assetReflectionsVisible)
                {
                    for (int j=0; j<assetReflectionLayer->geometries.count(); j++)
                    {
                        point1 = (QmapPoint *)assetReflectionLayer->geometries.at(j);

                        RwaAsset1 *item = (RwaAsset1 *)point1->data;
                        if(item->myState == state)
                            point1->move(dx, dy);

                    }
                }
            }
        }
    }

    if(stateRadiusVisible)
    {
        for (int j=0; j<stateRadiusLayer->geometries.count(); j++)
        {
            point1 = (QmapPoint *)stateRadiusLayer->geometries.at(j);

            RwaState *state = (RwaState *)point1->data;
            if(state->getScene() == currentScene)
                point1->move(dx, dy);

        }
    }
}

void RwaGraphicsView::movePixmapsOfCurrentState(double dx, double dy)
{
    QmapPoint *point1;

    //if(statesVisible)
    {
        for (int j=0; j<statesLayer->geometries.count(); j++)
        {
            point1 = (QmapPoint *)statesLayer->geometries.at(j);

            RwaState *state = (RwaState *)point1->data;

            if(state == currentState )
            {
                point1->move(dx, dy);
            }
        }
    }

    if(stateRadiusVisible)
    {
        for (int j=0; j<stateRadiusLayer->geometries.count(); j++)
        {
            point1 = (QmapPoint *)stateRadiusLayer->geometries.at(j);

            RwaState *state = (RwaState *)point1->data;

            if(state == currentState )
            {
                point1->move(dx, dy);
            }
        }
    }

    if(assetsVisible)
    {
        if(currentState->childrenDoFollowMe())
        {

            for (int j=0; j<assetLayer->geometries.count(); j++)
            {
                point1 = (QmapPoint *)assetLayer->geometries.at(j);

                RwaAsset1 *item = (RwaAsset1 *)point1->data;

                if(item->myState == currentState )
                {
                    point1->move(dx, dy);
                }
            }

            if(assetReflectionsVisible)
                movePixmapsOfAssetReflections(dx, dy);
        }


    }

}

void RwaGraphicsView::updateAssetPixmaps()
{
    for (int i=0; i<assetLayer->geometries.count(); i++)
    {
        RwaMapItem *point;
        point = (RwaMapItem *)assetLayer->geometries.at(i);

        if(point->getRwaType() == RWAPOSITIONTYPE_ASSET)
        {
            if(point->data == currentAsset)
            {
                point->setPixmap(assetLayer->getActivePixmap());
            }
            else
            {
                point->setPixmap(assetLayer->getPassivePixmap());
            }
        }
    }
    if(assetLayer->isVisible())
        assetLayer->setVisible(true);
}

void RwaGraphicsView::updateReflectionPixmaps()
{
    for (int i=0; i<assetReflectionLayer->geometries.count(); i++)
    {
        RwaMapItem *point;
        point = (RwaMapItem *)assetReflectionLayer->geometries.at(i);

        if(point->getRwaType() == RWAPOSITIONTYPE_REFLECTIONPOSITION)
        {
            if(point->getChannel() == currentAsset->currentReflection)
            {
                point->setPixmap(assetReflectionLayer->getActivePixmap());
            }
            else
            {
                point->setPixmap(assetReflectionLayer->getPassivePixmap());
            }
        }
    }
    if(assetReflectionLayer->isVisible())
        assetReflectionLayer->setVisible(true);
}

void RwaGraphicsView::updateStatePixmaps()
{
    for (int i=0; i<statesLayer->geometries.count(); i++)
    {
        QmapPoint *point;
        QLineEdit *lineEdit;
        point = (QmapPoint *)statesLayer->geometries.at(i);
        if(point->data == currentState)
        {
            point->setPixmap(statesLayer->getActivePixmap());
            lineEdit = point->lineEdit();
            if(!backend->isSimulationRunning() && stateLineEditVisible)
            {
                point->setLineEditVisible(true);
                point->setLabelVisible(false);
            }
        }
        else
        {
            point->setPixmap(statesLayer->getPassivePixmap());
            lineEdit = point->lineEdit();
            point->setLineEditVisible(false);
            point->setLabelVisible(true);
        }
    }
}

void RwaGraphicsView::updatePixmaps(RwaLocation1 *active, GeometryLayer *layer)
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

void RwaGraphicsView::resizeArea(QPointF myPoint, RwaArea *currentArea)
{
    std::vector<double> myPoint1(2, 0.0);
    myPoint1[0] = myPoint.x();
    myPoint1[1] = myPoint.y();
    double newRadius = RwaUtilities::calculateDistance1(currentArea->getCoordinates(), myPoint1);
    std::vector<double> tmp = currentArea->getCoordinates();
    tmp[1] = (myPoint.y());
    double newWidth = RwaUtilities::calculateDistance1(tmp, myPoint1);
    tmp = currentArea->getCoordinates();
    tmp[0] = (myPoint.x());
    double newHeight = RwaUtilities::calculateDistance1(tmp, myPoint1);

    if(editAreaRadius)
        currentArea->setRadius(newRadius*1000);

    if( (editAreaWidth) && (currentArea->getAreaType() == RWAAREATYPE_SQUARE) )
    {
        currentArea->setWidth(newWidth*2000);
        currentArea->setHeight(newWidth*2000);
    }

    if( (editAreaHeight) && (currentArea->getAreaType() == RWAAREATYPE_SQUARE) )
    {
        currentArea->setWidth(newHeight*2000);
        currentArea->setHeight(newHeight*2000);
    }

    if( editAreaWidth && (currentArea->getAreaType() == RWAAREATYPE_RECTANGLE) )
    {
        currentArea->setWidth(newWidth*2000);
    }

    if( editAreaHeight && (currentArea->getAreaType() == RWAAREATYPE_RECTANGLE) )
    {
        currentArea->setHeight(newHeight*2000);
    }

    if( editAreaCorner && (currentArea->getAreaType() == RWAAREATYPE_POLYGON) )
    {
        std::vector<double> tmp(2, 0.0);
        tmp[0] = myPoint.x();
        tmp[1] = myPoint.y();
        currentArea->corners[areaCornerIndex2Edit] = tmp;
    }
}

bool RwaGraphicsView::mouseDownArea(QPointF myPoint, RwaArea *currentArea)
{
     double clickDistance;
     double radiusInPx;
     double bearing;
     QPointF somePoint;
     QPointF east, west, north, south;
     QPointF tmp(currentArea->getCoordinates()[0], currentArea->getCoordinates()[1]);

     clickDistance = getDistanceInPxFromGps(tmp, myPoint);
     bearing = RwaUtilities::calculateBearing(tmp, myPoint);

     north = RwaUtilities::calculateDestination(tmp, currentArea->getHeight()/2, 0);
     west = RwaUtilities::calculateDestination(tmp, currentArea->getWidth()/2, 90);
     south = RwaUtilities::calculateDestination(tmp, currentArea->getHeight()/2, 180);
     east = RwaUtilities::calculateDestination(tmp, currentArea->getWidth()/2, 270);

     north = mapadapter->coordinateToDisplay(north);
     east = mapadapter->coordinateToDisplay(east);
     south = mapadapter->coordinateToDisplay(south);
     west = mapadapter->coordinateToDisplay(west);
     somePoint = mapadapter->coordinateToDisplay(myPoint);

     if(currentArea->getAreaType() == RWAAREATYPE_CIRCLE)
     {
         radiusInPx = getDistanceInPxFromGps(tmp,
                     RwaUtilities::calculatePointOnCircle(tmp, currentArea->getRadius()));// * 0.5;

         if(fabs(clickDistance-radiusInPx) < 10)
         {
             mc->setMouseMode(MapControl::None);
             editAreaRadius = true;
             editArea = true;
             return true;
         }
     }

     if(currentArea->getAreaType() == RWAAREATYPE_SQUARE || currentArea->getAreaType() == RWAAREATYPE_RECTANGLE)
     {
         if( (fabs(somePoint.x() - east.x()) < 10)
                 && ( (somePoint.y() > north.y() && somePoint.y() < south.y() ) ) )
             editAreaWidth = true;

         if( (fabs(somePoint.x() - west.x()) < 10)
                 && ( (somePoint.y() > north.y() && somePoint.y() < south.y() ) ) )
             editAreaWidth = true;

         if( (fabs(somePoint.y() - north.y()) < 10)
                 && ( (somePoint.x() > west.x() && somePoint.x() < east.x() ) ) )
             editAreaHeight = true;

         if( (fabs(somePoint.y() - south.y()) < 10)
                 && ( (somePoint.x() > west.x() && somePoint.x() < east.x() ) ) )
             editAreaHeight = true;

         if(editAreaWidth || editAreaHeight)
         {
             mc->setMouseMode(MapControl::None);
             editArea = true;
             return true;
         }
     }

     if(currentArea->getAreaType() == RWAAREATYPE_POLYGON)
     {
         QPoint corner = QPoint();

         for(u_int64_t i = 0; i < currentArea->corners.size(); i++)
         {
             corner = mapadapter->coordinateToDisplay(QPointF(currentArea->corners[i][0], currentArea->corners[i][1]));
             if( (fabs(somePoint.x() - corner.x()) < 6)
                     && (fabs(somePoint.y() - corner.y()) < 6) )
             {
                 editAreaCorner = true;
                 editArea = true;
                 mc->setMouseMode(MapControl::None);
                 areaCornerIndex2Edit = i;

                 return true;
             }
         }
     }
     return false;
}

bool RwaGraphicsView::mouseDoubleClickArea(QPointF myPoint, RwaArea *currentArea)
{
    if(currentArea->getAreaType() == RWAAREATYPE_POLYGON)
    {
        std::vector<double> corner(2, 0.0);
        std::vector<double> tmp(2, 0.0);
        tmp[0] = myPoint.x();
        tmp[1] = myPoint.y();

        for(u_int32_t i = 0; i < currentArea->corners.size(); i++)
        {
            corner = currentArea->corners[i];
            if( (fabs(myPoint.x() - corner[0]) < 0.0001)
                    && (fabs(myPoint.y() - corner[1]) < 0.0001) )
            {
                currentArea->corners.erase(currentArea->corners.begin()+i);
                return true;
            }
        }

        u_int64_t lastCornerIndex = currentArea->corners.size() - 1;
        std::vector<double> *lastCorner = &currentArea->corners[lastCornerIndex];
        for(u_int32_t i = 0; i < currentArea->corners.size(); i++)
        {
            if(RwaUtilities::mouseDownOnPolygonVertex1(*lastCorner, currentArea->corners[i], myPoint))
            {
                mc->setMouseMode(MapControl::None);
                currentArea->corners.insert(currentArea->corners.begin()+i, tmp);
                return true;
            }
            lastCorner = &currentArea->corners[i];
        }
    }
    return false;
}

void RwaGraphicsView::receiveStateName(QString name)
{
    tmpObjectName = name;
   // emit sendStateName(currentState, name);
}

void RwaGraphicsView::updateCurrentState()
{
    if(currentState->objectName() != tmpObjectName.toStdString())
    {
        currentState->setObjectName(tmpObjectName.toStdString());
        emit sendWriteUndo("Renamed State");
    }

    updatePixmaps(currentStatePoint, statesLayer);
    if(stateRadiusVisible)
        drawArea(currentState, stateRadiusLayer);

    if(!(QObject::sender() == this->backend))
        emit sendCurrentScene(currentScene );
}

void RwaGraphicsView::workAroundLineEditBug()
{
    QTimer::singleShot(100, this, SLOT(updateCurrentState()));
}

void RwaGraphicsView::drawArea(RwaArea *area, GeometryLayer *layer, bool isActive)
{
    float radius;
    float width;
    float height;
    QPointF tmp(area->getCoordinates()[0], area->getCoordinates()[1]);

    if(area->getAreaType() == RWAAREATYPE_CIRCLE)
    {
        radius = getDistanceInPxFromGps(tmp,
                    RwaUtilities::calculatePointOnCircle(tmp, area->getRadius()));

        CirclePoint* stateArea = new CirclePoint(area->getCoordinates()[0], area->getCoordinates()[1], radius);
        stateArea->setData(area);
        layer->addGeometry(stateArea);
    }

    if(area->getAreaType() == RWAAREATYPE_RECTANGLE || area->getAreaType() == RWAAREATYPE_SQUARE)
    {
        width = getDistanceInPxFromGps(tmp,
                    RwaUtilities::calculatePointOnCircle(tmp, area->getWidth()/2));

        height = getDistanceInPxFromGps(tmp,
                    RwaUtilities::calculatePointOnCircle(tmp, area->getHeight()/2));

        QPointF northWest = RwaUtilities::calculateNorthWest(tmp, area->getWidth(), area->getHeight());
        QPointF southEast = RwaUtilities::calculateSouthEast(tmp, area->getWidth(), area->getHeight());

        //RectPoint *stateArea = new RectPoint(area->getCoordinates().x(), area->getCoordinates().y(), width, height);
        RectPoint *stateArea = new RectPoint(area->getCoordinates()[0], area->getCoordinates()[1], northWest, southEast);
        if(!isActive)
            stateArea->setPenColor(QColor(240,60,50,200));

        stateArea->setData( area);
        layer->addGeometry(stateArea);
    }

    if(area->getAreaType() == RWAAREATYPE_POLYGON)
    {

        if(area->corners.empty())
        {

        }
        else
        {
            PolygonPoint *stateArea = new PolygonPoint(area->getCoordinates()[0], area->getCoordinates()[1], &area->corners);
            if(!isActive)
                stateArea->setPenColor(QColor(240,60,50,200));
            stateArea->setData( area);
            layer->addGeometry(stateArea);
        }
    }
}

void RwaGraphicsView::drawScene(RwaScene *scene, bool isActive)
{
    QmapPoint* newPlace;

    if(isActive)
        newPlace = new QmapPoint(scene->getCoordinates()[0], scene->getCoordinates()[1], scenesLayer->getActivePixmap(), scene);
    else
        newPlace = new QmapPoint(scene->getCoordinates()[0], scene->getCoordinates()[1], scenesLayer->getPassivePixmap(), scene);

    scenesLayer->addGeometry(newPlace);
}

void RwaGraphicsView::drawState(RwaState *state, bool isActive)
{
    QmapPoint* newPlace;

    if(isActive)
        newPlace = new QmapPoint(state->getCoordinates()[0], state->getCoordinates()[1], statesLayer->getActivePixmap(), state);
    else
        newPlace = new QmapPoint(state->getCoordinates()[0], state->getCoordinates()[1], statesLayer->getPassivePixmap(), state);

    QLineEdit *stateName = new QLineEdit(mc);
    QLabel *stateNameLabel = new QLabel(mc);

    stateName->setText(QString::fromStdString(state->objectName()));
    stateName->setFixedWidth(80);
    stateName->setFixedHeight(21);
    stateNameLabel->setText(QString::fromStdString(state->objectName()));
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

    //qDebug() << "COORDINATES: " << newPlace->coordinate().x();
    //statesLayer->addGeometry(radiusCircle);
}

void RwaGraphicsView::drawAsset(RwaAsset1 *item, bool isActive)
{
    RwaMapItem *mapItem;

    if(item->getMute())
        return;

    if(isActive)
        mapItem = new RwaMapItem(QPointF(item->getCoordinates()[0],item->getCoordinates()[1]), item, RWAPOSITIONTYPE_ASSET, assetLayer->getActivePixmap());
    else
        mapItem = new RwaMapItem(QPointF(item->getCoordinates()[0],item->getCoordinates()[1]), item, RWAPOSITIONTYPE_ASSET, assetLayer->getPassivePixmap());

    mapItem->setAllowTouches(true);
    assetLayer->addGeometry(mapItem);

   // if(!backend->isSimulationRunning() && !item->individuellChannelPositions)
        //item->calculateChannelPositions(); // else they are calculated in the gameloop for visualisation purposes

    if(item->getMoveFromStartPosition())
    {
        if(assetStartPointsVisible)
        {
            mapItem = new RwaMapItem(QPointF(item->getStartPosition()[0], item->getStartPosition()[1]), item, RWAPOSITIONTYPE_ASSETSTARTPOINT, assetLayer->getPixmap5());
            mapItem->setAllowTouches(true);
            assetLayer->addGeometry(mapItem);
        }

        if(backend->isSimulationRunning())
        {
            QPointF tmp = QPointF(item->getCurrentPosition()[0], item->getCurrentPosition()[1]);
            mapItem = new RwaMapItem(tmp, item, RWAPOSITIONTYPE_CURRENTASSETPOSITION, assetLayer->getPixmap4());
            mapItem->setAllowTouches(false);
            assetLayer->addGeometry(mapItem);
        }
    }

    if(item->getChannelRadius() > 0)
    {
        if(item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALMONO
        || item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALMONO_FABIAN)
        {
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[0][0], item->channelcoordinates[0][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 0);
            assetLayer->addGeometry(mapItem);
        }

        if(item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSTEREO
        || item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN)
        {
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[0][0], item->channelcoordinates[0][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL,assetLayer->getPixmap3(), 0);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[1][0], item->channelcoordinates[1][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 1);
            assetLayer->addGeometry(mapItem);
        }

        if(item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL5CHANNEL
        || item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN)
        {
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[0][0], item->channelcoordinates[0][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 0);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[1][0], item->channelcoordinates[1][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 1);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[2][0], item->channelcoordinates[2][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 2);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[3][0], item->channelcoordinates[3][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(),3);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[4][0], item->channelcoordinates[4][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(),4);
            assetLayer->addGeometry(mapItem);
        }

        if(item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN)
        {
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[0][0], item->channelcoordinates[0][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 0);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[1][0], item->channelcoordinates[1][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 1);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[2][0], item->channelcoordinates[2][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(), 2);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[3][0], item->channelcoordinates[3][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(),3);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[4][0], item->channelcoordinates[4][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(),4);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[5][0], item->channelcoordinates[5][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(),5);
            assetLayer->addGeometry(mapItem);
            mapItem = new RwaMapItem(QPointF(item->channelcoordinates[6][0], item->channelcoordinates[6][1]), item, RWAPOSITIONTYPE_ASSETCHANNEL, assetLayer->getPixmap3(),6);
            assetLayer->addGeometry(mapItem);
        }
    }

    if(item->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSPACE)
    {
        if(assetReflectionsVisible)
        {
            for(int i = 0;i < item->getReflectionCount(); i++)
            {
                if(i == item->currentReflection)
                    mapItem = new RwaMapItem(QPointF(item->reflectioncoordinates[i][0], item->reflectioncoordinates[i][1]), item, RWAPOSITIONTYPE_REFLECTIONPOSITION, assetReflectionLayer->getActivePixmap(), i);
                else
                    mapItem = new RwaMapItem(QPointF(item->reflectioncoordinates[i][0], item->reflectioncoordinates[i][1]), item, RWAPOSITIONTYPE_REFLECTIONPOSITION, assetReflectionLayer->getPassivePixmap(), i);

                assetReflectionLayer->addGeometry(mapItem);
            }
        }
    }
}

void RwaGraphicsView::drawEntity(RwaEntity *entity, bool isActive)
{
    QmapPoint* newPlace;

    if(isActive)
        newPlace = new QmapPoint(entity->getCoordinates()[0], entity->getCoordinates()[1], entityLayer->getActivePixmap(), entity);
    else
        newPlace = new QmapPoint(entity->getCoordinates()[0], entity->getCoordinates()[1], entityLayer->getPassivePixmap(), entity);

    entityLayer->addGeometry(newPlace);
}

void RwaGraphicsView::redrawAssets()
{
    if(assetReflectionsVisible)
        assetReflectionLayer->clearGeometries();

    if(assetsVisible)
    {
        assetLayer->clearGeometries();
        RwaState *state;
        RwaAsset1 *item;

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
    assetReflectionLayer->clearGeometries();

    RwaState *state;
    RwaAsset1 *item;

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
                drawScene(scene);
        }
    }
}

void RwaGraphicsView::redrawStates()
{
    if(backend->logOther)
        qDebug();

    statesLayer->clearGeometries();
    RwaState *state;
    foreach (state, currentScene->getStates())
    {
        //if(state->isGpsState)
        {
             if(state != currentState)
                drawState(state);
             else
                 drawState(state, 1);
        }
    }
}

void RwaGraphicsView::resizeSceneRadii()
{

    for (int j=0; j<sceneRadiusLayer->geometries.count(); j++)
    {

        QmapPoint *point1 = (QmapPoint *)statesLayer->geometries.at(j);

        point1->setVisible(true);
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
         {
            if(scene == currentScene)
                drawArea(scene, sceneRadiusLayer, true);
            else
                drawArea(scene, sceneRadiusLayer, false);
         }
    }
}

void RwaGraphicsView::redrawStateRadii()
{
    if(backend->logOther)
        qDebug();

    stateRadiusLayer->clearGeometries();
    RwaState *state;
    RwaScene *scene;
    int currentLevel = currentScene->getLevel();
    foreach (scene, backend->getScenes())
    {
        if(scene->getLevel() == currentLevel)
        {
            foreach (state, scene->getStates())
            {
                if(state->isGpsState)
                {
                     if(scene == currentScene)
                        drawArea(state, stateRadiusLayer, true);
                     else
                         drawArea(state, stateRadiusLayer, false);
                }
            }
        }
    }
}

void RwaGraphicsView::redrawEntities()
{
    for (int i=0; i<backend->simulator->entities.count(); i++)
    {
        RwaEntity *entity = backend->simulator->entities.at(i);
        if(!entity)
            return;

        entity->setCoordinates(currentScene->getCoordinates());
        drawEntity(entity, true);
    }
}

void RwaGraphicsView::setEntityCoordinates()
{
    for (int i=0; i<backend->simulator->entities.count(); i++)
    {
        RwaEntity *entity = backend->simulator->entities.at(i);
        if(!entity)
            return;

        entity->setCoordinates(currentScene->getCoordinates());
    }
}


