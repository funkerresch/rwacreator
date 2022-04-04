#ifndef RWAASSETVIEW_H
#define RWAASSETVIEW_H

#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QComboBox>
#include <QListWidget>
#include <QMediaPlayer>
#include <qmediaplayer.h>
#include "rwaassetlist.h"
#include "rwagraphicsview.h"
#include "rwaassetattributeview.h"

class RwaStateView : public RwaGraphicsView
{
    Q_OBJECT
public:
    explicit RwaStateView(QWidget* parent = 0, RwaScene *scene = 0);

    void zoomIn();
    void zoomOut();

private:

    QmapPoint *currentPoint;
    RwaMapItem *currentMapItem = nullptr;
    CirclePoint *currentRadius;
    RectPoint *currentRect;
    RwaAssetList *assetList;
    RwaAssetAttributeView *assetAttributes;

public slots:
    void receiveMouseMoveEvent(const QMouseEvent*, const QPointF);
    void receiveMouseReleaseEvent();
    void receiveMouseDownEvent(const QMouseEvent*, const QPointF);
    void dropEvent(QDropEvent *event);
    void handleSplitter(int pos, int index);
    void setMapCoordinates(double lon, double lat);
    void setMapCoordinates(QPointF coordinates);
    void setZoomLevel(qint32 zoomLevel);
    void adaptSize(qint32 width, qint32 height);
    void moveCurrentState();
    void setCurrentState(RwaState *currentState);
    void setCurrentAsset(RwaAsset1 *currentAsset);
    void addAssetItem(const QString &path, qint32 type);
    void deleteAssetItem(const QString &path);
    int getNumberOfSelectedAssets();
    void redrawStateRadii();
    void receiveUpdateCurrentStateRadius();
    void receiveNewGameSignal();

protected:
    void keyPressEvent(QKeyEvent *event);

signals:

    void sendNewAsset(RwaState *currentState, RwaAsset1 *item);

private slots:
    void correctMapView();
private:
    bool editStateRadius;
    bool editStateWidth;
    bool editStateHeight;
    bool editStatePosition;

};

#endif // RWAASSETVIEW_H
