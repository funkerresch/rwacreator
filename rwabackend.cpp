#include "rwabackend.h"

RwaBackend *RwaBackend::instance = nullptr;

RwaBackend *RwaBackend::getInstance()
{
    if(RwaBackend::instance == nullptr)
        RwaBackend::instance = new RwaBackend();

    return RwaBackend::instance;
}

void RwaBackend::StartHttpServer(qint32 port)
{
    char buffer[20];
    QString httpServerStart = QString("python3 -m http.server %1 --directory /Users/harveykeitel/RWACreator/Games & echo $!").arg(port);
    FILE* pipe = popen(httpServerStart.toStdString().c_str(), "r");
    if (!pipe)
       qDebug() << "Could not create http server";

    if (fgets(buffer, 128, pipe) != nullptr)
        httpProcessId = getNumberFromQString(QString(buffer));

    pclose(pipe);
}

RwaBackend::RwaBackend(QWidget *parent) :
    QTextEdit(parent)
{
    CFURLRef url = (CFURLRef)CFAutorelease((CFURLRef)CFBundleCopyBundleURL(CFBundleGetMainBundle()));
    QString path = QUrl::fromCFURL(url).path();
    httpProcessId = -1;
    //StartHttpServer(8088);

    completeBundlePath = path + "Contents/MacOS/";
    completeProjectPath = QString();
    completeFilePath = QString();
    completeUndoPath = QString();
    completeAssetPath = QString();
    completeTmpPath = QString();
    projectName = QString();
    currentMapCoordinates = QPointF(QPointF(8.27,50));
    simulator = new RwaSimulator(this, this);
    headtracker = RwaHeadtrackerConnect::getInstance();
    clipboardStates = new RwaScene(std::string("ClipboardScene"), std::vector<double>(2, 0.0), 0);

    assetStringList = QStringList();
    appendScene();
}

RwaBackend::~RwaBackend()
{
   // QString killHttp = QString("kill %1").arg(httpProcessId);
   // system(killHttp.toStdString().c_str());
}

/**
 * ****************************** Undo read and write******************************
 *
 * All editors are connected to receiveWriteUndo slot. Backend emits the undoAction
 * to the creator class which is responsibe for file i/o.
 *
 */

void RwaBackend::receiveReadUndoFile(QString name)
{
    emit readUndoFile(name);
    emit sendLastTouchedScene(lastTouchedScene);
}

void RwaBackend::receiveWriteUndo(QString undoAction)
{
    emit sendWriteUndo(undoAction);
}

/**
 * ****************************** Last touched/selected functionality ******************************
 *
 * All  editors are connected to the last touched asset/state/scene slot receiver
 * and to the sendLast*.* signals. Whenever the user select a scene/state/asset from an
 * editor, all other views are signaled and render the lastTouched asset/state/scene too.
 *
 * All editors also emit signals of currently selected assets,states and scenes for
 * delete, copy, paste and multi-item editing functionality
 *
 */

void RwaBackend::receiveLastTouchedAsset(RwaAsset1 *asset)
{
    if(!asset)
        return;

   lastTouchedAssetItem = asset;

   if(lastTouchedState)
        lastTouchedState->lastTouchedAsset = asset;

   emit sendLastTouchedAsset(asset);
}

void RwaBackend::receiveLastTouchedState(RwaState *state)
{
    if(!state)
        return;

    lastTouchedState = state;

    if(lastTouchedScene)
        lastTouchedScene->lastTouchedState = state;

    emit sendLastTouchedState(state);
}

void RwaBackend::receiveLastTouchedScene(RwaScene *scene)
{
    if(!scene)
        return;

    lastTouchedScene = scene;
    currentMapCoordinates = QPointF(scene->getCoordinates()[0], scene->getCoordinates()[1]);
    emit sendLastTouchedScene(scene);
}

