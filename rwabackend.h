/*
*
* This file is part of RwaCreator
* an open-source cross-platform Middleware for creating interactive Soundwalks
*
* Copyright (C) 2015 - 2022 Thomas Resch
*
* License: MIT
*
* The RwaBackend class contains the data model in the form of a linked list of rwaScenes.
* It is realised a singleton so all views/editors can easily access it.
* Almost all GUI updates are realised by sending the lastTouched.*.() signals from the
* corresponding view to the backend which then emits the sendLastTouched.*. signal
* to synchronize all other views/editors. The only exception are the list views where states, scenes
* and assets can be deleted vie keyboard using the basic QListWidget event.
* Otherwise the setCurrent.*.() functions should only be called using Qt's SIGNALS/SLOTS.
*
*/

#ifndef AFXSCENEBACKEND_H
#define AFXSCENEBACKEND_H

#include <QTimer>
#include <QTextEdit>
#include <QRegularExpression>
#include "rwascene.h"
#include "rwaentity.h"
#include "rwasimulator.h"
#include "bluetooth/device.h"
#include "httplib.h"

#define RWATOOL_ARROW 1
#define RWATOOL_PEN 2
#define RWATOOL_RUBBER 3
#define RWATOOL_MARKEE 4

/** ************************************** Game Server ********************************************** */

class RwaGamesServer : public QObject {
    Q_OBJECT
public:
    RwaGamesServer(httplib::Server *svr, int port);
    ~RwaGamesServer();

public slots:
    void process();

private:
    httplib::Server *svr; // Reference is passed from backend in order to call stop() from main thread
    int port;
};

/** **************************************Rwa Backend ********************************************** */

class RwaBackend : public QTextEdit
{
    Q_OBJECT

public:

    explicit RwaBackend(QWidget *parent = nullptr);
    ~RwaBackend();
    static RwaBackend *instance;
    static RwaBackend *getInstance();

    httplib::Server svr;
    void StartHttpServer(qint32 port);
    void StartHttpServer1(qint32 port);

    RwaSimulator *simulator;
    QString projectName;
    QString completeBundlePath;
    QString completeProjectPath;
    QString completeFilePath;
    QString completeUndoPath;
    QString completeTmpPath;
    QString completeAssetPath;
    QString completeClientExportPath;
    QString completeClientProjectExportPath;
    QString completeClientFileExportPath;
    QString completeClientAssetExportPath;
    QStringList assetStringList;
    QStringList stateStringList;
    QStringList currentlySelectedAssets;
    QStringList currentlySelectedStates;
    qint32 httpProcessId;

    bool trashAsset = false;
    bool showStateRadii = false;
    bool showAssets = false;
    bool heroFollowsSceneAndState = false;
    bool logCoordinates = false;
    bool logPd = false;
    bool logSim = false;
    bool logOther = false;

    QByteArray stateViewGeometry;
    QByteArray stateViewState;

private:

    QPointF currentMapCoordinates;
    QList<RwaScene *> scenes;
    RwaScene *clipboardStates;
    RwaState *lastTouchedState = nullptr;
    RwaScene *lastTouchedScene = nullptr;
    RwaEntity *lastTouchedEntity = nullptr;
    RwaAsset1 *lastTouchedAssetItem = nullptr;
    RwaHeadtrackerConnect *headtracker = nullptr;
    QThread *serverThread = nullptr;

    void updateLastTouchedSceneStateAndAsset();

    void StopHttpServer1();
public slots:

    /** *********************************Undo read and write********************************************** */

    void receiveReadUndoFile(QString name);
    void receiveWriteUndo(QString undoAction);

    /** ****************************** Last touched/selected functionality ******************************* */

    void receiveLastTouchedAsset(RwaAsset1 *item);
    void receiveLastTouchedState(RwaState *state);
    void receiveLastTouchedScene(RwaScene *scene);
    void receiveLastTouchedScene(qint32 sceneNumber);
    void receiveSelectedAssets(QStringList assets);
    void receiveSelectedStates(QStringList states);
    void receiveSelectedScenes(QStringList scenes);

    /** ********************************** Scene-related functionality *********************************** */

    /** ****************************************** Get methods ******************************************* */

    RwaScene *getLastTouchedScene();
    QList<RwaScene *> &getScenes();
    RwaScene *getFirstScene();
    RwaScene *getSceneAt(qint32 sceneNumber);
    qint32 getNumberOfScenes();
    RwaScene *getScene(QString sceneName);

    /** *********************************** New/Remove Scene functionality ***************************** */

    void appendScene();
    void appendScene(RwaScene *scene);
    void duplicateScene();
    void newSceneFromSelectedStates();

    /** ************************ Clear/Remove/Reset Scene(s) functionality **************************** */

