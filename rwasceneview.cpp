#include "rwasceneview.h"

#include <QDebug>

RwaSceneView::RwaSceneView(QWidget *parent, RwaScene *scene, QString name)
: RwaView(parent, scene, name)
{
    setAcceptDrops(true);
    setAlignment(Qt::AlignTop);

    windowSplitter = new QSplitter(this);

    layout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    layout->setContentsMargins(0,0,0,0);

    stateList = new RwaStateList(this, scene);
    stateAttributes = new RwaStateAttributeView(this, scene);
    stateAttributes->scrollArea->setFixedWidth(260);
    stateAttributes->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
    if(backend->isSimulationRunning())
    {
        // the entity's currentState would keep a pointer to the deleted state
        qWarning() << "Cannot delete a state while the simulation is running.";
        return;
    }

    if(!currentScene)
        return;

    RwaState *state = currentScene->getState(stateName.toStdString());
    if(!state)
        return;

    currentScene->removeState(state);
    emit sendCurrentScene(currentScene);
    emit sendCurrentState(currentScene->lastTouchedState);
    emit sendWriteUndo("Delete State");
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