void RwaBackend::receiveLastTouchedScene(qint32 sceneNumber)
{
    if(sceneNumber < scenes.count())
    {
        lastTouchedScene = this->scenes.at(sceneNumber);
        currentMapCoordinates = QPointF(this->lastTouchedScene->getCoordinates()[0], this->lastTouchedScene->getCoordinates()[1]);
        emit sendLastTouchedScene(lastTouchedScene);
    }
}

void RwaBackend::receiveSelectedAssets(QStringList assets)
{
    if(logOther)
        qDebug() << assets;

    currentlySelectedAssets = assets;
    emit sendSelectedAssets(currentlySelectedAssets); // not connected to anything yet
}

void RwaBackend::receiveSelectedStates(QStringList states) // not connected to anything yet
{
    if(logOther)
        qDebug() << states;

    currentlySelectedStates = states;
    emit sendSelectedStates(currentlySelectedStates);
}

void RwaBackend::receiveSelectedScenes(QStringList scenes) // not connected to anything yet
{
    if(logOther)
        qDebug() << scenes;

//    currentlySelectedScenes = scenes;
//    emit sendSelectedScenes(currentlySelectedScenes);
}

/** ****************************** Scene-related functionality ****************************** */

/** *************************************** Get methods ************************************* */

QList<RwaScene *>& RwaBackend::getScenes()
{
    return scenes;
}

RwaScene *RwaBackend::getFirstScene()
{
    return scenes.first();
}

RwaScene *RwaBackend::getSceneAt(qint32 sceneNumber)
{
    return scenes.at(sceneNumber);
}

RwaScene * RwaBackend::getLastTouchedScene()
{
    return lastTouchedScene;
}

qint32 RwaBackend::getNumberOfScenes()
{
    return scenes.count();
}

RwaScene *RwaBackend::getScene(QString sceneName)
{
    RwaScene *scene = nullptr;
    foreach(scene, scenes)
    {
        if(scene->objectName() == sceneName.toStdString())
            return scene;
    }
    return scene;
}

/** ****************************** New/Append Scene functionality ****************************** */

void RwaBackend::appendScene(RwaScene *scene)
{
    if(scene->objectName().empty())
        scene->setObjectName(QString("Scene %1").arg(scenes.count()).toStdString());

    scenes.append(scene);
    emit sendAppendScene();
}

void RwaBackend::appendScene()
{
    std::vector<double> tmp(2, 0.0);
    tmp[0] = currentMapCoordinates.x();
    tmp[1] = currentMapCoordinates.y();
    RwaScene *newScene = new RwaScene(tmp);
    newScene->setObjectName(QString("Scene %1").arg(scenes.count()).toStdString());
    scenes.append(newScene);
    lastTouchedScene = newScene;
    emit sendAppendScene();
}

void RwaBackend::newSceneFromSelectedStates()
{
    bool hasFallback = false;
    bool hasBackground = false;
    std::string sceneName = "Scene " + std::to_string(scenes.count());
    RwaScene *newScene = new RwaScene(sceneName, lastTouchedScene->getCoordinates(), lastTouchedScene->getZoom());

    foreach (QString name, currentlySelectedStates)
    {
        RwaState *state = lastTouchedScene->getState(name.toStdString());
        RwaState *newState = new RwaState(state->objectName());
        state->copyAttributes(newState);
        newState->setScene(newScene);
        newScene->getStates().push_back(newState);
        if(state->objectName() == "FALLBACK")
            hasFallback = true;
        if(state->objectName() == "BACKGROUND")
            hasBackground = true;
    }

    if(!hasFallback)
        newScene->InsertDefaultFallbackState();
    if(!hasBackground)
        newScene->InsertDefaultBackgroundState();

    newScene->findStateSurroundingArea();
    scenes.append(newScene);
    lastTouchedScene = newScene;
    emit sendAppendScene();
    emit sendLastTouchedScene(lastTouchedScene);

}

void RwaBackend::duplicateScene(RwaScene *scene)
{
    if(logOther)
        qDebug("BACKEND: duplicateScene");
}

