/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * The RwaCreator class is the main window. On startup, it loads all views/editors.
 * It is also responsible for all File I/O.
 *
 *
 * ToDo next:
 * Make select states in Map GUI working
 * Renaming assets and assets with same name in same state need a solution
 * Toolbar in Stateview ??
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGlobal>
#include <QMainWindow>
#include <QAudioOutput>
#include <QTextEdit>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QDir>
#include <QtSerialPort/QSerialPortInfo>
#include "rwaimport.h"
#include "rwaexport.h"
#include "rwabackend.h"
#include "rwaheadtrackerconnect.h"
#include "rwamapview.h"
#include "rwastateview.h"
#include "rwaentity.h"
#include "rwasceneview.h"
#include "rwalogview.h"
#include "rwagameview.h"
#include "rwahistory.h"

QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QSignalMapper)

/** ***************************************** Subclassed QDockWidget ************************************************** */

class RwaDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    RwaDockWidget(QWidget *parent = nullptr, const QString& title = "Default")
        : QDockWidget(title, parent)
    {
        QWidget *titleBar = new QWidget(this);
        titleBar->setAutoFillBackground(true);
        titleBar->setStyleSheet(R"(
            QWidget {
                background-color: #2c2c2c;
            }
            QLabel {
                color: white;
                font: bold 11pt "Arial";
            }
            QToolButton {
                background: transparent;
                border: none;
                padding: 2px;
            }
            QToolButton:hover {
                background-color: rgba(255,255,255,0.15);
                border-radius: 4px;
            }
        )");

        QHBoxLayout *layout = new QHBoxLayout(titleBar);
        layout->setContentsMargins(6, 0, 6, 0);
        layout->setSpacing(2);

        QLabel *label = new QLabel(title, titleBar);
        layout->addWidget(label);
        layout->addStretch();

        QToolButton *floatButton = new QToolButton(titleBar);
        floatButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
        floatButton->setToolTip("Float / Dock");
        layout->addWidget(floatButton);

        connect(floatButton, &QToolButton::clicked, this, [this]() {
            setFloating(!isFloating());
        });

        QToolButton *closeButton = new QToolButton(titleBar);
        closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
        closeButton->setToolTip("Close");
        layout->addWidget(closeButton);
        connect(closeButton, &QToolButton::clicked, this, &QDockWidget::close);
        setTitleBarWidget(titleBar);
    }
};

/*************************************** Rwa Creator Application Main Window ***************************************** */

class RwaCreator : public QMainWindow
{
    Q_OBJECT

public:
    RwaCreator(QWidget *parent = nullptr);
    //~RwaCreator(); //  nothing to do because all free functions are called through Qt's parent/child system.

private:
    static RwaLogWindow *logWindow;
    RwaBackend *backend = nullptr;
    RwaMapView *mapView = nullptr;
    RwaStateView *stateView = nullptr;
    RwaHeadtrackerConnect *headtracker = nullptr;
    QList<RwaDockWidget *> rwaDockWidgets = QList<RwaDockWidget *>();
    QSignalMapper *mapper = nullptr;
    QMenu *destroyDockWidgetMenu = nullptr;
    QMenu *headtrackerMenu = nullptr;
    QMenu *audioPreferencesMenu = nullptr;
    QActionGroup *selectSampleRateGroup = nullptr;
    QActionGroup *selectAudioOutputDeviceGroup = nullptr;
    QActionGroup *selectAudioInputDeviceGroup = nullptr;
    QAction *createDockWidgetAction = nullptr;
    QAction *defaultViewWidgetAction = nullptr;
    QAction *selectAudioDeviceAction = nullptr;
    QAction *selectSampleRateAction = nullptr;
    QAction *selectHeadtrackerAction = nullptr;
    QAction *runSimulationAction = nullptr;
    QAction *stopSimulationAction = nullptr;
    QString headtrackerSerialPort = QString();
    qint32 headtrackerSerialPortIndex = -1;
    bool allViewsLoaded = false;
    quint32 undoCounter = 0;

/**
 * @brief Implemented for RwaCreator for debugging purposes. Currently not in use. <br>
 * @param event The QShowEvent to be handled. <br>
 * Implemented for RwaCreator for debugging purposes. Currently not in use. <br>
 * Could do something with show events of all RWA views
 */

    void showEvent(QShowEvent *event);

/**
 * @brief Implemented as Callback for Resizing Events.<br>
 * @param event The QEvent to be handled. <br>
 * @param obj The QObject sending the event. <br>
 * @return Returns QWidget::eventFilter(obj, event) <br>
 * The eventFilter is installed in order to handle resize <br>
 * events of the main window. The MapViews' mapController must <br>
 * be resized manually here.
 */
    bool eventFilter(QObject *obj, QEvent *event);

/** **************************************** Main application menu ****************************************** */

/**
 * @brief Creates the main menu bar.<br>
 * Creates the main menu bar for all menus: <br>
 * File menu <br>
 * View menu <br>
 * Edit menu <br>
 * Audio Preferences menu <br>
 * Head tracker menu
 */

