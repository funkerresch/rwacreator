#include "rwasceneview.h"


RwaSceneView::RwaSceneView(QWidget *parent, RwaScene *scene) :
    RwaView(parent, scene)

{
    setAcceptDrops(true);
    setAlignment(Qt::AlignTop);
    windowSplitter = new QSplitter(this);
    layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    layout->setContentsMargins(0,0,0,0);
    setMinimumSize(450,100);


    stateList = new RwaStateList(this, scene);
    stateAttributes = new RwaStateAttributeView(this, scene);
    stateAttributes->scrollArea->setMaximumWidth(250);

    currentScene = scene;
    currentState = NULL;

    windowSplitter->addWidget(stateAttributes->scrollArea);
    windowSplitter->addWidget(stateList);

    layout->addWidget(windowSplitter);

    connect(stateList, SIGNAL(deleteState(const QString &)),
              this, SLOT(deleteState(const QString &)));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
            backend, SLOT(receiveLastTouchedScene(RwaScene*)));
}

void RwaSceneView::deleteState(const QString &stateName)
{

    if(currentScene)
    {
       // qDebug() << "delete" << stateName;
        currentScene->removeState(currentScene->getState(stateName.toStdString()));
        emit sendCurrentScene(currentScene);
    }
}

void RwaSceneView::adaptSize(qint32 width, qint32 height)
{
    //stateAttributes->setFixedWidth(width*0.55);
    //stateList->setFixedWidth(width*0.35);
}

void RwaSceneView::setCurrentScene(RwaScene *currentScene)
{
    this->currentScene = currentScene;
    //this->stateList->setCurrentScene(currentScene);
    stateList->update();
}

void RwaSceneView::setCurrentState(RwaState *currentState)
{
    this->currentState = currentState;
    //stateList->update();
}

