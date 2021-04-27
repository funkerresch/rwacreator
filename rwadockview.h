#ifndef RWAVIEW_H
#define RWAVIEW_H

#include <QtWidgets>
#include <QDockWidget>
#include <QNetworkAccessManager>
#include <QGraphicsView>
#include <QMenuBar>
#include <QtNetwork>
#include <QBoxLayout>
#include "toolbar.h"
#include "rwabackend.h"
#include "rwaentity.h"
#include "rwadockwidget.h"
#include "../qmapcontrol/QMapControl/qmapcontrol.h"

using namespace qmapcontrol;

class RwaView : public RwaDockWidget
{
    Q_OBJECT

public:
    explicit RwaView(QWidget *parent = 0, RwaScene *scene = NULL, RwaBackend *backend = NULL);

    RwaScene *getCurrentScene();
    RwaState *getCurrentState();
    RwaEntity *getCurrentEntity();
    RwaAssetItem *getCurrentAsset();

signals:
    void sendCurrentState(RwaState *currentState);
    void sendCurrentScene(RwaScene *currentScene);
    void sendCurrentAsset(RwaAssetItem *currentAsset);
    void sendCurrentEntity(RwaEntity *currentEntity);

public slots:

    void receiveClearAll();
    void updateSceneMenus();

    virtual void setCurrentState(RwaState *currentState);
    virtual void setCurrentState(qint32 stateNumber);
    virtual void setCurrentScene(RwaScene *currentScene);
    virtual void setCurrentScene(qint32 sceneNumber);
    virtual void setCurrentAsset(RwaAssetItem *currentAsset);
    virtual void setCurrentEntity(RwaEntity *currentEntity);
    virtual void adaptSize(qint32 width, qint32 height) = 0;

protected:

    RwaViewToolbar *setupToolbar(qint32 flags);

    RwaState *currentState;
    RwaScene *currentScene;
    RwaEntity *currentEntity;
    RwaAssetItem *currentAsset;
    RwaViewToolbar *toolbar;

    bool writeUndo;
    QBoxLayout *layout;
    QBoxLayout *topLayout;
    QBoxLayout *innerLayout;
    QBoxLayout *leftLayout;
    QBoxLayout *rightLayout;

    qint32 tool;
private:
};

#endif // RWAVIEW_H
