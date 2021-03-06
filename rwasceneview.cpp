#include "rwasceneview.h"


RwaSceneView::RwaSceneView(QWidget *parent, RwaScene *scene, QString name) :
    RwaView(parent, scene, name)

{
    setAcceptDrops(true);
    setAlignment(Qt::AlignTop);
    windowSplitter = new QSplitter(this);
    layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    layout->setContentsMargins(0,0,0,0);

    stateList = new RwaStateList(this, scene);
    stateAttributes = new RwaStateAttributeView(this, scene);
    stateAttributes->scrollArea->setMaximumWidth(250);

    currentScene = scene;
    currentState = nullptr;

    windowSplitter->addWidget(stateAttributes->scrollArea);
    windowSplitter->addWidget(stateList);
    layout->addWidget(windowSplitter);

    connect(stateList, SIGNAL(deleteState(const QString &)),
              this, SLOT(deleteState(const QString &)));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
            backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
            backend, SLOT(receiveLastTouchedState(RwaState*)));

    readSplitterLayout();
}

void RwaSceneView::deleteState(const QString &stateName)
{
    if(currentScene)
    {
        currentScene->removeState(currentScene->getState(stateName.toStdString()));
        emit sendCurrentScene(currentScene);
        emit sendCurrentState(currentScene->lastTouchedState);
        emit sendWriteUndo("Delete State");
    }
}

void RwaSceneView::setCurrentScene(RwaScene *scene)
{
    currentScene = scene;
}

void RwaSceneView::setCurrentState(RwaState *currentState)
{
    this->currentState = currentState;
    //stateList->update();
}

