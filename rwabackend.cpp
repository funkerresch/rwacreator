#include "rwabackend.h"
#include <QStandardPaths>
#include <QThread>
#include <QLocale>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include "rwautilities.h"

RwaBackend *RwaBackend::instance = nullptr;

/**
 * ****************************** Game Server ******************************
 *
 * The Game Server is a static httplib server for downloading games from the client.
 * It is started manually from the creator application.
 *
 */

RwaGamesServer::RwaGamesServer(httplib::Server *svr, int port, std::string mountPoint)
{
    this->svr = svr;
    this->port = port;
    this->mountPoint = mountPoint;
}

void RwaGamesServer::process()
{
    using namespace httplib;
    auto ret = svr->set_mount_point("/", mountPoint);

    if (!ret) {
        qWarning() << "Sharing Server directory does not exist!";
    }

    // Report what connected players ask for. Both handlers run on an httplib worker
    // thread and therefore only emit - formatting and the log view belong to the main
    // thread, which the queued connection in StartHttpServer1 takes care of.

    // Arrival: the download attempt itself. Logged separately because a large game
    // takes a while to transfer and would otherwise show up only once it is through.
    svr->set_pre_routing_handler([this](const Request &req, Response &) {
        emit clientRequest(QString::fromStdString(req.remote_addr),
                           QString::fromStdString(req.method),
                           QString::fromStdString(req.path),
                           false, 0, -1);
        return Server::HandlerResponse::Unhandled;
    });

    // Outcome. httplib also calls the logger when writing the response failed, in
    // which case the status is still the one it started sending - a client that walks
    // out of wifi mid-download looks like a successful transfer here.
    svr->set_logger([this](const Request &req, const Response &res) {
        const std::string length = res.get_header_value("Content-Length");
        emit clientRequest(QString::fromStdString(req.remote_addr),
                           QString::fromStdString(req.method),
                           QString::fromStdString(req.path),
                           true, res.status,
                           length.empty() ? -1 : QString::fromStdString(length).toLongLong());
    });

    svr->listen("0.0.0.0", port);
}

RwaBackend *RwaBackend::getInstance()
{
    if(RwaBackend::instance == nullptr)
        RwaBackend::instance = new RwaBackend();

    return RwaBackend::instance;
}

void RwaBackend::StartHttpServer1(qint32 port)
{
    serverThread = new QThread();
    serverThread->setObjectName("RWA Listener");
    RwaGamesServer* worker = new RwaGamesServer(&svr, port, completeSharingServerPath.toStdString());
    worker->moveToThread(serverThread);
    connect( serverThread, &QThread::started, worker, &RwaGamesServer::process);
    connect( serverThread, &QThread::finished, worker, &QObject::deleteLater);
    connect( worker, &RwaGamesServer::clientRequest, this, &RwaBackend::receiveClientRequest);
    serverThread->start();

    qInfo() << "Sharing server listening on port" << port << "serving" << completeSharingServerPath;
}

/** Logs what the connected players do with the sharing server. Runs in the main thread. */

void RwaBackend::receiveClientRequest(QString clientAddress, QString method, QString path,
                                      bool finished, int status, qint64 bytes)
{
    // httplib percent-decodes req.path for us, so path is already the plain game
    // name - decoding it a second time would mangle any name containing a '%'.

    if(!finished)
    {
        qInfo().noquote() << QString("Player %1 requests %2 %3")
                                 .arg(clientAddress, method, path);
        return;
    }

    const QString size = bytes >= 0 ? QLocale().formattedDataSize(bytes) : QString("unknown size");

    // One arg() call per message, never chained: chaining rescans what was already
    // substituted, so a game called "50%20 off.zip" would eat the placeholder that
    // the status goes into.
    if(status >= 400)
        qWarning().noquote() << QString("Player %1 could not get %2 (%3)")
                                    .arg(clientAddress, path, QString::number(status));
    else
        qInfo().noquote() << QString("Player %1 got %2 (%3, %4)")
                                 .arg(clientAddress, path, QString::number(status), size);
}

qint32 RwaBackend::getSampleRate() const
{
    return sampleRate;
}

