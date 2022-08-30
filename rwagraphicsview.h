#ifndef RWAGRAPHICSVIEW_H
#define RWAGRAPHICSVIEW_H

#include "rwaview.h"

class RwaGraphicsView : public RwaView
{
    Q_OBJECT
public:
    RwaGraphicsView(QWidget *parent = nullptr, RwaScene *scene = nullptr, QString name = "");
    MapControl* mc;

    virtual void receiveMouseDownEvent(const QMouseEvent*, const QPointF) = 0;
    virtual void receiveMouseMoveEvent(const QMouseEvent*, const QPointF) = 0;
    virtual void receiveMouseReleaseEvent() = 0;

    void drawArea(RwaArea *area, GeometryLayer *layer, bool isActive = 0);
    void drawScene(RwaScene *scene, bool isActive = 0);
    void drawState(RwaState *state, bool isActive = 0);
    void drawEntity(RwaEntity *entity, bool isActive = 0);
    void drawAsset(RwaAsset1 *item, bool isActive = 0);

    void redrawState(QmapPoint *statePoint);
    void redrawArea(QmapPoint *area);
    void redrawAsset(QmapPoint *assetPoint);
    void updateStatePixmaps();
    void updateAssetPixmaps();

    bool mouseDownArea(QPointF myPoint, RwaArea *currentArea);
    void resizeArea(QPointF myPoint, RwaArea *currentArea);
    bool mouseDoubleClickArea(QPointF myPoint, RwaArea *currentArea);


public slots:
    void updateCurrentState();
    void movePixmapsOfCurrentAsset(double dx, double dy);
    void movePixmapsOfCurrentState(double dx, double dy);
    void movePixmapsOfCurrentAssetChannel(double dx, double dy, int channel);
    void movePixmapsOfCurrentAssetReflection(double dx, double dy, int channel);
    void setAssetsVisible(bool assetsVisible);
    void setRadiiVisible(bool onOff);
    void setEntitiesVisible(bool entitiesVisible);
    void setStatesVisible(bool statesVisible);
    void setStateRadiusVisible(bool radiusVisible);
    void setEntityCoordinates2CurrentScene();
    void setEntityCoordinates2CurrentState();
    void setTool(qint32 tool);
    void initNewGame();

    void redrawAssets();
    void redrawAssetsOfCurrentState();
    void redrawRadiusOfCurrentState();
    void redrawStates();
    void redrawScenes();
    void redrawStateRadii();
    void redrawSceneRadii();
    void redrawEntities();

    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void adaptSize(qint32 width, qint32 height) = 0;

    void receiveStateName(QString name);
    void workAroundLineEditBug();

    void movePixmapsOfCurrentScene(double dx, double dy);
    void resizeSceneRadii();

    void movePixmapsOfAssetReflections(double dx, double dy);
    void moveReflectionPixmapsOfCurrentAsset(double dx, double dy);

protected:
    void updatePixmaps(QmapPoint *active, GeometryLayer *layer);
    void updatePixmaps(RwaLocation1 *active, GeometryLayer *layer);

    double getDistanceInPxFromGps(QPointF p1, QPointF p2);
    double getDistanceInPxFromScreen(QPointF p1, QPointF p2);

    MapAdapter* mapadapter;
    //ImageMapAdapter *imageMapAdapter;
    MapLayer *l;
    Layer *imageLayer;

    GeometryLayer *scenesLayer;
    GeometryLayer *sceneRadiusLayer;
    GeometryLayer *statesLayer;
    GeometryLayer *stateRadiusLayer;
    GeometryLayer *entityLayer;
    GeometryLayer *assetLayer;
    GeometryLayer *assetReflectionLayer;

    QmapPoint *currentRadiusPoint = nullptr;
    QmapPoint *currentScenePoint = nullptr;
    QmapPoint *currentStatePoint = nullptr;
    QmapPoint *currentEntityPoint = nullptr;
    QmapPoint *currentAssetPoint = nullptr;
    QmapPoint *currentPolygonPoint = nullptr;
    QmapPoint *currentAssetReflection = nullptr;

    QPushButton *zoomInButton;
    QPushButton *zoomOutButton;
    QPointF mapCoordinates;

    void addZoomButtons();

    bool stateLineEditVisible = false; // allow editing object name via GUI if active (i.e. last touched)
    bool sceneLineEditVisible = false;
    bool entityLineEditVisible = false;
    bool assetLineEditVisible = false;
    bool onlyAssetsOfCurrentStateVisible = false;
    bool onlyRadiiOfCurrentStateVisible = false;
    bool entityVisible = false;
    bool statesVisible = false;
    bool scenesVisible = false;
    bool stateRadiusVisible = false;
    bool sceneRadiusVisible = false;
    bool assetStartPointsVisible = false;
    bool assetReflectionsVisible = false;

    bool entityInitialized = false;
    bool heroFollowsSceneAndState = false;
    bool editArea = false;
    bool editAreaWidth = false;
    bool editAreaHeight = false;
    bool editAreaRadius = false;
    bool editAreaCorner = false;
    bool editStateArea = false;
    bool editSceneArea = false;

    u_int64_t areaCornerIndex2Edit = false;
    QString tmpObjectName;
    RwaState *tmpState = nullptr;

    void setMap2AreaZoomLevel(RwaArea *area);
public:
    bool assetsVisible;

    void updateReflectionPixmaps();
signals:
    void sendCurrentStateEdited();
    void sendCurrentStateRadiusEdited();
    void sendCurrentSceneRadiusEdited();
    void sendMoveCurrentScene();
    void sendMoveCurrentState();
    void sendMoveCurrentState1(double dx, double dy);
    void sendMovePixmapsOfCurrentAsset1(double dx, double dy);
    void sendMoveCurrentAssetChannel(double dx, double dy, int channel);
    void sendMoveCurrentAssetReflection(double dx, double dy, int reflectionNumber);
    void sendMoveCurrentAsset();
    void sendRedrawAssets();
    void sendRedrawStates();
    void sendEntityPosition(QPointF position);
    void sendMapPosition(QPointF position);
    void sendStateName(RwaState *state, QString name);
protected slots:
    void setCurrentScene(RwaScene *scene);
    void setCurrentState(RwaState *state);
    void setCurrentAsset(RwaAsset1 *currentAsset);

};

#endif // RWAGRAPHICSVIEW_H
