#include "RwaGameView.h"


RwaGameView::RwaGameView(QWidget *parent, RwaScene *scene) :
    RwaView(parent, scene)

{
    setAcceptDrops(true);
    setAlignment(Qt::AlignTop);
    windowSplitter = new QSplitter(this);
    layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    layout->setContentsMargins(0,0,0,0);
    setMinimumSize(450,100);

    sceneList = new RwaSceneList(this, scene);
    sceneAttributes = new RwaSceneAttributeView(this, scene);
    sceneAttributes->scrollArea->setMaximumWidth(250);

    currentScene = scene;
    currentState = NULL;

    windowSplitter->addWidget(sceneAttributes->scrollArea);
    windowSplitter->addWidget(sceneList);

    layout->addWidget(windowSplitter);

    connect(sceneList, SIGNAL(deleteScene(const QString &)),
              this, SLOT(deleteScene(const QString &)));

    disconnect(backend, SIGNAL(sendLastTouchedState(RwaState *)),
              this, SLOT(setCurrentState(RwaState*)));

    disconnect(backend, SIGNAL(sendLastTouchedAsset(RwaAsset1 *)),
              this, SLOT(setCurrentAsset(RwaAsset1*)));

    disconnect(backend, SIGNAL(sendLastTouchedEntity(RwaEntity *)),
              this, SLOT(setCurrentEntity(RwaEntity*)));
}

void RwaGameView::deleteScene(const QString &sceneName)
{
    if(currentScene)
    {
        backend->removeScene(currentScene);
        //emit sendCurrentScene(currentScene);
    }
}

void RwaGameView::adaptSize(qint32 width, qint32 height)
{
    //stateAttributes->setFixedWidth(width*0.55);
    //stateList->setFixedWidth(width*0.35);
}

void RwaGameView::setCurrentScene(RwaScene *currentScene)
{
    if(this->currentScene != currentScene)
    {
        this->currentScene = currentScene;
        //this->sceneList->setCurrentScene(currentScene);
        sceneList->update();

        if(backend->logOther)
            qDebug();
    }
}