    void setupMenuBar();

/**
 * @brief Creates the file menu.<br>
 * @param fileMenu The the new menu.
 * Creates the file menu. <br>
 */

    void initFileMenu(QMenu *fileMenu);

/**
 * @brief Creates the simulation menu.
 * @param simulationMenu The the new menu.
 * Creates the simulation menu, which starts and stops the
 * simulation and carries the key commands for it. The same
 * can be done with the start/stop buttons of the Map View toolbar.
 */

    void initSimulationMenu(QMenu *simulationMenu);

/**
 * @brief Creates the audio prefs menu.
 * @param audioDeviceMenu The the new menu.
 * Creates the audio preferences menu. Enables the user
 * to rescan the audio devices and to select input/output
 * device and the sample rate. Can be called again to rebuild
 * the menu from the current device list.
 */

    void initAudioPreferencesMenu(QMenu *audioDeviceMenu);

/**
 * @brief Creates the head tracker menu.<br>
 * @param headtrackerMenu The the new menu.
 * Creates head tracker menu. Enables the user to <br>
 * set the head tracker name and connect/disconnect via <br>
 * bluetooth.
 */

    void initHeadtrackerMenu(QMenu *headtrackerMenu);

/**
 * @brief Creates the view menu.<br>
 * @param fileMenu The the new menu.
 * Creates the view menu. Enables the user <br>
 * to attach a new RWA view to the main window <br>
 * and to make existing views visible again. <br>
 */

    void initViewMenu1(QMenu *fileMenu);

/**
 * @brief Empties tmp directories.<br>
 * Empties the tmp directories TileCache, <br>
 * tmp and undo inside the current game folder
 */

    void emptyTmpDirectories();

/**
 * @brief Asks if the game should be saved. <br>
 * @return Returns 1 if the game was successfully saved <br>
 * Currently always called on quitting the RWACreator application. <br>
 * A pop-up dialogue asks the user, if she wants to save the game.
 */

    bool maybeSave();

/**
 * @brief Helper function for saving/exporting RWA games. <br>
 * @param fullpath The path and name of the game to be saved. <br>
 * @param flags Enables to configure for save, export or export for client. <br>
 * Helper function for saving and exporting RWA games.
 */

    void prepareWrite1(QString fullpath, int flags);

/**
 * @brief Writes RWA .xml files to disk. <br>
 * @param writeMessage Message to appear in the Main Window. <br>
 * @param flags Enables to configure for save, export or export for client. <br>
 * @param newCompleteFilePath Complete file path and name of the game to be saved. <br>
 * Depending on the flags: save, saveas, export, export for client <br>
 */

    void write1(QString writeMessage, qint32 flags, QString newCompleteFilePath);

/**
 * @brief Converts menu index to integer & string sample rate  <br>
 * @param i selected index from the menu. <br>
 * @param sr_int Passed integer address where the sample rate is written to.<br>
 * @param sr Passed string address where the sample rate is written to. <br>
 * Helper function for converting a selected menu index for the sample rate to <br>
 * an integer and a String.
 */
    void audioPrefsSRHelper(int i, qint32 &sr_int, QString &sr);

/**
 * @brief Menu text of the "follow the system default" device entry.
 * @param defaultDeviceIndex Pa_GetDefaultOutputDevice() or Pa_GetDefaultInputDevice().
 * Names the device the system default currently resolves to, so the menu
 * shows what following it means right now.
 */
    QString defaultDeviceMenuText(int defaultDeviceIndex);

/**
 * @brief Stores the selected audio devices in the settings.
 * Stored by name, not by index, as PortAudio's device indices <br>
 * are meaningless in the next session. An empty value means: <br>
 * follow the system default. Read back in loadLayoutAndSettings().
 */
    void saveAudioDeviceSettings();

/** ********** Create RWA directory in Applications Support/RWA and a filelist of RWA games as .txt file ************ */

/**
 * @brief Creates RWA directory in Applications Support and a filelist of RWA games as .txt file  <br>
 * Creates a directory RWACreator in Library/Application Support/ <br>
 * Originally it was used to save Applications settings which is now done with save/loadLayoutAndSettings. <br>
 * It creates a list allgames.txt in Library/Application Support/RWACreator/Games of all exported .zip <br>
 * RWA games. This currently not really in use anymore, as it is much easier to simply copy the games  <br>
 * from a computer to a connected iphone via Finder.
 */