void RwaBackend::setSampleRate(qint32 newSampleRate)
{
    if(newSampleRate == 44100 || newSampleRate == 48000)
        sampleRate = newSampleRate;
    else
        sampleRate = 48000;

    RwaRuntime::pdSampleRate = sampleRate;
}

void RwaBackend::StopHttpServer1()
{
    qDebug() << "Try Stopping";
    svr.stop(); // according to documentation this should be thread-safe and the only possibility to stop listening
    serverThread->quit();
    serverThread->wait();
}

/** The Python server was used for debugging purposes, might still be useful for something. */

void RwaBackend::StartHttpServer(qint32 port)
{
    char buffer[20];
    QString httpServerStart = QString("python3 -m http.server %1 --directory /Users/harveykeitel/RWACreator/Games & echo $!").arg(port);
    FILE* pipe = popen(httpServerStart.toStdString().c_str(), "r");
    if (!pipe)
    {
       qWarning() << "Could not start http server";
       return;
    }

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
    completeBundlePath = path + "Contents/Resources/";
    completeProjectPath = QString();
    completeFilePath = QString();
    completeUndoPath = QString();
    completeAssetPath = QString();
    completeTmpPath = QString();
    completeTransferToPlayerExportPath = QString("%1%2").arg(QDir::homePath()).arg("/Desktop");
    completeSharingServerPath = QString("%1%2").arg(QDir::homePath()).arg("/Library/Application Support/RWACreator/Games");
    completeSharingServerPathWithEscape = QString("%1%2").arg(QDir::homePath()).arg("\"/Library/Application Support/RWACreator/Games\"");
    applicationSupportPath = QString("%1%2").arg(QDir::homePath()).arg("/Library/Application Support/RWACreator");
    applicationSupportPathWithEscape = QString("%1%2").arg(QDir::homePath()).arg("\"/Library/Application Support/RWACreator\"");
    projectName = QString();
    currentMapCoordinates = QPointF(QPointF(8.27,50));
    simulator = new RwaSimulator(this, this);
    headtracker = RwaHeadtrackerConnect::getInstance();
    clipboardStates = new RwaScene(std::string("ClipboardScene"), std::vector<double>(2, 0.0), 0);
    assetStringList = QStringList();
    setSampleRate(48000);
    appendScene();
}

RwaBackend::~RwaBackend()
{
   // QString killHttp = QString("kill %1").arg(httpProcessId);
   // system(killHttp.toStdString().c_str());
}
void RwaBackend::updateLastTouchedSceneStateAndAsset()
{
    emit updateGame();

    if(scenes.empty()) // e.g. an undo file failed to parse
        return;

    if(!lastTouchedScene)
        lastTouchedScene = scenes.front();

    emit sendLastTouchedScene(lastTouchedScene);

    if(!lastTouchedScene->lastTouchedState && !lastTouchedScene->states.empty())
        lastTouchedScene->lastTouchedState = lastTouchedScene->states.front();

    lastTouchedState = lastTouchedScene->lastTouchedState;

    emit sendLastTouchedState(lastTouchedState);

    if(lastTouchedState && !lastTouchedState->assets.empty())
    {
        if(!lastTouchedState->lastTouchedAsset)
            lastTouchedState->lastTouchedAsset = lastTouchedState->assets.front();

        lastTouchedAssetItem = lastTouchedState->lastTouchedAsset;
        emit sendLastTouchedAsset(lastTouchedAssetItem);
    }

    emit sendMoveHero2CurrentScene();
}

void RwaBackend::receiveReadNewGame()
{
    emit newGameLoaded();
    updateLastTouchedSceneStateAndAsset();
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
    bool wasRunning = simulator->isSimulationRunning();
    emit readUndoFile(name);
    updateLastTouchedSceneStateAndAsset();
    emit undoGameLoaded();
    if(wasRunning)
        simulator->startRwaSimulation();
}

/** ******* Receives Undo Action as String from Editors and send signal to RwaCreator.c ******** **/

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

    // default to the first asset before broadcasting, so every view agrees on
    // the touched asset; null for states without assets (the asset views clear)
    if(!state->lastTouchedAsset && !state->assets.empty())
        state->lastTouchedAsset = state->assets.front();

    lastTouchedAssetItem = state->lastTouchedAsset;

    emit sendLastTouchedState(state);
}

