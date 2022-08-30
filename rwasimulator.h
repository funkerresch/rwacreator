#ifndef RWASIMULATOR_H
#define RWASIMULATOR_H

#include <QObject>
#include <QTimer>
#include "rwaruntime.h"
#include "rwaaudiooutput.h"
#include "qoscserver.h"
#include "qoscclient.h"
#include "qosctypes.h"
#include "bluetooth/device.h"
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
    void newEntity(QString name, qint32 type);
    void appendEntityAttribute(QString entityName, QString attributeName, double floatValue);
    void appendEntityAttribute(QString entityName, QString attributeName, QString stringValue);
    void renderEntities();
    void setEntityState();
    void sendData2Devices();

    static QList<RwaEntity *> entities;
    audioProcessor *ap;
    QTimer *gameLoopTimer;
    QTimer *entityTimer;
    qint32 entityUpdateInterval;
    bool simulationIsRunning;
    RwaBackend *backend;

    bool step = false;
    bool devicesRegistered;

    QList<oscDevice *> devices;
    QOscServer* oscServer;

    PathObject *registerPath;
    PathObject *coordinatesPath;
    PathObject *downloadGamesPath;

    qint32 getSchedulerRate() const;
    void setSchedulerRate(const qint32 &value);

    void sendSelectedScene2Devices();

    void clearEntities();
    void initGandalf();

signals:
    void updateScene();
    void sendRedrawAssets();
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
    void clearGame();
    void setCurrentScene(RwaScene *scene);
    void receiveRegisterMessage(QVariant data);
    void receiveDownloadMessage(QVariant data);
    void receiveAzimuth(float azimuth);
    void receiveElevation(float elevation);
    void receiveZ(float Z);
    void receiveLastTouchedScene(RwaScene *scene);
//    void receiveEntityPosition(QPointF position);
    void receiveEntityPosition(vector<double> position);
    void receiveStep();

    void receiveUndoGameLoaded();

private:
    qint32 schedulerFrequency;


 public:

    float getChannelCount(QString absoluteAssetPath);
};

#endif // RWASIMULATOR_H