    void createInitFolder();

/** *************************************** Save and load main window layout ***************************************** */

/**
 * @brief Saves Window/View Layout and application settings. <br>
 * Saves Window/View settings such as postion, size <br>
 * docked/floating Window and globals application settings <br>
 * such as last opened game, sample rate etc. <br>
 */

    void saveLayoutAndSettings();

/**
 * @brief Loads Window/View Layout and application settings. <br>
 * Loads Window/View settings such as postion, size <br>
 * docked/floating Window and globals application settings <br>
 * such as last opened game, sample rate etc. <br>
 */

    void loadLayoutAndSettings();

/**
 * @brief Check if undo folder is empty or not. <br>
 * Currently the function does nothing. <br>
 * Will be used to restore the last game state <br>
 * if RWA crashes.
 */

    void checkUndoFolder();

/** ************************************************* RWA Views ***************************************************** */

private slots:

/**
 * @brief Loads default views on application start.<br>
 * Loads all RWA views as docking windows at default positions on application start: <br>
 * MapView <br>
 * SceneView <br>
 * GameView <br>
 * StateView <br>
 * History (Undo) View <br>
 * LogView
 */

    void loadDefaultViews();

/**
 * @brief Makes closed views visible again. <br>
 * Makes cloesed views visible again at their last known position <br>
 * and size.
 */

    void gatherViews();
    void restoreFloatingViews();

/**
 * @brief Creates a Map View. <br>
 * Creates and adds a Map View to the Main Window.
 */

    void addMapView();

/**
 * @brief Creates a Scene View. <br>
 * Creates and adds a Scene View to the Main Window.
 */

    void addSceneView();

/**
 * @brief Creates a Game View. <br>
 * Creates and adds a Game View to the Main Window.
 */

    void addGameView();

/**
 * @brief Creates a State View. <br>
 * Creates and adds a State View to the Main Window.
 */

    void addStateView();

/**
 * @brief Creates a History (Undo) View. <br>
 * Creates and adds a History (Undo) View to the Main Window.
 */

    void addHistoryView();

/**
 * @brief Creates a Log View. <br>
 * Creates and adds a Log View to the Main Window.
 */

    void addLogView();

/** ************************************** Write, export and open functionality ******************************************** */

/**
 * @brief Exports (copies) a whole project. <br>
 * Copies the whole project with a new name/path <br>
 * to a new location including all folders <br>
 * (undo, tilecache, assets)
 */

    void exportProject();

/**
 * @brief Asks for a project location and writes the whole project there.
 * @param dialogTitle Title of the file dialogue.
 * The implementation behind exportProject(), new() and the first save()
 * of an unsaved game - they only differ in what the file dialogue is
 * called, so that its title says what the user is actually doing.
 */

    void exportProjectAs(const QString &dialogTitle);

/**
 * @brief Exports (copies) a project for a mobile client. <br>
 * Copies the whole project with a new name/path <br>
 * to a new location without copying undo and
 * tilecache folder for a mobile client. <br>
 */

    void exportForTransferToPlayer();

/**
 * @brief Exports (copies) a project for a mobile client as *.zip-file. <br>
 * Copies the whole project with a new name/path <br>
 * to a new location without copying undo and
 * tilecache folder for a mobile client as *.zip-file. <br>
 */

    void exportZip();

/**
 * @brief Saves the current state of a RWA game.<br>
 * Saves the current state with the current name <br>
 * and current project path of an RWA game to disc.
 */

    void save();

/**
 * @brief Saves the current state of a RWA game with a new name.<br>
 * Saves the current state of an RWA game to disc with a new name <br>
 * Project and Assets folder paths are not changed.
 */

    void saveAs();

/**
 * @brief Opens an RWA game. <br>
 * @param fileName The name of the game to be opened <br>
 * @param throwDialogue Forces to throw an open dialogue if fileName is empty or NULL <br>
 * Opens an RWA game with the specified fileName. If the game can not be found <br>
 * and throwDialogue is set to true, the function throws an open file dialogue.
 */

    qint32 open(QString fileName = QString(""), bool throwDialogue = true);

/**
 * @brief Starts a new game.
 * Clears the current game and asks for the location and
 * name of the new project.
 */

    void newProject();

/**
 * @brief Starts the simulation, restarts it if it is already running. Cmd-R
 * means "run the game from the top": on a running simulation it stops and
 * starts it again.
 */

    void runSimulation();

/**
 * @brief Stops the simulation.
 * Does nothing if the simulation is not running.
 */

    void stopSimulation();

/**
 * @brief Keeps the Simulation menu on the state of the simulation.
 * @param running Whether the simulation is running now.
 * A key command can not show what it is going to do, so the menu has to:
 * "Run Simulation" becomes "Restart Simulation" while the simulation runs,
 * and "Stop Simulation" is greyed out while it does not.
 */