void RwaBackend::receiveCurrentSceneWithouRepositioning(RwaScene *scene)
{
    if(!scene)
        return;

    lastTouchedScene = scene;
    emit sendCurrentSceneWithoutRepositioning(scene);
}

void RwaBackend::receiveCurrentStateWithouRepositioning(RwaState *state)
{
    qDebug();
    if(!state)
        return;

    lastTouchedState = state;
    emit sendCurrentStateWithoutRepositioning(state);
}

void RwaBackend::receiveLastTouchedScene(RwaScene *scene)
{
    if(!scene)
        return;

    lastTouchedScene = scene;
    //currentMapCoordinates = QPointF(scene->getCoordinates()[0], scene->getCoordinates()[1]);
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

void RwaBackend::validateRequiredStates()
{
    foreach(RwaScene *scene, scenes)
    {
        foreach(RwaState *state, scene->getStates())
        {
            foreach(const std::string &requiredName, state->getRequiredStates())
            {
                QStringList foundInScenes;
                foreach(RwaScene *candidateScene, scenes)
                {
                    if(candidateScene->getState(requiredName))
                        foundInScenes.append(QString::fromStdString(candidateScene->objectName()));
                }

                if(foundInScenes.isEmpty())
                    qWarning() << "State" << QString::fromStdString(state->objectName())
                               << "in scene" << QString::fromStdString(scene->objectName())
                               << "requires state" << QString::fromStdString(requiredName)
                               << "which does not exist in any scene.";
                else if(foundInScenes.count() > 1)
                    qWarning() << "State" << QString::fromStdString(state->objectName())
                               << "in scene" << QString::fromStdString(scene->objectName())
                               << "requires state" << QString::fromStdString(requiredName)
                               << "which exists in multiple scenes (" << foundInScenes.join(", ")
                               << "); visiting any of them will satisfy the requirement.";
            }
        }
    }
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
    emit updateGame();
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
    emit updateGame();
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
        if(!state)
            continue;
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
    emit updateGame();
    emit sendLastTouchedScene(lastTouchedScene);

}

void RwaBackend::duplicateScene()
{
    RwaScene *newScene = new RwaScene(lastTouchedScene->objectName(), lastTouchedScene->getCoordinates(), lastTouchedScene->getZoom());
    adjust2UniqueSceneName(newScene);

    foreach (RwaState *state, lastTouchedScene->states)
    {
        RwaState *newState = new RwaState(state->objectName());
        state->copyAttributes(newState);
        newState->setScene(newScene);
        newScene->getStates().push_back(newState);
    }

    lastTouchedScene->copyAttributes(newScene);
    scenes.append(newScene);
    lastTouchedScene = newScene;
    moveScene2CurrentMapLocation();
    emit updateGame();
    emit sendLastTouchedScene(lastTouchedScene);
}

/** **************************** Clear/Remove/Reset Scene(s) functionality **************************** */

void RwaBackend::removeScene(RwaScene *scene)
{
    if(isSimulationRunning())
    {
        // the entity's currentScene would keep a pointer to the deleted scene
        qWarning() << "Cannot delete a scene while the simulation is running.";
        return;
    }

    if(scenes.count() == 1) // keep always one scene
    {
        qWarning() << "The last remaining scene can not be deleted.";
        return;
    }

    qint32 index = scenes.indexOf(scene);
    if(index < 0)
        return;

    scenes.removeAt(index);
    index--;
    if(index < 0)
        index = 0;

    lastTouchedScene = scenes.at(index);
    lastTouchedState = nullptr;
    lastTouchedAssetItem = nullptr;
    updateLastTouchedSceneStateAndAsset();
    delete scene;

    emit sendWriteUndo("Delete Scene");
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
    lastTouchedState = nullptr;
    lastTouchedAssetItem = nullptr;
    updateLastTouchedSceneStateAndAsset();
}

void RwaBackend::clearScenes()
{
    if(simulator && simulator->isSimulationRunning())
        simulator->stopRwaSimulationNow();

    foreach(RwaScene *scene, scenes)
        scene->clear();

    scenes.clear();
    lastTouchedScene = nullptr;
    lastTouchedState = nullptr;
    lastTouchedAssetItem = nullptr;
}

void RwaBackend::reset()
{
    projectName = QString();
    completeFilePath = QString();
    completeProjectPath = QString();

    // The derived paths must not keep pointing into the previously opened
    // project: an unsaved new project would write its undo steps, tmp files and
    // dropped assets in there. Until it is saved it works in a scratch folder;
    // "Save" (exportProjectAs) copies the assets from there into the real project.
    QString scratch = unsavedProjectPath();
    completeUndoPath = scratch + "/undo";
    completeTmpPath = scratch + "/tmp";
    completeAssetPath = scratch + "/assets";
    for (const QString &folder : {completeUndoPath, completeTmpPath, completeAssetPath})
    {
        QDir dir(folder);
        if (!dir.exists() && !dir.mkpath("."))
            qWarning() << "Could not create scratch folder" << folder;
        RwaUtilities::emtpyDirectory(folder); // leftovers of an earlier unsaved project (e.g. after a crash)
    }

    clearScenes();
    appendScene();
    emit projectPathsChanged();
    updateLastTouchedSceneStateAndAsset();
}

QString RwaBackend::unsavedProjectPath() const
{
    return applicationSupportPath + "/unsaved";
}

QString RwaBackend::tileCachePath() const
{
    if (!completeProjectPath.isEmpty() && QDir(completeProjectPath).isAbsolute())
        return completeProjectPath + "/tilecache";

    return unsavedProjectPath() + "/tilecache";
}

/** ************************************* Location functionality ************************************* */

void RwaBackend::moveScene2CurrentMapLocation()
{
    std::vector<double> tmp(2, 0.0);
    tmp[0] = currentMapCoordinates.x();
    tmp[1] = currentMapCoordinates.y();
    lastTouchedScene->currentViewCoordinates = tmp;

    if(lastTouchedScene)
        lastTouchedScene->moveScene2NewLocation(tmp);

    emit sendLastTouchedScene(lastTouchedScene);
    emit sendMoveHero2CurrentScene();
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
        if(!state)
            continue;
        RwaState *newState = new RwaState(state->objectName());
        state->copyAttributes(newState);
        // copyAttributes carries myScene over from the source state, which would
        // dangle once that scene is deleted. The copy belongs to the clipboard
        // until pasteStatesFromClipboard() gives it its new scene.
        newState->setScene(clipboardStates);
        clipboardStates->getStates().push_back(newState);
    }
}

