#ifndef RWAVIEW_H
#define RWAVIEW_H

#include <QtWidgets>
#include <QDockWidget>
#include <QNetworkAccessManager>
#include <QGraphicsView>
#include <QMenuBar>
#include <QtNetwork>
#include <QBoxLayout>
#include "rwatoolbar.h"
#include "rwabackend.h"
#include "rwaentity.h"
#include "rwalistview.h"
#include "qmapcontrol/QMapControl/qmapcontrol.h"

using namespace qmapcontrol;

class RwaView : public QGraphicsView
{
    Q_OBJECT

    friend class RwaListView;

public:
    explicit RwaView(QWidget *parent = nullptr, RwaScene *scene = nullptr, QString name = "");

    RwaScene *getCurrentScene();
    RwaState *getCurrentState();
    RwaEntity *getCurrentEntity();
    RwaAsset1 *getCurrentAsset();

signals:
    void sendCurrentState(RwaState *currentState);
    void sendCurrentScene(RwaScene *currentScene);
    void sendCurrentAsset(RwaAsset1 *currentAsset);
    void sendCurrentEntity(RwaEntity *currentEntity);
    void sendWriteUndo();
    void sendWriteUndo(QString undoAction);

public slots:

    void receiveClearAll();
    void updateSceneMenus();

    virtual void setCurrentState(RwaState *state);
    virtual void setCurrentState(qint32 stateNumber);
    virtual void setCurrentScene(RwaScene *scene);
    virtual void setCurrentScene(qint32 sceneNumber);
    virtual void setCurrentAsset(RwaAsset1 *currentAsset);
    virtual void setCurrentEntity(RwaEntity *currentEntity);
    virtual void adaptSize(qint32 width, qint32 height) = 0;

protected:

    RwaViewToolbar *setupToolbar(qint32 flags);
    RwaBackend *backend;
    RwaState *currentState;
    RwaState *lastState;
    RwaScene *currentScene;
    RwaEntity *currentEntity;
    RwaAsset1 *currentAsset;
    RwaAsset1 *lastAsset;
    RwaViewToolbar *toolbar;
    QSplitter *windowSplitter = nullptr;

    bool dirty;
    QString undoAction;

    void writeUndo();
    void logLocationCoordinates(RwaLocation1 *location);
    void copyLocationCoordinates2Clipboard(RwaLocation1 *location);

public:
    QGridLayout *grid = nullptr;
    QBoxLayout *layout = nullptr;
    QBoxLayout *topLayout = nullptr;
    QBoxLayout *innerLayout = nullptr;
    QBoxLayout *leftLayout = nullptr;
    QBoxLayout *rightLayout = nullptr;

    qint32 tool;

    void setUndoAction(const QString &value);
    void readSplitterLayout();
    void writeSplitterLayout();
};

#endif // RWAVIEW_H
