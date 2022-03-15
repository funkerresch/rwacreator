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
    QStringList currentlySelectedAssets;
    QStringList currentlySelectedStates;
    qint32 httpProcessId;

    RwaScene *getLastTouchedScene();
    QList<RwaScene *> &getScenes();
    RwaScene *getFirstScene();
    RwaScene *getSceneAt(qint32 sceneNumber);
    qint32 getNumberOfScenes();
    qint32 generateAssetId();

    void setLastTouchedScene(RwaScene *scene);
    RwaScene *getScene(QString sceneName);

    void scanSerialPorts();
    void setHeadtrackerName(QString name);

    bool trashAsset = false;
    bool logCoordinates = false;
    bool logPd = false;
    bool logSim = false;
    bool logOther = false;

    int getStateNameCounter(std::list<RwaState *> &states);  

    void clearForHistory();
public slots:

    void generateUuidsForClipboardState(RwaState *state);
    void receiveLastTouchedAsset(RwaAsset1 *item);
    void receiveLastTouchedState(RwaState *state);
    void receiveLastTouchedScene(RwaScene *scene);
    void receiveLastTouchedScene(qint32 sceneNumber);
    void receiveCurrentStateEdited();
    void receiveMoveCurrentState1(double dx, double dy);
    void receiveMoveCurrentAsset1(double dx, double dy);
    void receiveMoveCurrentAssetChannel(double dx, double dy, int channel);
    void receiveCurrentStateRadiusEdited();
    void receiveCurrentSceneRadiusEdited();
    void receiveUpdatedAssets();
    void receiveMapCoordinates(QPointF);
    void receiveSceneName(RwaScene *scene, QString name);
    void receiveStateName(RwaState *state, QString name);
    void receiveCurrentStateString(RwaState *state, QString stateName);

    void receiveRedrawAssets();
    void receiveSelectedAssets(QStringList assets);
    void receiveSelectedStates(QStringList states);
    void receiveEntityPosition(QPointF position);
    void receiveStatePosition(QPointF position);

    void receiveLogLonAndLat(int onOff);
    void receiveLogLibPd(int onOff);
    void receiveLogSimulator(int onOff);
    void receiveLogOther(int onOff);

    void receiveTrashAssets(bool onOff);
    void startStopSimulator(bool startStop);  
    void setMainVolume(int volume);
    void calibrateHeadtracker();

    void appendScene();
    void appendScene(RwaScene *scene);
    void clearScene(RwaScene *scene);
    void removeScene(RwaScene *scene);
    void duplicateScene(RwaScene *scene);
    void clearData();
    void clearScenes();
    void emptyTmpDirectories(); 
    bool isSimulationRunning();

    void newSceneFromSelectedStates();
    void clear();
    void moveScene2CurrentMapLocation();
    void copySelectedStates2Clipboard();
    void pasteStatesFromClipboard();

    void receiveReadUndoFile(QString name);
    void receiveWriteUndo(QString undoAction);
    void receiveMoveCurrentAssetReflection(double dx, double dy, int channel);

    void StartHttpServer(qint32 port);
signals:
    void readUndoFile(QString name);
    void sendWriteUndo(QString undoAction);
    void sendLastTouchedState(RwaState *state);
    void sendLastTouchedScene(RwaScene *scene);
    void sendLastTouchedAsset(RwaAsset1 *asset);
    void sendLastTouchedEntity(RwaEntity *entity);
    void sendSelectedAssets(QStringList assets);
    void sendSelectedStates(QStringList states);

    void updateScene(RwaScene *scene);
    void sendClearAll();
    void sendEditedAsset(RwaState *state, RwaAsset1 *item);
    void sendNewAsset(RwaState *state, RwaAsset1 *item);
    void sendAddTts2CurrentState(QString fullpath);
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
    void sendEntityPosition(QPointF position);
    void sendStatePosition(QPointF position);

private:

    QPointF currentMapCoordinates;
    QList<RwaScene *> scenes;
    RwaScene *clipboardStates;
    RwaState *lastTouchedState;
    RwaScene *lastTouchedScene;
    RwaEntity *lastTouchedEntity;
    RwaAsset1 *lastTouchedAssetItem;
    RwaHeadtrackerConnect *headtracker;

    int undoCounter = -1;

};

#endif