static QByteArray fileChecksum(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        return QByteArray();

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if(!hash.addData(&file))
        return QByteArray();

    return hash.result();
}

static bool filesAreIdentical(const QString &pathA, const QString &pathB)
{
    if(QFileInfo(pathA).size() != QFileInfo(pathB).size())
        return false;

    QByteArray checksumA = fileChecksum(pathA);
    return !checksumA.isEmpty() && checksumA == fileChecksum(pathB);
}

void RwaBackend::copyAssetFile2Project(RwaAsset1 *asset)
{
    QString fileName = QString::fromStdString(asset->getFileName());
    QString sourcePath = QString::fromStdString(asset->getFullPath());
    QString targetPath = QString("%1/%2").arg(completeAssetPath).arg(fileName);

    if(QFileInfo(sourcePath).absoluteFilePath() == QFileInfo(targetPath).absoluteFilePath())
        return;

    if(!QFile::exists(sourcePath))
    {
        if(!QFile::exists(targetPath))
            qWarning() << "Pasted asset" << fileName << "is missing at its source location" << sourcePath
                       << "- copy the file into" << completeAssetPath << "manually.";
        asset->setFullPath(targetPath.toStdString());
        return;
    }

    // A same-named file with different content must not be overwritten; count up
    // until the name is free or points at a copy from an earlier paste.
    QFileInfo info(fileName);
    int counter = 2;

    while(QFile::exists(targetPath) && !filesAreIdentical(sourcePath, targetPath))
    {
        fileName = QString("%1-%2.%3").arg(info.completeBaseName()).arg(counter++).arg(info.suffix());
        targetPath = QString("%1/%2").arg(completeAssetPath).arg(fileName);
    }

    if(!QFile::exists(targetPath) && !QFile::copy(sourcePath, targetPath))
        qWarning() << "Could not copy pasted asset from" << sourcePath << "to" << targetPath;

    asset->setFileName(fileName.toStdString());
    asset->setObjectName(fileName.toStdString());
    asset->setFullPath(targetPath.toStdString());
}