    void updateSimulationMenu(bool running);

/**
 * @brief Empties the Log View.
 * Clears the log window which is currently receiving the messages,
 * i.e. the one added last.
 */

    void clearLogWindow();

/** **************************************** Main application menu SLOTS ****************************************** */

/**
 * @brief Re-enumerates the audio devices and rebuilds the audio prefs menu.
 * PortAudio only knows the devices which were present when it was
 * initialized, so devices connected or removed while the application
 * is running are invisible and the selected device index goes stale
 * (which silently kills the audio output). Called from the audio
 * preferences menu and before the simulation is started. Stops a
 * running simulation.
 */

    void rescanAudioDevices();

/**
 * @brief Rebuilds the audio prefs menu after the device list has changed.
 * Connected to RwaSimulator::sendAudioDevicesChanged().
 */

    void receiveAudioDevicesChanged();

/**
 * @brief Follows the system default output device again. <br>
 * Gives up a device explicitly picked from the menu, so that every <br>
 * following rescan takes whatever macOS currently defaults to - which <br>
 * is the connected headset, if there is one.
 */

    void selectSystemDefaultOutputDevice();

/**
 * @brief Follows the system default input device again. <br>
 * The input counterpart of selectSystemDefaultOutputDevice().
 */

    void selectSystemDefaultInputDevice();

/**
 * @brief Selects the audio output device. <br>
 * @param index Passed menu index.
 * Selects the audio output device according <br>
 * to the chosen menu index.
 */

    void selectOutputDevice(qint32 index);

/**
 * @brief Selects the audio input device. <br>
 * @param index Passed menu index.
 * Selects the audio input device according <br>
 * to the chosen menu index.
 */

    void selectInputDevice(qint32 index);

/**
 * @brief Set the project sample rate. <br>
 * @param index Passed menu index.
 * Sets the project sample rate according to the <br>
 * chosen menu index. This sample rate setting does <br>
 * not affect the audio device sample rate. This must still be <br>
 * done in the Audio/MIDI settings.
 */

    void selectSampleRate(qint32 index);

/**
 * @brief Throws a dialogue for entering the head tracker name. <br>
 * The function is called from the head tracker menu and throws <br>
 * a dialogue for entering the head tracker name.
 */

    void enterHtName();

/**
 * @brief Throws a dialogue for entering file path preferences. <br>
 * The function is called from the file menu and throws <br>
 * a dialogue for entering file paths for game export.
 */

    void enterFilePathPreferences();

/**
 * @brief Shows the About dialogue. <br>
 * Shows app name, version, git commit hash and links to <br>
 * the GitHub repository and the FHNW research project page. <br>
 * On macOS the action is placed in the application menu ("RWA Creator").
 */

    void about();

/**
 * @brief Deletes unused assets. <br>
 * The function deletes unused assets. <br>
 * Currently the ThrashAssets option from the <br>
 * toolbar does not take into account sound files <br>
 * which are used more than once. So rather don't use the ThrashAssets<br>
 * option, instead call this function from the menu from time to <br>
 * time.
 */

    void deleteUnusedAssetFiles();

/** ******************************* Save app settings and clean up before quit**************************************** */

/**
 * @brief Callback when application is closed. <br>
 * @param event The QCloseEvent.
 * The function gets called when the RWACreator <br>
 * is closed (on quit). Currently it does nothing.
 */

    void closeEvent(QCloseEvent *event);

/**
 * @brief Cleans up before quitting. <br>
 * The function is called when the user is <br>
 * closing the RWACreator. It stops the simulation <br>
 * deletes tmp directories and throws a save dialogue. <br>
 */

    void cleanUpBeforeQuit();

public slots:

    /**
    * @brief Opens an RWA game requested from outside the application.
    * @param path Full path of the .rwa file to open.
    * Entry point for document-open requests from the OS (macOS Finder
    * double-click / Dock drop via RwaApplication) and from the command
    * line on Windows/Linux. Replaces the currently loaded project,
    * same as File > Open.
    */
    void openProject(const QString &path);

/** *********************************************** Undo functionality *********************************************** */

/**
 * @brief Reads an undo file. <br>
 * @param name The name of the undo file to read.
 * The function gets called when the user selects <br>
 * an undo file from the history view and reads the <br>
 * corresponding game state into memory.
 */

    void readUndoFile(QString name);

/**
 * @brief Writes an undo file. <br>
 * @param undoAction The name of the undo action. <br>
 * The function gets called on every user interaction that affects <br>
 * the current state of the game and writes a corresponding <br>
 * undo file.
 */

    void writeUndo(QString undoAction);

/** *************************** logMessages redirects qDebug() to rwalogview ***************************************** */

    static void logMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg);

signals:
    void sendReadNewGame();
};

#endif
