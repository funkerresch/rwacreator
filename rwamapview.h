#ifndef RWAMAPVIEW_H
#define RWAMAPVIEW_H

#include "rwagraphicsview.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class RwaMapView : public RwaGraphicsView
{
    Q_OBJECT
public:

    explicit RwaMapView(QWidget* parent = nullptr, RwaScene *scene = nullptr, QString name = "");

    void mouseDownArrow(const QMouseEvent *event, const QPointF myPoint);
    void mouseDownRubber(const QMouseEvent *event, const QPointF myPoint);
    void mouseDownMarkee(const QMouseEvent *event, const QPointF myPoint);

    float selectRectX;
    float selectRectY;

    void moveCurrentState1(const QPointF myPoint);
    void moveCurrentScene(const QPointF myPoint);
    bool mouseDownScenes(const QPointF myPoint);

private:
    int stateIndex2Delete;

public slots:
    void receiveMouseMoveEvent(const QMouseEvent*, const QPointF);
    void receiveMouseReleaseEvent();
    void receiveMouseDownEvent(const QMouseEvent*, const QPointF);
    void receiveSelectRect(QRectF selectRect);
    void receiveStartStopSimulator(bool startStop);
    void receiveAppendScene();
    void receiveClearScene();
    void receiveDuplicateScene();
    void receiveRemoveScene();
    void receiveSceneName(QString name);
    void receiveUpdateCurrentStateRadius();
    //void receiveUpdateCurrentSceneRadius();
    void setAssetsVisible(bool onOff);
    void setRadiiVisible(bool onOff);

    bool mouseDownStates(const QPointF myPoint);
    bool mouseDownEntities(const QPointF myPoint);

    void setMapCoordinates(double lon, double lat);
    void setMapCoordinates(QPointF coordinates);
    void setZoomLevel(qint32 zoomLevel);
    void zoomIn();
    void zoomOut();
    void adaptSize(qint32 width, qint32 height);
    void moveCurrentAsset();

    void receiveUpdateCurrentSceneRadius();
    void receiveHeroPositionEdited();
protected:
     void setCurrentScene(RwaScene *scene);
     void setCurrentState(RwaState *state);
     void setCurrentState(qint32 stateNumber);
     void setCurrentAsset(RwaAsset1 *asset);
     //void setCurrentAsset(RwaAssetItem *currentAsset);

signals:
     void sendSceneName(RwaScene *scene, QString name);
     void sendMapCoordinates(double lon, double lat);
     void sendAppendScene();
     void sendClearScene(RwaScene *scene);
     void sendDuplicateScene(RwaScene *scene);
     void sendRemoveScene(RwaScene *scene);
     void sendStartStopSimulator(bool startStop);
     void sendStateCoordinate(QPointF);
     void sendSelectedStates(QStringList states);


};

#endif // RWAMAPVIEW_H