/** **************************** Clear/Remove/Reset Scene(s) functionality **************************** */

void RwaBackend::removeScene(RwaScene *scene)
{
    qint32 index;

    if(scenes.count() == 1) // keep always one scene
        return;
    else
    {
        index = scenes.indexOf(scene);
        index--;
        if(index < 0)
            index = 0;

        scenes.removeOne(scene);
        lastTouchedScene = scenes.at(index);
        emit sendLastTouchedScene(lastTouchedScene);
    }
    scene->clear();
}

void RwaBackend::removeScene(QString sceneName)
{
    RwaScene *scene = getScene(sceneName);
    if(!scene)
        return;

    removeScene(scene);
}

void RwaBackend::clearScene(RwaScene *scene)
{
    scene->clear();
    lastTouchedScene = scene;
    emit sendLastTouchedScene(lastTouchedScene);
}

void RwaBackend::clearScenes()
{
    foreach(RwaScene *scene, scenes)
        scene->clear();

    scenes.clear();
}

void RwaBackend::reset()
{
    clearScenes();
    appendScene();
    emit updateScene(scenes.first());
}

/** ************************************* Location functionality ************************************* */

void RwaBackend::moveScene2CurrentMapLocation()
{
    std::vector<double> tmp(2, 0.0);
    tmp[0] = currentMapCoordinates.x();
    tmp[1] = currentMapCoordinates.y();

    if(lastTouchedScene)
        lastTouchedScene->moveScene2NewLocation(tmp);

    qDebug() << "Move Scene";

    emit sendEntityPosition(tmp);
    emit sendHeroPositionEdited();
    emit sendLastTouchedScene(lastTouchedScene);
}

void RwaBackend::receiveMapCoordinates(QPointF mapCoordinates)
{
    currentMapCoordinates = mapCoordinates;
}

/** *******************************  State copy and paste ******************************* */

void RwaBackend::copySelectedStates2Clipboard()
{
    foreach (RwaState *clipboardState, clipboardStates->getStates())
        clipboardStates->removeState(clipboardState);

    foreach (QString name, currentlySelectedStates)
    {
        RwaState *state = lastTouchedScene->getState(name.toStdString());
        RwaState *newState = new RwaState(state->objectName());
        state->copyAttributes(newState);
        clipboardStates->getStates().push_back(newState);
    }
}

void RwaBackend::pasteStatesFromClipboard()
{
    foreach (RwaState *clipboardState, clipboardStates->getStates())
    {
        RwaState *newState = new RwaState(clipboardState->objectName());
        clipboardState->copyAttributes(newState);
        generateUuidsForClipboardState(newState);
        adjust2UniqueStateName(lastTouchedScene, newState);
        lastTouchedScene->getStates().push_back(newState);
        newState->setScene(lastTouchedScene);
    }

    emit sendLastTouchedScene(lastTouchedScene);
}

/** *************************  RWA Graphic View receiver to emitter functions ************************* */

void RwaBackend::receiveEntityPosition(QPointF position)
{
    //emit sendEntityPosition(position);
}

void RwaBackend::receiveStatePosition(QPointF position)
{
    emit sendStatePosition(position);
}

void RwaBackend::receiveMoveCurrentState1(double dx, double dy)
{
    emit sendMoveCurrentState1(dx, dy);
}

void RwaBackend::receiveMoveCurrentAsset1(double dx, double dy)
{
    emit sendMoveCurrentAsset1(dx, dy);
}

void RwaBackend::receiveMoveCurrentAssetChannel(double dx, double dy, int channel)
{
    emit sendMoveCurrentAssetChannel(dx, dy, channel);
}

void RwaBackend::receiveMoveCurrentAssetReflection(double dx, double dy, int channel)
{
    emit sendMoveCurrentAssetReflection(dx, dy, channel);
}

void RwaBackend::receiveCurrentStateRadiusEdited()
{
    emit sendCurrentStateRadiusEdited();
}

