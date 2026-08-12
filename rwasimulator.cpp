#include "rwasimulator.h"
#include "z_libpd.h"
#include "m_pd.h"
#include "util/z_queued.h"
#include "rwautilities.h"
#include "rwaimport.h"
#include "rwa_binauralsimple~.h"

QList <RwaEntity *> RwaSimulator::entities;

RwaSimulator::RwaSimulator(QObject *parent, RwaBackend *backend) :
    QObject(parent)
{
    this->backend = backend;
    QString puredataPath = backend->completeBundlePath+"/puredata";
    QString assetPath = backend->completeAssetPath;

    initGandalf();
    setSchedulerRate(25);

    gameLoopTimer = new QTimer(this);
    devicesRegistered = false;
    ap = new audioProcessor(1024);
    runtime = new RwaRuntime(this, puredataPath.toStdString().c_str(), assetPath.toStdString().c_str(), ap->getSampleRate(), 25, &ap->pdMutex, backend);
    runtime->entities = std::list(entities.begin(), entities.end());
    simulationIsRunning = 0;
    gameLoopTimer->setInterval(getSchedulerRate());
    oscServer = new QOscServer(8000, nullptr);
    registerPath = new PathObject("/register", QVariant::List, oscServer);
    positionPath = new PathObject("/position", QVariant::List, oscServer);
    downloadGamesPath = new PathObject("/download", QVariant::List, oscServer);
    headTracker = RwaHeadtrackerConnect::getInstance();

    connect (this, SIGNAL(sendSelectedScene(RwaScene *)),
             backend, SLOT(receiveLastTouchedScene(RwaScene *)));

    connect (this, SIGNAL(sendSelectedState(RwaState *)),
             backend, SLOT(receiveLastTouchedState(RwaState *)));

    connect(gameLoopTimer, SIGNAL(timeout()),
            this, SLOT(updateRwaGameState()));

    connect (backend, SIGNAL(sendEntityPosition(vector<double>)),
             this, SLOT(receiveEntityPosition(vector<double>)));

    connect (headTracker, SIGNAL(sendAzimuth(float)),
             this, SLOT(receiveAzimuth(float)));

    connect (headTracker, SIGNAL(sendElevation(float)),
             this, SLOT(receiveElevation(float)));

    connect (headTracker, SIGNAL(sendStep()),
             this, SLOT(receiveStep()));

    connect (backend, SIGNAL(newGameLoaded()),
             this, SLOT(receiveNewGameSignal()));

    connect (backend, SIGNAL(undoGameLoaded()),
             this, SLOT(receiveUndoGameLoaded()));

    connect (backend, SIGNAL(sendLastTouchedScene(RwaScene*)),
             this, SLOT(receiveLastTouchedScene(RwaScene*)));

    connect (runtime, SIGNAL(sendSelectedScene(RwaScene *)),
                 this, SLOT(receiveCurrentSceneFromRuntime(RwaScene *)));

    connect (runtime, SIGNAL(sendSelectedState(RwaState *)),
                 this, SLOT(receiveCurrentStateFromRuntime(RwaState *)));

    connect (runtime, SIGNAL(sendRedrawAssets()),
                 this, SLOT(receiveRedrawAssetsFromRuntime()));

    QObject::connect(registerPath, SIGNAL(data(QVariant) ), this, SLOT( receiveRegisterMessage(QVariant)) );
    QObject::connect(positionPath, SIGNAL(data(QVariant) ), this, SLOT( receivePositionMessage(QVariant)) );

    setMainVolume(1.0);
}

RwaSimulator::~RwaSimulator()
{
    oscServer->deleteLater();
    delete ap;
    foreach(oscDevice *device, devices)
        delete (device);
}

void RwaSimulator::receiveRedrawAssetsFromRuntime()
{
    emit sendRedrawAssets();
}

void RwaSimulator::receiveCurrentSceneFromRuntime(RwaScene *scene)
{
    emit sendSelectedScene(scene);
}

void RwaSimulator::receiveCurrentStateFromRuntime(RwaState *state)
{
    emit sendSelectedState(state);
}

void RwaSimulator::receiveLastTouchedScene(RwaScene *scene)
{
    // while simulating, GUI interactions that change
    // scenes need to be ignored.
    if(simulationIsRunning)
        return;

    RwaEntity *entity;
    foreach(entity, entities)
    {
        if(entity->getCurrentScene() == scene)
            continue;
        entity->setCurrentScene(scene);
        entity->setTimeInCurrentScene(0);
        sendSelectedScene2Devices();
    }
}