    void removeScene(QString sceneName);
    void removeScene(RwaScene *scene);
    void clearScene(RwaScene *scene);
    void clearScenes();
    void reset();

    /** ************************************* Location functionality ********************************** */

    void moveScene2CurrentMapLocation();
    void receiveMapCoordinates(QPointF);

    /** ************************************  State copy and paste *********************************** */

    void copySelectedStates2Clipboard();
    void pasteStatesFromClipboard();

    /** ************************  RWA Graphic View receiver to emitter functions ********************* */

    void receiveMoveCurrentState1(double dx, double dy);
    void receiveMoveCurrentAsset1(double dx, double dy);
    void receiveMoveCurrentAssetChannel(double dx, double dy, int channel);
    void receiveMoveCurrentAssetReflection(double dx, double dy, int channel);
    void receiveCurrentStateRadiusEdited();
    void receiveCurrentSceneRadiusEdited();
    void receiveStatePosition(QPointF position);
    void receiveEntityPosition(QPointF position);

    /** ************************* Editor Global Rendering/Functionality ***************************** */

    void receiveTrashAssets(bool onOff);
    void receiveActivateClientSync(bool onOff);
    void receiveShowStateRadii(bool onOff);
    void receiveShowAssets(bool onOff);
    void receiveHeroFollowsSceneAndState(bool onOff);


     /** **************************"""**** Logging related functions ***************************** */

    /**
     * @brief receiveLogLonAndLat
     * @param onOff
     */

    void receiveLogLonAndLat(int onOff);

    /**
     * @brief receiveLogLibPd
     * @param onOff
     */

    void receiveLogLibPd(int onOff);

    /**
     * @brief receiveLogSimulator
     * @param onOff
     */

    void receiveLogSimulator(int onOff);

    /**
     * @brief receiveLogOther
     * @param onOff
     */

    void receiveLogOther(int onOff);

    /** ****************************** Simulator and headtracker related functions ***************************** */

    /**
     * @brief startStopSimulator
     * @param startStop
     */

    void startStopSimulator(bool startStop);

    /**
     * @brief setMainVolume
     * @param volume
     */

    void setMainVolume(int volume);

    /**
     * @brief calibrateHeadtracker
     */

    void calibrateHeadtracker();

    /**
     * @brief isSimulationRunning
     * @return
     */

    bool isSimulationRunning();

     /** *********************** Static utility functionality (string generation, UUIDs, ..) ************************* */

    void receiveMoveHero2CurrentState();
    void receiveMoveHero2CurrentScene();
    void receiveMoveCurrentScene();
    void receiveReadNewGame();
public:

    static void generateUuidsForClipboardState(RwaState *state);
    static void adjust2UniqueStateName(RwaScene *targetScene, RwaState *newState);
    static bool adjust2UniqueStateNameRecursively(RwaScene *targetScene, RwaState *newState);
    static int getStateNameCounter(std::list<RwaState *> &states);
    static int getNumberFromQString(const QString &xString);

    /** ************************************************** Signals ************************************************** */

    bool fileUsedByAnotherAsset(RwaAsset1 *asset2Delete);
    bool adjust2UniqueSceneNameRecursively(RwaScene *newScene);
    void adjust2UniqueSceneName(RwaScene *newScene);
signals:
    void readUndoFile(QString name);
    void sendWriteUndo(QString undoAction);
    void sendLastTouchedState(RwaState *state);
    void sendLastTouchedScene(RwaScene *scene);
    void sendLastTouchedAsset(RwaAsset1 *asset);
    void sendLastTouchedEntity(RwaEntity *entity);
    void sendSelectedAssets(QStringList assets);
    void sendSelectedStates(QStringList states);
    void sendMoveHero2CurrentState();
    void sendMoveHero2CurrentScene();
    void updateScene(RwaScene *scene);
    void updateGame();
    void sendClearAll();
    void sendNewAsset(RwaState *state, RwaAsset1 *item);
    void sendMoveCurrentScene();
    void sendMovePixmapsOfCurrentState1(double dx, double dy);
    void sendMovePixmapsOfCurrentAsset1(double dx, double dy);
    void sendMoveCurrentAssetChannel(double dx, double dy, int channel);
    void sendMoveCurrentAssetReflection(double dx, double dy, int channel);
    void sendCurrentStateRadiusEdited();
    void sendCurrentSceneRadiusEdited();
    void sendHeroPositionEdited();
    void updateAssets();  // update simulator if assets are updated while simulation runs..
    void newGameLoaded();
    void undoGameLoaded();
    void sendRedrawAssets();
    void sendEntityPosition(vector<double> position);
    void sendStatePosition(QPointF position);
    void stopServer(int i);
};

#endif
