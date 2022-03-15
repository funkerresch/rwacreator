#include "rwabackend.h"

RwaBackend *RwaBackend::instance = nullptr;

RwaBackend *RwaBackend::getInstance()
{
    if(RwaBackend::instance == nullptr)
        RwaBackend::instance = new RwaBackend();

    return RwaBackend::instance;
}

int getNumberFromQString(const QString &xString)
{
      QRegExp xRegExp("(-?\\d+(?:[\\.,]\\d+(?:e\\d+)?)?)");
      xRegExp.indexIn(xString);
      QStringList xList = xRegExp.capturedTexts();
      if (true == xList.empty())
      {
        return 0;
      }
      return xList.begin()->toInt();
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
    StartHttpServer(8088);

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
    appendScene();
}

RwaBackend::~RwaBackend()
{
    QString killHttp = QString("kill %1").arg(httpProcessId);
    system(killHttp.toStdString().c_str());
}

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

RwaScene * RwaBackend::getLastTouchedScene()
{
    return lastTouchedScene;
}

qint32 RwaBackend::getNumberOfScenes()
{
    return scenes.count();
}

void RwaBackend::newSceneFromSelectedStates()
{
    std::string sceneName = "Scene " + std::to_string(scenes.count());
    RwaScene *newScene = new RwaScene(sceneName, lastTouchedScene->getCoordinates(), lastTouchedScene->getZoom());

    foreach (QString name, currentlySelectedStates)
    {
        RwaState *state = lastTouchedScene->getState(name.toStdString());
        RwaState *newState = new RwaState(state->objectName());
        state->copyAttributes(newState);
        state->setScene(newScene);
        newScene->getStates().push_back(newState);
    }

    newScene->findStateSurroundingArea();
    newScene->setAreaType(RWAAREATYPE_RECTANGLE);
    scenes.append(newScene);
    emit sendLastTouchedScene(newScene);
}

void RwaBackend::appendScene()
{
    std::vector<double> tmp(2, 0.0);
    tmp[0] = currentMapCoordinates.x();
    tmp[1] = currentMapCoordinates.y();
    RwaScene *newScene = new RwaScene(tmp);
    newScene->setObjectName(QString("Scene %1").arg(scenes.count()).toStdString());
    scenes.append(newScene);
    emit updateScene(newScene);
    emit sendLastTouchedScene(newScene);
}

void RwaBackend::appendScene(RwaScene *scene)
{
    if(scene->objectName().empty())
        scene->setObjectName(QString("Scene %1").arg(scenes.count()).toStdString());

    scenes.append(scene);
    emit updateScene(scene);
    emit sendLastTouchedScene(scene);
}

void RwaBackend::clearScene(RwaScene *scene)
{
    scene->clear();
    emit updateScene(scene);
}

void RwaBackend::duplicateScene(RwaScene *scene)
{
    if(logOther)
        qDebug("BACKEND: duplicateScene");
}

void RwaBackend::receiveSceneName(RwaScene *scene, QString name)
{
    scene->setName(name.toStdString());
}

void RwaBackend::receiveLastTouchedScene(RwaScene *scene)
{
    if(logOther)
        qDebug();

    this->lastTouchedScene = scene;
    emit sendLastTouchedScene(scene);
    if(lastTouchedScene->currentState)
        receiveLastTouchedState(lastTouchedScene->currentState);

    currentMapCoordinates = QPointF(scene->getCoordinates()[0], scene->getCoordinates()[1]);
}

void RwaBackend::receiveLastTouchedScene(qint32 sceneNumber)
{
    if(sceneNumber < scenes.count())
    {
        this->lastTouchedScene = this->scenes.at(sceneNumber);
        emit sendLastTouchedScene(lastTouchedScene);
        if(lastTouchedScene->currentState)
            receiveLastTouchedState(lastTouchedScene->currentState);
    }

    currentMapCoordinates = QPointF(this->lastTouchedScene->getCoordinates()[0], this->lastTouchedScene->getCoordinates()[1]);
}

void RwaBackend::removeScene(RwaScene *scene)
{
    RwaScene *currentScene;
    qint32 index;
    if(scenes.count() == 1) // keep always one scene
        return;
    else
    {
        index = scenes.indexOf(scene);
        index--;
        scenes.removeOne(scene);
        currentScene = scenes.at(index);
        emit sendLastTouchedScene(currentScene);
    }
}

void RwaBackend::moveScene2CurrentMapLocation()
{
    std::vector<double> tmp(2, 0.0);
    tmp[0] = currentMapCoordinates.x();
    tmp[1] = currentMapCoordinates.y();

    if(lastTouchedScene)
        lastTouchedScene->moveScene2NewLocation(tmp);

    emit sendLastTouchedScene(lastTouchedScene);
}