void RwaSimulator::receivePositionMessage(QVariant data)
{
    qDebug() << data;
    double lon = data.toList().at(0).toDouble();
    double lat = data.toList().at(1).toDouble();
    std::vector pos = std::vector<double>(2,0);
    pos[0] = lon;
    pos[1] = lat;

    if(simulationIsRunning)
    {
        RwaEntity *entity;
        foreach(entity, entities)
        {
            entity->setCoordinates(pos);
            emit backend->sendHeroPositionEdited();
        }
    }
}

void RwaSimulator::receiveRegisterMessage(QVariant data)
{
    oscDevice *newDevice = new oscDevice;
    newDevice->name = data.toList().at(0).toString();
    newDevice->ip = data.toList().at(1).toString();
    newDevice->oscClient =  new QOscClient( QHostAddress(newDevice->ip), 8001, nullptr );
    qDebug() << "Registered iOS Client " << newDevice->ip;
    devices.append(newDevice);
    devicesRegistered = true;
}

void RwaSimulator::receiveDownloadMessage(QVariant data)
{
    qDebug() << "Start Download GAMES";
    int onOff = data.toList().at(1).toInt();
    qDebug() << "OnOff: " << onOff;
    if(onOff)
    {
        qDebug() << "Start Server";
        backend->StartHttpServer1(8088);
    }
}

void RwaSimulator::initGandalf()
{
    newEntity("Gandalf", RWAENTITYTYPE_HERO);
    appendEntityAttribute("Gandalf", "Sex", "male");
    appendEntityAttribute("Gandalf", "Intelligence", "15");
}

void RwaSimulator::receiveUndoGameLoaded()
{
    qDebug() << "SIMULATOR: received undo signal";
    runtime->entities.front()->scenes = std::list(backend->getScenes().begin(), backend->getScenes().end());
}

void RwaSimulator::receiveNewGameSignal()
{
    qDebug() << "SIMULATOR: received new game signal";
    std::ostringstream path;
    path << backend->completeAssetPath.toStdString() << "/";

    clearEntities();
    initGandalf();
    //runtime->entities = entities.toStdList();
    runtime->entities = std::list(entities.begin(), entities.end());
    runtime->entities.front()->scenes = std::list(backend->getScenes().begin(), backend->getScenes().end());
    runtime->assetPath = path.str();
}

void RwaSimulator::receiveEntityPosition(vector<double> position)
{
    sendData2Devices();
    RwaEntity *entity;
    foreach(entity, entities)
        entity->setCoordinates(position);
}

void RwaSimulator::receiveStep()
{
    qDebug() << "Received Step Event from Headtracker";
    runtime->step = true;

    oscDevice *device;
    RwaEntity *entity;

    foreach(entity, entities)
    {
        string name = entity->objectName();
        foreach(device, devices)
        {
            if(name == (device->name).toStdString())
            {
               device->oscClient->sendData("/step", 1);
               qDebug() << "send step to Device";
            }
        }
    }

    //step = true;
}

void RwaSimulator::receiveAzimuth(float azimuth)
{
    if(entities.empty())
        return;

    RwaEntity *entity = entities.front();
    entity->setAzimuth(static_cast<int32_t>(azimuth));
}

void RwaSimulator::receiveElevation(float elevation)
{
    if(entities.empty())
        return;

    RwaEntity *entity = entities.front();
    entity->setElevation(static_cast<int32_t>(elevation));
}

void RwaSimulator::receiveZ(float Z)
{
    if(entities.empty())
        return;
}

qint32 RwaSimulator::getSchedulerRate() const
{
    return schedulerFrequency;
}

void RwaSimulator::setSchedulerRate(const qint32 &value)
{
    schedulerFrequency = value;
}

// this seems to be dead code (unused, no callers)
void RwaSimulator::setCurrentScene(RwaScene *currentScene)
{
    RwaEntity *entity = entities.front();
    if(entity->getCurrentScene() != currentScene)
    {

        stopRwaSimulation();
        startRwaSimulation();
        entity->setCurrentScene(currentScene);
        if((QObject::sender() == this->backend))
        {
            sendSelectedScene2Devices();
            // qDebug() << QString::fromStdString(currentScene->objectName());
        }
    }
}

void RwaSimulator::clearGame()
{
    RwaEntity *entity;
    foreach(entity, entities)
    {
        //entity->scenes.clear();
        entity->setCurrentScene(nullptr);
        entity->setCurrentState(nullptr);
    }
}