void RwaBackend::pasteStatesFromClipboard()
{
    if(!lastTouchedScene)
        return;

    foreach (RwaState *clipboardState, clipboardStates->getStates())
    {
        RwaState *newState = new RwaState(clipboardState->objectName());
        clipboardState->copyAttributes(newState);
        generateUuidsForClipboardState(newState);
        adjust2UniqueStateName(lastTouchedScene, newState);

        foreach(RwaAsset1 *asset, newState->getAssets())
            copyAssetFile2Project(asset);

        lastTouchedScene->getStates().push_back(newState);
        newState->setScene(lastTouchedScene);
    }

    emit sendLastTouchedScene(lastTouchedScene);
}

/** *************************  RWA Graphic View receiver to emitter functions ************************* */

void RwaBackend::receiveMoveHero2CurrentState()
{
    qDebug();
    emit sendMoveHero2CurrentState();
}

void RwaBackend::receiveMoveHero2CurrentScene()
{
    emit sendMoveHero2CurrentScene();
}

void RwaBackend::receiveEntityPosition(QPointF position)
{
    vector<double> p = {position.x(), position.y()};
    emit sendEntityPosition(p);
}

void RwaBackend::receiveStatePosition(QPointF position)
{
    emit sendStatePosition(position);
}

void RwaBackend::receiveMoveCurrentState1(double dx, double dy)
{
    emit sendMovePixmapsOfCurrentState1(dx, dy);
}

