#include "rwaview.h"

RwaView::RwaView(QWidget *parent, RwaScene *scene, QString name) :
    QGraphicsView(parent)
{
    this->setObjectName(name);
    this->backend = RwaBackend::getInstance();
    this->currentScene = scene;
    this->currentState = scene->getStates().front();
    this->tool = RWATOOL_ARROW;
    this->dirty = false;
    this->setFrameStyle(QFrame::NoFrame);

    setFrameRect(parent->rect());

    connect(backend, SIGNAL(sendClearAll()),
              this, SLOT(receiveClearAll()));

    connect(backend, SIGNAL(sendLastTouchedState(RwaState *)),
              this, SLOT(setCurrentState(RwaState*)));

    connect(backend, SIGNAL(sendLastTouchedScene(RwaScene *)),
              this, SLOT(setCurrentScene(RwaScene*)));

    connect(backend, SIGNAL(sendLastTouchedAsset(RwaAsset1 *)),
              this, SLOT(setCurrentAsset(RwaAsset1*)));

    connect(backend, SIGNAL(sendLastTouchedEntity(RwaEntity *)),
              this, SLOT(setCurrentEntity(RwaEntity*)));

    connect(this, SIGNAL(sendWriteUndo(QString)), backend, SLOT(receiveWriteUndo(QString)));
}

RwaView::~RwaView()
{
    writeSplitterLayout();
}

void RwaView::writeSplitterLayout()
{
    if(!windowSplitter)
        return;

    QSettings settings("Intrinsic Audio", "Rwa Creator");
    settings.setValue(objectName()+"Geometry", windowSplitter->saveGeometry());
    settings.setValue(objectName()+"State", windowSplitter->saveState());
}

void RwaView::readSplitterLayout()
{
    if(!windowSplitter || objectName().isEmpty())
        return;

    QSettings settings("Intrinsic Audio", "Rwa Creator");
    windowSplitter->restoreGeometry(settings.value(objectName()+"Geometry").toByteArray());
    windowSplitter->restoreState(settings.value(objectName()+"State").toByteArray());
}

RwaViewToolbar* RwaView::setupToolbar(qint32 flags) // toolbar in menu..
{
    toolbar = new RwaViewToolbar(tr("Rwa View Toolbar"), flags, backend, this);
    toolbar->updateSelectSceneMenu();
    return toolbar;
}

void RwaView::writeUndo()
{
    if(dirty)
    {
        dirty = false;
        emit sendWriteUndo(undoAction);
    }
}

void RwaView::setUndoAction(const QString &value)
{
    undoAction = value;
    dirty = true;
}

void RwaView::logLocationCoordinates(RwaLocation1 *location)
{
    if(backend->logCoordinates)
        qDebug() << "Lon: "<< QString::number(location->getCoordinates()[0], 'f', 8) << " Lat: " << QString::number(location->getCoordinates()[1], 'f', 8);
}

void RwaView::copyLocationCoordinates2Clipboard(RwaLocation1 *location)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text = QString("%1 %2").arg(location->getCoordinates()[0]).arg(location->getCoordinates()[1]);
    clipboard->setText(text);
}

void RwaView::updateSceneMenus()
{
    qDebug() << "SCENE NAME UPDATE";
   // currentScene->setObjectName()
    toolbar->updateSelectSceneMenu();
    toolbar->updateSelectStateMenu();
}

void RwaView::receiveClearAll()
{
    this->currentScene = backend->getFirstScene();
    this->currentState = nullptr;
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

RwaAsset1 *RwaView::getCurrentAsset()
{
    return currentAsset;
}

void RwaView::setCurrentState(RwaState *state)
{
    if(!state)
        return;

    lastState = currentState;
    currentState = state;
    currentScene->lastTouchedState = state;

    if(currentState->getAssets().size() > 0)
    {
        if(!currentState->getLastTouchedAsset())
            currentState->lastTouchedAsset = currentState->getAssets().front();

        currentAsset = currentState->getLastTouchedAsset();
    }
}

void RwaView::setCurrentState(qint32 stateNumber)
{
    if(!currentScene)
        return;
    if(currentScene->getStates().empty())
        return;

    lastState = this->currentState;
    auto statesList = currentScene->getStates().begin();
    std::advance(statesList, stateNumber);
    currentState = *statesList;
}

void RwaView::setCurrentScene(RwaScene *scene)
{
    if(!scene)
        return;

    if(currentScene == scene)
        return;

    currentScene = scene;
    if(!currentScene->lastTouchedState)
        currentScene->lastTouchedState = currentScene->getStates().front();
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

void RwaView::setCurrentAsset(RwaAsset1 *asset)
{
    if(!currentState)
        return;

    lastAsset = currentAsset;
    currentAsset = asset;
    currentState->setLastTouchedAsset(this->currentAsset);
}