/**
  @brief Re-enumerates the audio devices, see paWrapper::rescanDevices().
  stops a running simulation, as PortAudio has to be restarted for this.
*/
int RwaSimulator::rescanAudioDevices()
{
    if(simulationIsRunning)
        stopRwaSimulationNow();

    int err = ap->rescanDevices();
    if(err != paNoError)
    {
        qWarning() << "Could not rescan the audio devices:" << ap->getErrorText(err);
        return err;
    }

    qDebug() << "Audio devices rescanned. Output device:"
            << ap->getDeviceName(ap->getOutputDevice())
            << "- input device:"
            << ap->getDeviceName(ap->getInputDevice());

    emit sendAudioDevicesChanged();

    return err;
}

void RwaSimulator::startRwaSimulation()
{
    // A start during teardown is queued, not lost:
    // finishStopRwaSimulation() launches it once the reset is complete.
    if(stopInProgress)
    {
        startPending = true;
        return;
    }

    if(simulationIsRunning)
        return;

    // Picks up devices which were connected or removed since the last scan.
    // Without this the stored device indices can be stale and the simulation would run without any audio.
    rescanAudioDevices();

    RwaEntity *entity = entities.front();
    RwaScene *startScene = backend->getLastTouchedScene();
    if(!startScene)
        startScene = backend->getScenes().front();

    runtime->unblockStates(entity);
    runtime->entities.front()->scenes = std::list(backend->getScenes().begin(), backend->getScenes().end());
    runtime->initDynamicPdPatchers(entity);
    runtime->setScene(entity, startScene);

    libpd_init_audio(ap->inputChannelCount() , ap->outputChannelCount(), backend->sampleRate); // 2 inputs, 2 output

    // start silent: the master [line~] in stereoout.pd jumps to 0 before the stream
    // opens, so nothing left standing in the signal graph reaches the first blocks.
    // The ramp up to 1 goes out once the stream runs.
    sendMasterFade(0.0f, 0);

    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");

    int err = ap->startAudio();
    if(err != paNoError)
        qWarning() << "No audio output:" << ap->getErrorText(err)
                   << "- device:" << ap->getDeviceName(ap->getOutputDevice())
                   << "- try Audio Preferences -> Rescan Audio Devices";

    sendMasterFade(1.0f, masterFadeInMs);

    gameLoopTimer->start();
    simulationIsRunning = true;
    sendSelectedScene2Devices();
    emit sendSimulationRunningChanged(true);
}

void RwaSimulator::sendMasterFade(float target, int milliseconds)
{
    ap->pdMutex.lock();
    libpd_start_message(2);
    libpd_add_float(target);
    libpd_add_float(milliseconds);
    libpd_finish_list("rwamasterfade");
    ap->pdMutex.unlock();
}

void RwaSimulator::stopRwaSimulation()
{
    if(!simulationIsRunning)
        return;

    if(stopInProgress)
    {
        // Stop while a queued start waits: the stop wins, the start is forgotten.
        startPending = false;
        return;
    }

    stopInProgress = true;
    gameLoopTimer->stop();

    sendMasterFade(0.0f, masterFadeOutMs);

    QTimer::singleShot(stopTeardownDelayMs, this, [this]{ finishStopRwaSimulation(); });
}

void RwaSimulator::stopRwaSimulationNow()
{
    if(!simulationIsRunning)
        return;

    gameLoopTimer->stop();
    startPending = false;
    stopInProgress = true;
    finishStopRwaSimulation();
    // If a phase-A single-shot is still pending, its finishStopRwaSimulation()
    // no-ops on arrival: simulationIsRunning is false by then.
}

/**
  Phase B of the two-phase stop: the stream is closed first, which makes everything
  below single-threaded - the audio callback calls libpd_process_float() until
  Pa_AbortStream() returns, and only the message sends in RwaRuntime take pdMutex;
  closing patches and "pd dsp 0" do not. See docs/teardown-investigation.md.
*/
void RwaSimulator::finishStopRwaSimulation()
{
    if(!simulationIsRunning)
        return;

    ap->stopAudio();

    runtime->freeAllPatchers();

    // Complete the release protocol for the whole pool, not only the active assets: a
    // patcher whose asset ended but is still fading out has left activeAssets, and its
    // pending [delay] would otherwise survive the stop and fire into the next simulation
    // (the pooled patchers are never closed, so their clocks persist). See
    // RwaRuntime::resetAllPatchers().
    runtime->resetAllPatchers();

    libpd_start_message(1);
    libpd_add_float(0.0f);
    libpd_finish_message("pd", "dsp");

    // Every pooled patcher now has a zero-length fade pending. Pd clocks only advance
    // inside libpd_process_float(), i.e. in the audio callback, and the stream is already
    // closed - so turn 30 ms of blocks over by hand to let every "<tag>-end" [delay]
    // mature here instead of on the first blocks of the next run. 30 ms covers the
    // zero-length fades as well as the fixed [delay 10] some patches use, and costs no
    // waiting: the blocks are computed as fast as the CPU can, into a buffer nobody hears.
    flushPdScheduler(30);

    // The "<tag>-playfinished" bangs the flush produced are in libpd's receive queue.
    // Dispatching them now is harmless - activeAssets is empty and every patcher idle -
    // and it keeps them out of the next simulation. This replaces a drain that was
    // scheduled 100 ms after the stop and could land inside the next run. Teardown log
    // lines reach the Log View at once as well.
    runtime->emptyPdMessageQueue();

    runtime->freeDynamicPdPatchers1();
    clearGame();

    simulationIsRunning = false;
    stopInProgress = false;
    emit sendSimulationRunningChanged(false);

    if(startPending)
    {
        startPending = false;
        startRwaSimulation();
    }
}

