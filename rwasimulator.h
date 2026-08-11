#ifndef RWASIMULATOR_H
#define RWASIMULATOR_H

#include <QObject>
#include <QTimer>
#include "rwaruntime.h"
#include "rwaaudiooutput.h"
#include "qoscserver.h"
#include "qoscclient.h"
#include "qosctypes.h"
#include "rwaheadtrackerconnect.h"

#define RWA_MAXNUMBEROFPATCHERS 20
#define RWA_MAXNUMBEROF7CHANNELPATCHERS 2
#define RWA_MAXNUMBEROF5CHANNELPATCHERS 2
#define RWA_MAXNUMBEROFDYNAMICPATCHERS 50
#define RWA_MAXNUMBEROFBRIR1PATCHERS 2

class RwaBackend;

class RwaSimulator : public QObject
{
    Q_OBJECT
public:
    explicit RwaSimulator(QObject *parent = 0, RwaBackend *backend = nullptr);
    ~RwaSimulator();

    class oscDevice {
    public:
        QOscClient * oscClient;
        QString name;
        QString ip;
    };

    RwaRuntime *runtime;
    RwaHeadtrackerConnect *headTracker;
    RwaEntity *getEntity(QString name);
    qint32 getSchedulerRate() const;

    void newEntity(QString name, qint32 type);
    void appendEntityAttribute(QString entityName, QString attributeName, double floatValue);
    void appendEntityAttribute(QString entityName, QString attributeName, QString stringValue);
    void renderEntities();
    void setEntityState();
    void sendData2Devices();
    void setSchedulerRate(const qint32 &value);
    void sendSelectedScene2Devices();
    void clearEntities();
    void initGandalf();

    /**
    * @brief Runs Pd's scheduler for a stretch of logical time, with no audio device.
    * @param milliseconds How much Pd time to advance.
    *
    * Only to be called once the PortAudio stream is closed (Pa_AbortStream +
    * Pa_CloseStream), so we are the only ones calling into libpd and nothing is
    * racing the audio callback.
    *
    * Pd's clocks only advance inside libpd_process_float(), which normally runs
    * in the audio callback. After the audio stream is closed nothing advances
    * them any more, so a [delay] scheduled during the teardown of a simulation
    * would stay in the clock queue and fire into the next one. Turning the
    * blocks over by hand lets those clocks finish here, and costs no waiting: a
    * block is 64 samples of logical time and a few microseconds of real time.
    */
    void flushPdScheduler(int milliseconds);

    static QList<RwaEntity *> entities;
    audioProcessor *ap;
    QTimer *gameLoopTimer;
    QTimer *entityTimer;
    qint32 entityUpdateInterval;
    RwaBackend *backend;

    bool step = false;
    bool devicesRegistered;
    bool simulationIsRunning;

    QList<oscDevice *> devices;
    QOscServer* oscServer;
    PathObject *registerPath;
    PathObject *coordinatesPath;
    PathObject *downloadGamesPath;
    PathObject *positionPath;

signals:
    void updateScene();
    void sendRedrawAssets();
    void sendAudioDevicesChanged();

    /** The simulation was started or stopped, no matter from where: the toolbar
     *  button, the Simulation menu and its key commands, or a rescan of the audio
     *  devices, which has to stop a running simulation to restart PortAudio (see
     *  rescanAudioDevices()) and leaves it stopped. Everything showing the state
     *  of the simulation follows this. */
    void sendSimulationRunningChanged(bool running);
    void sendSelectedScene(RwaScene *scene);
    void sendSelectedState(RwaState *state);

public slots:
    void receiveRedrawAssetsFromRuntime();
    void receiveCurrentSceneFromRuntime(RwaScene *scene);
    void receiveCurrentStateFromRuntime(RwaState *state);
    void receiveNewGameSignal();
    void updateRwaGameState();
    void setMainVolume(float volume);
    void updateAssets();
    bool isSimulationRunning();
    void startRwaSimulation();
    void stopRwaSimulation();
    int rescanAudioDevices();
    void clearGame();
    void setCurrentScene(RwaScene *scene);
    void receiveRegisterMessage(QVariant data);
    void receiveDownloadMessage(QVariant data);
    void receiveAzimuth(float azimuth);
    void receiveElevation(float elevation);
    void receiveZ(float Z);
    void receiveLastTouchedScene(RwaScene *scene);
    void receiveEntityPosition(vector<double> position);
    void receiveStep();
    void receiveUndoGameLoaded();
    void receivePositionMessage(QVariant data);

private:
    qint32 schedulerFrequency;
};

#endif // RWASIMULATOR_H
