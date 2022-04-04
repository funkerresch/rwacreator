

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QGraphicsScene>
#include <QComboBox>
#include <QBoxLayout>
#include <QMenuBar>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QSlider>
#include "rwabackend.h"
#include "rwasearchbox.h"
#include "rwasearchdialog.h"
#include "rwascene.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QLabel)

#define RWATOOLBAR_ALL 4095
#define RWATOOLBAR_SUGGESTPLACES 1 << 0
#define RWATOOLBAR_SELECTSCENEMENU 1 << 1
#define RWATOOLBAR_SELECTSTATEMENU 1 << 2
#define RWATOOLBAR_SELECTASSETMENU 1 << 3
#define RWATOOLBAR_SCENEMENU 1 << 4
#define RWATOOLBAR_SELECTMOUSETOOL 1 << 5
#define RWATOOLBAR_NOPEN 1 << 6
#define RWATOOLBAR_NORUBBER 1 << 7
#define RWATOOLBAR_NOMARKEE 1 << 8
#define RWATOOLBAR_ASSETTOOLS 1 << 9
#define RWATOOLBAR_SIMULATORTOOLS 1 << 10
#define RWATOOLBAR_MAPEDITTOOLS 1 << 11
#define RWATOOLBAR_VIEWMENU 1 << 12
#define RWATOOLBAR_MAINVOLUME 1 << 13
#define RWATOOLBAR_STATEMENU 1 << 14

class RwaViewToolbar : public QToolBar
{
    Q_OBJECT

public:
    RwaViewToolbar(const QString &title, qint32 flags, RwaBackend *backend, QWidget *parent);
    void updateSelectSceneMenu();
    void updateSelectStateMenu();

private:

    void initSceneMenu();
    void initViewPreferencesMenu();
    void setScene(QGraphicsScene *scene);
    void initToolButtonGroup();
    void initControls();

    std::vector<double> coordinates;

    RwaBackend *backend;
    RwaSearchBox *searchEdit;
    RwaSearchDialog *searchDialog;
    RwaScene *currentScene;
    RwaState *currentState;
    RwaAsset1 *currentAsset;
    RwaEntity *currentEntity;

    QBoxLayout *layout;
    QSlider *mainVolume;

    QAction *selectSceneAction;
    QAction *selectStateAction;

    QAction *appendSceneAction;
    QAction *removeSceneAction;
    QAction *copySceneAction;
    QAction *pasteSceneAction;
    QAction *clearSceneAction;

    QAction *copyStateAction;
    QAction *pasteStateAction;
    QAction *newStateAction;
    QAction *newStateFromCurrentAction;
    QAction *newSceneFromSelectedStatesAction;
    QAction *moveScene2NewLocationAction;
    QAction *copySelectedStates2ClipboardAction;
    QAction *pasteClipboardStates2CurrentSceneAction;
    QAction *newGpsStateAction;
    QAction *newBluetoothStateAction;
    QAction *removeStateAction;

    QAction *showMapAction;
    QAction *startStopSimulatorAction;
    QActionGroup *selectSceneGroup;
    QActionGroup *selectStateGroup;

    QToolButton *scanSerialPortsButton;
    QToolButton *calibrateHeadtrackerButton;
    QToolButton *simulateHeadtrackerStepButton;
    QToolButton *trashAssetsButton;
    QToolButton *startSimulatorButton;
    QToolButton *stopSimulatorButton;
    QToolButton *assetsVisibleButton;
    QToolButton *radiiVisibleButton;
    QToolButton *findButton;

    QMenu *selectSceneMenu;
    QMenu *selectStateMenu;
    QMenu *sceneMenu;
    QMenu *stateMenu;
    QMenu *viewPreferencesMenu;
    QButtonGroup *selectTool;

    qint32 numberOfScenes;

    void initStateMenu();
signals:
    void sendSelectedScene(qint32 sceneNumber);
    void sendSelectedScene(RwaScene *scene);
    void sendSelectedState(qint32 stateNumber);
    void sendSelectedState(RwaState *state);

    void sendNewState();
    void sendNewStateFromCurrent();
    void sendNewSceneFromSelectedStates();
    void sendMoveScene2NewLocation();
    void sendCopySelectedStates2Clipboard();
    void sendPasteClipboardStates2CurrentScene();
    void sendNewGpsState();
    void sendNewBluetoothState();

    void sendAppendScene();
    void sendRemoveScene(RwaScene *scene);
    void sendDuplicateScene(RwaScene *scene);
    void sendClearScene(RwaScene *scene);
    void sendMapCoordinates(double lon, double lat);
    void sendSelectedTool(int tool);
    void sendAssetsVisible(bool assetsVisible);
    void sendRadiiVisible(bool radiiVisible);
    void sendTrashAssets(bool onOff);
    void sendStartStopSimulator(bool startStopSimulator);
    void sendCalibrateHeadtracker();
    void sendSimulateHeadtrackerStep();
    void sendScanSerialPorts();
    void sendWriteUndo();
    void sendWriteUndo(QString undoAction);


public slots:
    void setCurrentState(RwaState *currentState);
    void setCurrentState(qint32 stateNumber);
    void setCurrentScene(RwaScene *currentScene);
    void setCurrentScene(qint32 sceneNumber);
    void receiveMapCoordinates(double lon, double lat);
    void receiveTrashAssets(bool onOff);
private slots:

    // scene menu
    void appendScene();
    void removeScene();
    void duplicateScene();
    void clearScene();
    void updateAll();

    // select scene menu
    void selectScene(qint32 sceneNumber);
    void selectState(qint32 stateNumber);
    void setMapCoordinates(double lon, double lat);

    void receiveClickedTool(int tool);
    void receiveAssetsVisible(bool assetsVisible);
    void receiveRadiiVisible(bool radiiVisible);
    void receiveStartSimulator(bool startStopSimulator);
    void receiveStopSimulator(bool startStopSimulator);
    void receiveCurrentScene(RwaScene *scene);
    void receiveCurrentState(RwaState *state);

    void newBluetoothState();
    void newGpsState();
    void newStateFromCurrent();
    void receiveCalibrateHeadtracker(bool onOff);
    void newSceneFromSelectedStates();
    void moveScene2NewLocation();
    void showFindPlacesDialog();
    void receiveSendHeadtrackerStep(bool onOff);
};

#endif