void RwaSimulator::flushPdScheduler(int milliseconds)
{
    const int blockSize = libpd_blocksize();
    const int sampleRate = backend->sampleRate > 0 ? backend->sampleRate : 48000;
    const int blocks = qMax(1, qRound((milliseconds / 1000.0) * sampleRate / blockSize));

    std::vector<float> in(size_t(blockSize) * size_t(qMax(1, ap->inputChannelCount())), 0.0f);
    std::vector<float> out(size_t(blockSize) * size_t(qMax(1, ap->outputChannelCount())), 0.0f);

    ap->pdMutex.lock();
    for(int i = 0; i < blocks; i++)
        libpd_process_float(1, in.data(), out.data());
    ap->pdMutex.unlock();
}

void RwaSimulator::setMainVolume(float volume)
{
    ap->pdMutex.lock();
    libpd_float("rwamainvolume", volume);
    ap->pdMutex.unlock();
}

bool RwaSimulator::isSimulationRunning()
{
    if(simulationIsRunning)
        return true;
    return false;
}

void RwaSimulator::newEntity(QString name, qint32 type)
{
    RwaEntity *newEntity = new RwaEntity(name.toStdString(), type);
    entities.append(newEntity);
}

void RwaSimulator::clearEntities()
{
    entities.clear();
}

RwaEntity *RwaSimulator::getEntity(QString name)
{
    RwaEntity *ptr;
    foreach( ptr, entities )
    {
        if(ptr->objectName() == name.toStdString())
            return ptr;
    }
    return nullptr;
}

void RwaSimulator::appendEntityAttribute(QString entityName, QString attributeName, double floatValue)
{
    RwaEntity *ptr = getEntity(entityName);
    if(ptr)
         ptr->addAttribute(attributeName.toStdString(), floatValue);
}

void RwaSimulator::appendEntityAttribute(QString entityName, QString attributeName, QString stringValue)
{
    RwaEntity *ptr = getEntity(entityName);
    if(ptr)
        ptr->addAttribute(attributeName.toStdString(), stringValue.toStdString());
    else
        qDebug() << "Could not find entity!";
}

void RwaSimulator::renderEntities()
{
    RwaEntity *ptr;
    foreach( ptr, entities )
    {
        qDebug() << QString::fromStdString(ptr->objectName());
        qDebug() << QString::fromStdString(ptr->getStringAttribute("Sex"));
        qDebug() << QString::fromStdString(ptr->getStringAttribute("Intelligence"));
    }
}

void RwaSimulator::updateAssets()
{
    qDebug() << "updateAssets";
}

void RwaSimulator::sendSelectedScene2Devices()
{
    RwaEntity *entity;
    oscDevice *device;

    foreach(entity, entities)
    {
        string name = entity->objectName();
        foreach(device, devices)
        {
            if(name == (device->name).toStdString())
            {
               device->oscClient->sendData("/currentscene", QString::fromStdString(entity->getCurrentScene()->objectName()));
            }
        }
    }
}

void RwaSimulator::sendData2Devices()
{
    RwaEntity *entity;
    oscDevice *device;

    foreach(entity, entities)
    {
        string name = entity->objectName();
        foreach(device, devices)
        {
            if(name == (device->name).toStdString())
            {
               device->oscClient->sendData("/lon", entity->getCoordinates()[0]);
               device->oscClient->sendData("/lat", entity->getCoordinates()[1]);
               //qDebug() << "Send Entity Position";
            }
        }
    }
}

void RwaSimulator::updateRwaGameState()
{
    RwaEntity *entity = entities.front();
    runtime->update(entity);
    emit backend->sendRedrawAssets();
}