void RwaBackend::clearScenes()
{
    scenes.clear();
    //appendScene();
    //emit sendClearAll();
}

void RwaBackend::receiveReadUndoFile(QString name)
{
    emit readUndoFile(name);
}

void RwaBackend::receiveWriteUndo(QString undoAction)
{
    emit sendWriteUndo(undoAction);
}

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

void RwaBackend::receiveRedrawAssets()
{
    emit sendRedrawAssets();
}

void RwaBackend::receiveSelectedAssets(QStringList assets)
{
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

void RwaBackend::copySelectedStates2Clipboard()
{
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

        lastTouchedScene->getStates().push_back(newState);
        newState->setScene(lastTouchedScene);
    }

    foreach (RwaState *clipboardState, clipboardStates->getStates())
    {
        clipboardStates->removeState(clipboardState);
    }

    emit sendLastTouchedScene(lastTouchedScene);
}

void RwaBackend::generateUuidsForClipboardState(RwaState *state)
{
    foreach(RwaAsset1 *asset, state->getAssets())
    {
        asset->getUniqueId() = std::string(QUuid::createUuid().toString().toLatin1());
    }
}

void RwaBackend::startStopSimulator(bool startStop)
{
    if(startStop)
        simulator->startRwaSimulation();
    else
        simulator->stopRwaSimulation();

    emit updateScene(lastTouchedScene);
}

void RwaBackend::receiveTrashAssets(bool onOff)
{
    trashAsset = onOff;
}

void RwaBackend::setMainVolume(int volume)
{
    simulator->setMainVolume(float(volume)/100.0f);
}

bool RwaBackend::isSimulationRunning()
{
    return simulator->isSimulationRunning();
}
void RwaBackend::setLastTouchedScene(RwaScene *scene)
{
    lastTouchedScene = scene;
}

void RwaBackend::receiveStateName(RwaState *state, QString name)
{
    state->setObjectName(name.toStdString());
}

void RwaBackend::clearForHistory()
{
    scenes.clear();
}

void RwaBackend::clear()
{
    emptyTmpDirectories();
    scenes.clear();
}

void RwaBackend::clearData()
{
    emptyTmpDirectories();
    scenes.clear();
    appendScene();
    emit updateScene(scenes.first());
}

void RwaBackend::emptyTmpDirectories()
{
    RwaUtilities::emtpyDirectory(completeUndoPath);
    RwaUtilities::emtpyDirectory(completeTmpPath);
}

void RwaBackend::receiveMapCoordinates(QPointF mapCoordinates)
{
    currentMapCoordinates = mapCoordinates;
}

void RwaBackend::receiveCurrentStateEdited()
{
   emit sendLastTouchedState(this->lastTouchedState);
}

void RwaBackend::receiveUpdatedAssets()
{
    emit updateAssets();
}

void RwaBackend::calibrateHeadtracker()
{
    headtracker->calibrateHeadtracker();
}

void RwaBackend::receiveEntityPosition(QPointF position)
{
    emit sendEntityPosition(position);
}

void RwaBackend::receiveStatePosition(QPointF position)
{
    emit sendStatePosition(position);
}

void RwaBackend::receiveLastTouchedAsset(RwaAsset1 *item)
{
   this->lastTouchedAssetItem = item;
   emit sendLastTouchedAsset(item);
}

void RwaBackend::receiveLastTouchedState(RwaState *state)
{
    RwaAsset1 *lastTouched = nullptr;
    this->lastTouchedState = state;
    emit sendLastTouchedState(state);

    if(state->getLastTouchedAsset())
        lastTouched = state->getLastTouchedAsset();

    else
    {
        if(!state->getAssets().empty())
            lastTouched = state->getAssets().front();
    }

    if(lastTouched)
        emit sendLastTouchedAsset(lastTouched);
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

void RwaBackend::receiveCurrentStateString(RwaState *state, QString stateName)
{
    QStringList lines = stateName.split( "\n", QString::SkipEmptyParts );
    foreach (const QString &text, lines)
    {
        if(!text.indexOf("@name: "))
        {
            QStringList tmp = text.split( ": ", QString::SkipEmptyParts );
            tmp.removeFirst();
            foreach (const QString &attr, tmp)
            {
                QStringList values = attr.split( ";", QString::SkipEmptyParts );
                {
                    foreach (const QString &value, values)
                        state->setObjectName(value.toStdString());
                }
            }
        }

        else if(!text.indexOf("@name:"))
        {
            QStringList attr = text.split( ":", QString::SkipEmptyParts );
            attr.removeFirst();
            foreach (const QString &value, attr)
            {
                state->setObjectName(value.toStdString());

            }
        }
    }
    emit updateScene(state->getScene());
}


