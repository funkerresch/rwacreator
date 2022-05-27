#include "RwaGameView.h"

RwaGameView::RwaGameView(QWidget *parent, RwaScene *scene, QString name) :
    RwaView(parent, scene, name)

{
    setAcceptDrops(true);
    setAlignment(Qt::AlignTop);
    windowSplitter = new QSplitter(this);
    layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    layout->setContentsMargins(0,0,0,0);

    sceneList = new RwaSceneList(this, scene);
    sceneAttributes = new RwaSceneAttributeView(this, scene);

    currentScene = scene;
    currentState = nullptr;

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

    readSplitterLayout();
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
    (void) width; // silence warnings
    (void) height;
}

void RwaGameView::setCurrentScene(RwaScene *scene)
{
    if(this->currentScene != scene)
    {
        this->currentScene = scene;
        //this->sceneList->setCurrentScene(currentScene);
        //sceneList->update();

        if(backend->logOther)
            qDebug();
    }
}