void RwaBackend::receiveCurrentSceneRadiusEdited()
{
    emit sendCurrentSceneRadiusEdited();
}

/** ******************************** Editor Global Rendering/Functionality ********************************* */

void RwaBackend::receiveTrashAssets(bool onOff)
{
    trashAsset = onOff;
}

void RwaBackend::receiveShowStateRadii(bool onOff)
{
    showStateRadii = onOff;
}

void RwaBackend::receiveShowAssets(bool onOff)
{
    showAssets = onOff;
}

/** ****************************** Logging related functions ***************************** */

void RwaBackend::receiveLogLonAndLat(int onOff)
{
    logCoordinates = onOff;
}

void RwaBackend::receiveLogLibPd(int onOff)
{
    logPd = onOff;
}

void RwaBackend::receiveLogSimulator(int onOff)
{
    logSim = onOff;
}

void RwaBackend::receiveLogOther(int onOff)
{
    logOther = onOff;
}

/** ****************************** Simulator and headtracker related functions ***************************** */

void RwaBackend::startStopSimulator(bool startStop)
{
    if(startStop)
        simulator->startRwaSimulation();
    else
        simulator->stopRwaSimulation();

    emit updateScene(lastTouchedScene);
}

void RwaBackend::setMainVolume(int volume)
{
    simulator->setMainVolume(float(volume)/100.0f);
}

bool RwaBackend::isSimulationRunning()
{
    return simulator->isSimulationRunning();
}

void RwaBackend::calibrateHeadtracker()
{
    headtracker->calibrateHeadtracker();
}

/** *********************** Static utility functionality (string generation, UUIDs, ..) ************************* */

int RwaBackend::getStateNameCounter(std::list<RwaState *> &states)
{
    int maxStateCount = 2;
    foreach (RwaState *state, states)
    {
        QRegularExpression rx("State \\d+");
        QRegularExpressionMatch match = rx.match(QString::fromStdString(state->objectName()));

        if(match.hasMatch())
        {
            QRegularExpression re("\\d+");
            match = re.match(QString::fromStdString(state->objectName()));
            if(match.captured(0).toInt() > maxStateCount)
                maxStateCount = match.captured(0).toInt();
        }
    }
    maxStateCount++;
    return maxStateCount;
}

bool RwaBackend::adjust2UniqueStateNameRecursively(RwaScene *targetScene, RwaState *newState)
{
    bool found = false;

    foreach (RwaState *state, targetScene->getStates())
    {
        if(!state->objectName().compare(newState->objectName()))
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        std::string stateName(newState->objectName() + " copy");
        newState->setObjectName(stateName);
        return true;

    }
    return false;
}

bool RwaBackend::fileUsedByAnotherAsset(RwaAsset1 *asset2Delete)
{
    foreach(RwaScene *scene, scenes)
    {
        foreach(RwaState *state, scene->states)
        {
            foreach(RwaAsset1 *asset, state->assets)
            {
                if(asset !=asset2Delete)
                {
                    if(asset->getFileName() == asset2Delete->getFileName())
                       return true;
                }

            }
        }
    }
    return false;
}


void RwaBackend::adjust2UniqueStateName(RwaScene *targetScene, RwaState *newState)
{
    while(adjust2UniqueStateNameRecursively(targetScene, newState));
}

void RwaBackend::generateUuidsForClipboardState(RwaState *state)
{
    foreach(RwaAsset1 *asset, state->getAssets())
        asset->getUniqueId() = std::string(QUuid::createUuid().toString().toLatin1());
}

int RwaBackend::getNumberFromQString(const QString &xString)
{
    QRegExp xRegExp("(-?\\d+(?:[\\.,]\\d+(?:e\\d+)?)?)");
    xRegExp.indexIn(xString);
    QStringList xList = xRegExp.capturedTexts();
    if (true == xList.empty())
        return 0;

    return xList.begin()->toInt();
}
