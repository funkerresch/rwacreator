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
    QString path = backend->completeBundlePath;
    QString assetPath = backend->completeAssetPath;

    initGandalf();
    setSchedulerRate(25);

    gameLoopTimer = new QTimer(this);
    devicesRegistered = false;
    ap = new audioProcessor(1024);
    runtime = new RwaRuntime(this, path.toStdString().c_str(), assetPath.toStdString().c_str(), ap->getSampleRate(), 25, &ap->pdMutex, backend);
    runtime->entities = entities.toStdList();
    simulationIsRunning = 0;
    gameLoopTimer->setInterval(getSchedulerRate());
    oscServer = new QOscServer(8000, nullptr);
    registerPath = new PathObject("/register", QVariant::List, oscServer);
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

    connect (backend, SIGNAL(sendDisconnectHeadtracker()),
             this, SLOT(receiveDisconnectHeadtracker()));

    connect (runtime, SIGNAL(sendSelectedScene(RwaScene *)),
                 this, SLOT(receiveCurrentSceneFromRuntime(RwaScene *)));

    connect (runtime, SIGNAL(sendSelectedState(RwaState *)),
                 this, SLOT(receiveCurrentStateFromRuntime(RwaState *)));

    connect (runtime, SIGNAL(sendRedrawAssets()),
                 this, SLOT(receiveRedrawAssetsFromRuntime()));

    QObject::connect(registerPath, SIGNAL(data(QVariant) ), this, SLOT( receiveRegisterMessage(QVariant)) );
    //QObject::connect(downloadGamesPath, SIGNAL(data(QVariant) ), this, SLOT( receiveDownloadMessage(QVariant)) );

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
    //sendSelectedScene2Devices();
}

void RwaSimulator::receiveCurrentStateFromRuntime(RwaState *state)
{
    emit sendSelectedState(state);
}

float RwaSimulator::getChannelCount(QString absoluteAssetPath)
{
    libpd_symbol("filename4metadata", absoluteAssetPath.toLatin1());
}

void RwaSimulator::receiveLastTouchedScene(RwaScene *scene)
{
    RwaEntity *entity;
    foreach(entity, entities)
    {
       // sendEnd2backgroundAssets(entity);
        entity->setCurrentScene(scene);
        entity->setTimeInCurrentScene(0);
        if(simulationIsRunning)
            runtime->setEntityStartCoordinates(entity);

        sendSelectedScene2Devices();
    }
}

void RwaSimulator::receiveRegisterMessage(QVariant data)
{
    oscDevice *newDevice = new oscDevice;
    newDevice->name = data.toList().at(0).toString();
    newDevice->ip = data.toList().at(1).toString();
    newDevice->oscClient =  new QOscClient( QHostAddress(newDevice->ip), 8000, nullptr ); //FOR MAX CHANGE THIS TO 8001
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
    runtime->entities.front()->scenes = backend->getScenes().toStdList();
}

void RwaSimulator::receiveNewGameSignal()
{
    qDebug() << "SIMULATOR: received new game signal";
    std::ostringstream path;
    path << backend->completeAssetPath.toStdString() << "/";

    clearEntities();
    initGandalf();
    runtime->entities = entities.toStdList();
    runtime->entities.front()->scenes = backend->getScenes().toStdList();
    runtime->assetPath = path.str();
}

void RwaSimulator::receiveEntityPosition(vector<double> position)
{
    sendData2Devices();
    RwaEntity *entity;
    qDebug() << "Entity Coordinates";
    foreach(entity, entities)
    {
        entity->setCoordinates(position);
    }
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
           // qDebug() << currentScene->objectName();

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

void RwaSimulator::startRwaSimulation()
{
    RwaEntity *entity = entities.front();
    runtime->unblockStates(entity);
    runtime->entities.front()->scenes = backend->getScenes().toStdList();

    if(backend->getLastTouchedScene())
        entity->setCurrentScene(backend->getLastTouchedScene());
    else
        entity->setCurrentScene(backend->getScenes().front());

    if(!entity->getCurrentScene()->fallbackDisabled())
        entity->setCurrentState(entity->getCurrentScene()->states.front());

    runtime->initDynamicPdPatchers(entity);
    libpd_init_audio(ap->inputChannelCount() , ap->outputChannelCount(), 44100); // 2 inputs, 2 output
    libpd_start_message(1);
    libpd_add_float(1.0f);
    libpd_finish_message("pd", "dsp");
    ap->startAudio();
    gameLoopTimer->start();
    simulationIsRunning = true;
    sendSelectedScene2Devices();
    runtime->setEntityStartCoordinates(entity);
}

void RwaSimulator::stopRwaSimulation()
{
    RwaEntity *entity = entities.front();
    runtime->freeAllPatchers();

    libpd_start_message(1);
    libpd_add_float(0.0f);
    libpd_finish_message("pd", "dsp");
    gameLoopTimer->stop();
    simulationIsRunning = false;
    runtime->freeDynamicPdPatchers1();
    clearGame();
    ap->stopAudio();

    emit updateScene();
}

void RwaSimulator::setMainVolume(float volume)
{
    ap->pdMutex.lock();
    libpd_float("rwamainvolume", volume);
    qDebug("%f", volume);
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
}

