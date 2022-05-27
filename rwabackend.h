#ifndef AFXSCENEBACKEND_H
#define AFXSCENEBACKEND_H

#include <QTimer>
#include <QTextEdit>
#include <QRegularExpression>
#include "rwascene.h"
#include "rwaentity.h"
#include "rwasimulator.h"
#include "bluetooth/device.h"

#define RWATOOL_ARROW 1
#define RWATOOL_PEN 2
#define RWATOOL_RUBBER 3
#define RWATOOL_MARKEE 4

class RwaBackend : public QTextEdit
{
    Q_OBJECT

public:

    explicit RwaBackend(QWidget *parent = nullptr);
    ~RwaBackend();
    static RwaBackend *instance;
    static RwaBackend *getInstance();

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

public slots:

    /** *********************************Undo read and write*********************************** */

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
    void duplicateScene(RwaScene *scene);
    void newSceneFromSelectedStates();
    void removeScene(RwaScene *scene);

    void clearScene(RwaScene *scene);
    void clearData();
    void clearScenes();
    void emptyTmpDirectories(); 

    void clear();
    void moveScene2CurrentMapLocation();
    void receiveMapCoordinates(QPointF);
    void StartHttpServer(qint32 port);

    /** ************************************  State copy and paste *********************************** */

    void copySelectedStates2Clipboard();
    void pasteStatesFromClipboard();

    /** *******************  RWA Graphic View receiver to emitter functions ********************* */

    void receiveMoveCurrentState1(double dx, double dy);
    void receiveMoveCurrentAsset1(double dx, double dy);
    void receiveMoveCurrentAssetChannel(double dx, double dy, int channel);
    void receiveMoveCurrentAssetReflection(double dx, double dy, int channel);
    void receiveCurrentStateRadiusEdited();
    void receiveCurrentSceneRadiusEdited();
    void receiveStatePosition(QPointF position);
    void receiveEntityPosition(QPointF position);

    /** ************************* Editor Global Rendering/Functionality ************************** */

    void receiveTrashAssets(bool onOff);
    void receiveShowStateRadii(bool onOff);
    void receiveShowAssets(bool onOff);


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

public:

    static void generateUuidsForClipboardState(RwaState *state);
    static void adjust2UniqueStateName(RwaScene *targetScene, RwaState *newState);
    static bool adjust2UniqueStateNameRecursively(RwaScene *targetScene, RwaState *newState);
    static int getStateNameCounter(std::list<RwaState *> &states);
    static int getNumberFromQString(const QString &xString);

    /** ************************************************** Signals ************************************************** */

signals:
    void readUndoFile(QString name);
    void sendWriteUndo(QString undoAction);
    void sendLastTouchedState(RwaState *state);
    void sendLastTouchedScene(RwaScene *scene);
    void sendLastTouchedAsset(RwaAsset1 *asset);
    void sendLastTouchedEntity(RwaEntity *entity);
    void sendSelectedAssets(QStringList assets);
    void sendSelectedStates(QStringList states);

    void sendAppendScene();
    void updateScene(RwaScene *scene);
    void sendClearAll();
    void sendNewAsset(RwaState *state, RwaAsset1 *item);
    void sendMoveCurrentState1(double dx, double dy);
    void sendMoveCurrentAsset1(double dx, double dy);
    void sendMoveCurrentAssetChannel(double dx, double dy, int channel);
    void sendMoveCurrentAssetReflection(double dx, double dy, int channel);
    void sendCurrentStateRadiusEdited();
    void sendCurrentSceneRadiusEdited();
    void updateAssets();  // update simulator if assets are updated while simulation runs..
    void sendSave();
    void newGameLoaded();
    void undoGameLoaded();
    void sendRedrawAssets();
//    void sendEntityPosition(QPointF position);
    void sendEntityPosition(vector<double> position);
    void sendStatePosition(QPointF position);
};

#endif
