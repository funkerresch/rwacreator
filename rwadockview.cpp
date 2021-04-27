#include "rwaview.h"

RwaView::RwaView(QWidget *parent, RwaScene *scene, RwaBackend *backend) :
    RwaDockWidget(parent, backend)
{
    this->currentScene = scene;
    this->tool = RWATOOL_ARROW;
    this->writeUndo = false;
    setFrameRect(parent->rect());

    connect(backend, SIGNAL(sendClearAll()),
              this, SLOT(receiveClearAll()));

    connect(backend, SIGNAL(sendLastTouchedState(RwaState *)),
              this, SLOT(setCurrentState(RwaState*)));

    connect(backend, SIGNAL(sendLastTouchedScene(RwaScene *)),
              this, SLOT(setCurrentScene(RwaScene*)));

    connect(backend, SIGNAL(sendLastTouchedAsset(RwaAssetItem *)),
              this, SLOT(setCurrentAsset(RwaAssetItem*)));

    connect(backend, SIGNAL(sendLastTouchedEntity(RwaEntity *)),
              this, SLOT(setCurrentEntity(RwaEntity*)));
}

RwaViewToolbar* RwaView::setupToolbar(qint32 flags) // toolbar in menu..
{
    toolbar = new RwaViewToolbar(tr("Rwa View Toolbar"), flags, backend, this);
    toolbar->updateSelectSceneMenu();
    return toolbar;
}

void RwaView::updateSceneMenus()
{
    toolbar->updateSelectSceneMenu();
    toolbar->updateSelectStateMenu();
    qDebug() << "VIEW RECEIVED UPDATE SCENE MENUS";
}


void RwaView::receiveClearAll()
{
    this->currentScene = backend->getFirstScene();
    this->currentState = NULL;
    setCurrentScene(currentScene);
}

RwaState *RwaView::getCurrentState()
{
    return currentState;
}

RwaScene *RwaView::getCurrentScene()
{
    return currentScene;
}

RwaEntity *RwaView::getCurrentEntity()
{
    return currentEntity;
}

RwaAssetItem *RwaView::getCurrentAsset()
{
    return currentAsset;
}

void RwaView::setCurrentState(RwaState *currentState)
{
    this->currentState = currentState;
}

void RwaView::setCurrentState(qint32 stateNumber)
{
    if(!currentScene)
        return;
    if(!currentScene->getStateCounter())
        return;

    currentState = currentScene->getStates().at(stateNumber);
}

void RwaView::setCurrentScene(RwaScene *currentScene)
{
    this->currentScene = currentScene;

}

void RwaView::setCurrentScene(qint32 sceneNumber)
{
    if(sceneNumber <= backend->getNumberOfScenes())
        setCurrentScene(backend->getSceneAt(sceneNumber));
}

void RwaView::setCurrentEntity(RwaEntity *currentEntity)
{
    this->currentEntity = currentEntity;
}

void RwaView::setCurrentAsset(RwaAssetItem *currentAsset)
{
    this->currentAsset = currentAsset;
}