void RwaBackend::receiveMoveCurrentAsset1(double dx, double dy)
{
    emit sendMovePixmapsOfCurrentAsset1(dx, dy);
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

void RwaBackend::receiveMoveCurrentScene()
{
    emit sendMoveCurrentScene();
}

/** ******************************** Editor Global Rendering/Functionality ********************************* */

void RwaBackend::receiveTrashAssets(bool onOff)
{
    trashAsset = onOff;
}

void RwaBackend::receiveActivateClientSync(bool onOff)
{
    if(onOff)
        StartHttpServer1(8088);
    else
        StopHttpServer1();
}

void RwaBackend::receiveShowStateRadii(bool onOff)
{
    showStateRadii = onOff;
}

void RwaBackend::receiveShowAssets(bool onOff)
{
    showAssets = onOff;
}

void RwaBackend::receiveHeroFollowsSceneAndState(bool onOff)
{
    heroFollowsSceneAndState = onOff;
}

/** ****************************** Logging related functions ***************************** */

void RwaBackend::receiveLogLonAndLat(int onOff)
{
    logCoordinates = onOff;
}

void RwaBackend::receiveLogLibPd(int onOff)
{
    logPd = onOff;
    RwaRuntime::logPd = onOff;
}

void RwaBackend::receiveLogSimulator(int onOff)
{
    logSim = onOff;
    RwaRuntime::logSim = onOff;
}

void RwaBackend::receiveLogOther(int onOff)
{
    logOther = onOff;
}

/** ****************************** Simulator and headtracker related functions ***************************** */

void RwaBackend::startStopSimulator(bool startStop)
{
    simulator->runtime->lastP = 0;
    if(startStop)
        simulator->startRwaSimulation();
    else
        simulator->stopRwaSimulation();

    //emit updateScene(lastTouchedScene);
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

bool RwaBackend::adjust2UniqueSceneNameRecursively(RwaScene *newScene)
{
    bool found = false;

    foreach (RwaScene *scene, scenes)
    {
        if(!scene->objectName().compare(newScene->objectName()))
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        std::string stateName(newScene->objectName() + " copy");
        newScene->setObjectName(stateName);
        return true;

    }
    return false;
}

void RwaBackend::adjust2UniqueSceneName(RwaScene *newScene)
{
    while(adjust2UniqueSceneNameRecursively(newScene));
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

/**
 * "On asset delete: keep/remove file" moves files to the project's tmp folder
 * instead of deleting them, so that restoring an undo snapshot can bring them
 * back. The folder shares the lifetime of the undo history: when tmp is emptied
 * (quit, open, new project) its contents are forwarded to the system trash as a
 * last-resort recovery path.
 */

QString RwaBackend::sessionTrashPath() const
{
    if(completeTmpPath.isEmpty())
        return QString();

    return completeTmpPath + "/trash";
}

bool RwaBackend::moveAsset2SessionTrash(const QString &fullPath)
{
    QString trashPath = sessionTrashPath();
    if(trashPath.isEmpty() || !QDir().mkpath(trashPath))
        return false;

    QString target = trashPath + "/" + RwaUtilities::getFileName(fullPath);
    QFile::remove(target); // same name trashed twice in one session: the newest deletion wins
    return QFile::rename(fullPath, target);
}

void RwaBackend::restoreAssetFilesFromSessionTrash()
{
    QString trashPath = sessionTrashPath();
    if(trashPath.isEmpty())
        return;

    foreach(RwaScene *scene, scenes)
    {
        foreach(RwaState *state, scene->states)
        {
            foreach(RwaAsset1 *asset, state->assets)
            {
                QString fullPath = QString::fromStdString(asset->getFullPath());
                if(QFile::exists(fullPath))
                    continue;

                QString trashed = trashPath + "/" + QString::fromStdString(asset->getFileName());
                if(QFile::exists(trashed) && QFile::rename(trashed, fullPath))
                    qInfo() << "Undo brought back asset" << QString::fromStdString(asset->getFileName())
                            << "- restored its file to the assets folder.";
            }
        }
    }
}

void RwaBackend::moveSessionTrash2SystemTrash()
{
    QDir trashDir(sessionTrashPath());
    if(sessionTrashPath().isEmpty() || !trashDir.exists())
        return;

    foreach(const QFileInfo &info, trashDir.entryInfoList(QDir::Files | QDir::Hidden))
        QFile::moveToTrash(info.absoluteFilePath());
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

/**
  Re-reads the audio properties of all assets from disk.

  The Creator caches duration, channel count and sample rate when a project is
  loaded and when an asset is added, and doesn't look at the files again. Editing
  an asset outside the app (trimming, resampling, convert to mono) requires
  an update to prevent the model from becoming stale.

  @return the number of assets whose properties changed.
*/

qint32 RwaBackend::refreshAssetFileProperties()
{
    qint32 numberOfChangedAssets = 0;

    foreach(RwaScene *scene, scenes)
    {
        foreach(RwaState *state, scene->states)
        {
            foreach(RwaAsset1 *asset, state->assets)
            {
                int64_t oldDuration = asset->getDuration();

                if(!asset->refreshFileProperties())
                    continue;

                numberOfChangedAssets++;
                QString fileName = QString::fromStdString(asset->getFileName());

                if(asset->getDuration() != oldDuration)
                    qInfo() << "Asset file changed on disk:" << fileName << "is now"
                            << asset->getDuration() << "ms, was" << oldDuration << "ms.";
                else
                    qInfo() << "Asset file changed on disk:" << fileName;

                if(asset->getCrossfadeTime() > asset->getDuration())
                    qWarning() << "Crossfade time of" << fileName << "("
                               << asset->getCrossfadeTime() << "ms ) is longer than the file ("
                               << asset->getDuration() << "ms ).";
            }
        }
    }

    return numberOfChangedAssets;
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
//    QRegExp xRegExp("(-?\\d+(?:[\\.,]\\d+(?:e\\d+)?)?)");
//    xRegExp.indexIn(xString);
//    QStringList xList = xRegExp.capturedTexts();
//    if (true == xList.empty())
//        return 0;

//    return xList.begin()->toInt();
    return 0;
}
