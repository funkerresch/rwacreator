/*
 *
 * Created by Thomas Resch
 * ©2022 by Thomas Resch
 *
 * rwacreator is the main window of the RwaCreator Application
 *
 */

#include "rwacreator.h"
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QApplication>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QTimer>
#include <qdebug.h>
#include <unistd.h>
#include "rwainputdialog.h"

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

// Set via target_compile_definitions in CMakeLists.txt
#ifndef RWA_VERSION
#define RWA_VERSION "unknown"
#endif
#ifndef RWA_GIT_COMMIT_HASH
#define RWA_GIT_COMMIT_HASH "unknown"
#endif

RwaLogWindow *RwaCreator::logWindow;

RwaCreator::RwaCreator(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("RWA Creator");
    setDockNestingEnabled(true);
    menuBar()->setObjectName("RWA Creator");
    statusBar()->hide();
    logWindow = nullptr;

    backend = RwaBackend::getInstance();
    backend->setReadOnly(true);
    backend->setMinimumSize(10, 10);
    backend->setObjectName("RWA Backend");
    allViewsLoaded = false;
    headtracker = RwaHeadtrackerConnect::getInstance();

    QObject::connect(this, SIGNAL(sendReadNewGame()), backend, SLOT(receiveReadNewGame()));
    QObject::connect(backend, SIGNAL(sendWriteUndo(QString)), this, SLOT(writeUndo(QString)));
    QObject::connect(backend, SIGNAL(readUndoFile(QString )), this, SLOT(readUndoFile(QString )));
    QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()),this, SLOT(cleanUpBeforeQuit()));
    QObject::connect(backend->simulator, SIGNAL(sendAudioDevicesChanged()), this, SLOT(receiveAudioDevicesChanged()));
    this->installEventFilter(this);

    setCentralWidget(backend);
    createInitFolder();
    loadDefaultViews();
    loadLayoutAndSettings();
    QTimer::singleShot(0, this, &RwaCreator::restoreFloatingViews);
    setupMenuBar(); // the key commands live on the menu actions, see initFileMenu()
}

/** *************************** logMessages redirects qDebug() to rwalogview ***************************************** */

void RwaCreator::logMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(logWindow)
        logWindow->outputMessage( type, context, msg );
}

/** ********* Create RWA directory in ~/Library/Applications Support and filelist of RWA games as .txt file ********** */

void RwaCreator::createInitFolder()
{
    QString path = backend->applicationSupportPath;
    if(!QDir(path).exists())
        QDir().mkdir(path);

    QString filename = path + "/" +"createfilelist.sh";
    {
        QFile file(filename);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            stream << "#!/bin/sh" << "\n";
            stream << "cd Games/" << "\n";
            stream << "ls *.zip > allfiles.txt" << "\n";
        }
        file.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser | QFileDevice::ReadOther);
        file.close();
    }

    path = QString(backend->completeSharingServerPath);
    if(!QDir(path).exists())
        QDir().mkdir(path);
}

/** ******************************** Save and load main window layout and settings ********************************** */

void RwaCreator::saveLayoutAndSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("lastgame", backend->completeFilePath);
    settings.setValue("xcodeclientprojectpath", backend->completeTransferToPlayerExportPath);
    settings.setValue("downloadpath", backend->completeSharingServerPath);
    settings.setValue("downloadpathwithescape", backend->completeSharingServerPathWithEscape);
    settings.setValue("headtrackerid", headtracker->getName());
    settings.setValue("samplerate", backend->sampleRate);
    settings.sync(); // forces to write the settings to storage
}

void RwaCreator::loadLayoutAndSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    if(settings.contains("headtrackerid"))
        headtracker->setName(settings.value("headtrackerid").toString());
    else
        headtracker->setName("rwaht00");

    if(settings.contains("samplerate"))
        backend->setSampleRate(settings.value("samplerate").toInt());
    else
        backend->setSampleRate(48000);

    // Absent or empty means: follow the system default. A stored device which is not connected
    // right now stays remembered and is taken as soon as it shows up in a rescan.
    backend->simulator->ap->setOutputDeviceByName(settings.value("audiooutputdevice").toString());
    backend->simulator->ap->setInputDeviceByName(settings.value("audioinputdevice").toString());

    if(settings.contains("xcodeclientprojectpath"))
        backend->completeTransferToPlayerExportPath = (settings.value("xcodeclientprojectpath").toString());
    else
        backend->completeTransferToPlayerExportPath = QString("%1%2").arg(QDir::homePath()).arg("/Desktop");

    if(settings.contains("downloadpath"))
        backend->completeSharingServerPath = (settings.value("downloadpath").toString());
    else
        backend->completeSharingServerPath = QString("%1%2").arg(QDir::homePath()).arg("/Library/Application Support/RWACreator/Games");

    if(settings.contains("downloadpathwithescape"))
        backend->completeSharingServerPathWithEscape = (settings.value("downloadpathwithescape").toString());
    else
        backend->completeSharingServerPathWithEscape = QString("%1%2").arg(QDir::homePath()).arg("'/Library/Application Support/RWACreator/Games\'");

    if(!open(settings.value("lastgame").toString(), false))
    {
        qWarning("Last game does not exist.");
        newProject();
    }
}

/** ********************************************* Add default views ************************************************** */

void RwaCreator::loadDefaultViews()
{
    addMapView();
    addStateView();
    addSceneView();
    addLogView();
    addGameView();
    addHistoryView();
    allViewsLoaded = true;
}

void RwaCreator::addMapView() // qt bug: stylesheet is applied only if widget is docked:(
{
    mapView = new RwaMapView(this, backend->getFirstScene(), "MapView");
    RwaDockWidget *dw = new RwaDockWidget(this, "Map View");
    dw->setObjectName(tr("Map View"));
    dw->setWindowTitle(tr("Map View"));
    dw->setGeometry(0,0,1000,300);
    dw->setWidget(mapView);
    dw->installEventFilter(this);
    dw->setWindowFlags(Qt::WindowStaysOnTopHint );
    addDockWidget(Qt::TopDockWidgetArea, dw);
    rwaDockWidgets.append(dw);
}

void RwaCreator::addLogView()
{
    logWindow = new RwaLogWindow(this);
    RwaDockWidget *dw = new RwaDockWidget(this, "Log View");
    dw->setObjectName(tr("Log View"));
    dw->setWindowTitle(tr("Log View"));
    dw->setGeometry(0,0,600,300);
    dw->setWidget(logWindow);
    dw->setWindowFlags(Qt::WindowStaysOnTopHint );
    addDockWidget(Qt::TopDockWidgetArea, dw);
    rwaDockWidgets.append(dw);
}

void RwaCreator::addGameView()
{
    RwaDockWidget *dw = new RwaDockWidget(this, "Game View");
    dw->setObjectName(tr("Game View"));
    dw->setWindowTitle(tr("Game View"));
    dw->setGeometry(0,0,600,300);
    dw->setWidget(new RwaGameView(this, backend->getFirstScene(), "GameView"));
    dw->installEventFilter(this);
    addDockWidget(Qt::LeftDockWidgetArea, dw);
    rwaDockWidgets.append(dw);
}

void RwaCreator::addSceneView()
{
    RwaDockWidget *dw = new RwaDockWidget(this, "Scene View");
    dw->setObjectName(tr("Scene View"));
    dw->setWindowTitle(tr("Scene View"));
    dw->setGeometry(0,0,600,300);
    dw->setWidget(new RwaSceneView(this, backend->getFirstScene(), "SceneView"));
    dw->installEventFilter(this);
    addDockWidget(Qt::LeftDockWidgetArea, dw);
    rwaDockWidgets.append(dw);
}

void RwaCreator::addStateView()
{
    RwaDockWidget *dw = new RwaDockWidget(this, "State View");
    dw->setObjectName(tr("State View"));
    dw->setWindowTitle(tr("State View"));
    dw->setGeometry(0,0,600,300);
    dw->setWidget(new RwaStateView(this, backend->getFirstScene(), "StateView"));
    dw->installEventFilter(this);
    addDockWidget(Qt::BottomDockWidgetArea, dw);
    rwaDockWidgets.append(dw);
}

void RwaCreator::addHistoryView()
{
    RwaDockWidget *dw = new RwaDockWidget(this, "History View");
    dw->setObjectName(tr("History View"));
    dw->setWindowTitle(tr("History View"));
    dw->setGeometry(0,0,600,300);
    dw->setWidget(new RwaHistory(this));
    addDockWidget(Qt::RightDockWidgetArea, dw);
    rwaDockWidgets.append(dw);
}

/** ******************************* Save app settings and clean up before quit**************************************** */

void RwaCreator::closeEvent(QCloseEvent *event)
{
    (void) event;
    maybeSave();
    qDebug();
}

void RwaCreator::cleanUpBeforeQuit()
{
    qDebug() << "Clean up and quit!";
    backend->simulator->stopRwaSimulation();
    saveLayoutAndSettings();
    emptyTmpDirectories();
    backend->clearScenes();
}

/** *************************** Currently maybeSave is always called on quitting RWA ********************************* */

bool RwaCreator::maybeSave()
{
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        save();
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

/** ***************************** Event filter calls resize events to the graphic views ******************************** */

bool RwaCreator::eventFilter(QObject *obj, QEvent *event)
{
    QDockWidget *myWidget;
    RwaGraphicsView *myView;

    if (event->type() == QEvent::Resize)
    {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
        myWidget = static_cast<QDockWidget *>(obj);

        if(obj->objectName() == "Map View" || obj->objectName() == "State View") // We need this for resizing the mapcontroller
        {                                                                        // when the parent window is resized
            myView =  static_cast<RwaGraphicsView *>(myWidget->widget());
            myView->adaptSize(resizeEvent->size().width(),resizeEvent->size().height());
        }
    }

    QMainWindow::changeEvent(event);

    return QWidget::eventFilter(obj, event);
}

/** *********************************** Select Input/Output Device and Sample Rate ************************************** */

void RwaCreator::selectOutputDevice(qint32 index)
{
    if(backend->simulator->isSimulationRunning())
        backend->simulator->stopRwaSimulation();

    backend->simulator->ap->setOutputDevice(index);
    saveAudioDeviceSettings();
}

void RwaCreator::selectInputDevice(qint32 index)
{
    if(backend->simulator->isSimulationRunning())
        backend->simulator->stopRwaSimulation();

    backend->simulator->ap->setInputDevice(index);
    saveAudioDeviceSettings();
}

/**
  Devices are stored by name, not by index: PortAudio hands out the indices in the order it happens
  to enumerate the hardware, so they mean nothing in the next session. An empty value is the
  "follow the system default" state.
*/
void RwaCreator::saveAudioDeviceSettings()
{
    QSettings settings;
    settings.setValue("audiooutputdevice", backend->simulator->ap->getExplicitOutputDeviceName());
    settings.setValue("audioinputdevice", backend->simulator->ap->getExplicitInputDeviceName());
    settings.sync(); // forces to write the settings to storage
}

void RwaCreator::selectSampleRate(qint32 index)
{
    QSettings settings;

    if(index == 0)
        backend->setSampleRate(44100);
    if(index == 1)
        backend->setSampleRate(48000);

    qInfo() << backend->sampleRate;

    settings.setValue("samplerate", backend->sampleRate);
    settings.sync(); // forces to write the settings to storage
}

void RwaCreator::audioPrefsSRHelper(int i, qint32 &sr_int, QString &sr)
{
    if(i == RWA_SR_44100)
    {
        sr = QString("44100");
        sr_int = 44100;
    }

    if(i == RWA_SR_48000)
    {
        sr = QString("48000");
        sr_int = 48000;
    }
}

/** ******************************************* Init Audio Preferences Menu ********************************************* */

void RwaCreator::initAudioPreferencesMenu(QMenu *audioDeviceMenu)
{
    if(!audioDeviceMenu)
        return;

    audioPreferencesMenu = audioDeviceMenu;

    // The menu is rebuilt on every rescan. QMenu::clear() drops the actions but not the action
    // groups, so those are recreated here too, otherwise every rescan would leak a set of them.
    audioDeviceMenu->clear();
    delete selectSampleRateGroup;
    delete selectAudioOutputDeviceGroup;
    delete selectAudioInputDeviceGroup;

    selectSampleRateGroup = new QActionGroup(audioDeviceMenu);
    selectAudioOutputDeviceGroup = new QActionGroup(audioDeviceMenu);
    selectAudioInputDeviceGroup = new QActionGroup(audioDeviceMenu);
    selectSampleRateGroup->setExclusive(true);
    selectAudioOutputDeviceGroup->setExclusive(true);
    selectAudioInputDeviceGroup->setExclusive(true);

    QAction *rescanDevicesAction = new QAction(audioDeviceMenu);
    rescanDevicesAction->setText(tr("Rescan Audio Devices"));
    connect(rescanDevicesAction, SIGNAL(triggered()), this, SLOT(rescanAudioDevices()));
    audioDeviceMenu->addAction(rescanDevicesAction);
    audioDeviceMenu->addSeparator();

    QAction *sampleRateLabel = new QAction(audioDeviceMenu);
    sampleRateLabel->setCheckable(false);
    sampleRateLabel->setText(QString("Target Sample Rate"));
    sampleRateLabel->setEnabled(false);
    audioDeviceMenu->addAction(sampleRateLabel );
    audioDeviceMenu->addSeparator();

    QString sr;
    qint32 sr_int;

    for(int i = 0; i < 2; i++)
    {
        audioPrefsSRHelper(i, sr_int, sr);
        selectSampleRateAction = new QAction(audioDeviceMenu);
        selectSampleRateAction->setCheckable(true);
        selectSampleRateAction->setText(sr);
        connect(selectSampleRateAction, &QAction::triggered, this, [this, i]{ selectSampleRate(i); });
        audioDeviceMenu->addAction(selectSampleRateAction );
        selectSampleRateGroup->addAction(selectSampleRateAction);
        if(backend->sampleRate == sr_int)
            selectSampleRateAction->setChecked(true);
    }

    audioDeviceMenu->addSeparator();

    QAction *outputDeviceLabel = new QAction(audioDeviceMenu);
    outputDeviceLabel->setCheckable(false);
    outputDeviceLabel->setText(QString("Select Output-Device"));
    outputDeviceLabel->setEnabled(false);
    audioDeviceMenu->addAction(outputDeviceLabel );
    audioDeviceMenu->addSeparator();

    // System default option: Without this entry a single click on a device would opt the user
    // out of following the system default for the rest of the session.
    QAction *systemDefaultOutputAction = new QAction(audioDeviceMenu);
    systemDefaultOutputAction->setCheckable(true);
    systemDefaultOutputAction->setText(defaultDeviceMenuText(Pa_GetDefaultOutputDevice()));
    connect(systemDefaultOutputAction, &QAction::triggered, this, [this]{ selectSystemDefaultOutputDevice(); });
    audioDeviceMenu->addAction(systemDefaultOutputAction);
    selectAudioOutputDeviceGroup->addAction(systemDefaultOutputAction);
    if(!backend->simulator->ap->hasExplicitOutputDevice())
        systemDefaultOutputAction->setChecked(true);

    const PaDeviceInfo* di;
    for(int i = 0;i < Pa_GetDeviceCount();i++)
    {
        if(backend->simulator->ap->isOutputDevice(i))
        {
            selectAudioDeviceAction= new QAction(audioDeviceMenu);
            selectAudioDeviceAction->setCheckable(true);
            di = Pa_GetDeviceInfo(i);
            selectAudioDeviceAction->setText(QString(di->name));
            connect(selectAudioDeviceAction, &QAction::triggered, this, [this, i]{ selectOutputDevice(i); });
            audioDeviceMenu->addAction(selectAudioDeviceAction );
            selectAudioOutputDeviceGroup->addAction(selectAudioDeviceAction);
            if(backend->simulator->ap->hasExplicitOutputDevice()
               && backend->simulator->ap->getOutputDevice() == i)
                selectAudioDeviceAction->setChecked(true);
        }
    }

    audioDeviceMenu->addSeparator();
    QAction *inputDeviceLabel = new QAction(audioDeviceMenu);
    inputDeviceLabel->setCheckable(false);
    inputDeviceLabel->setText(QString("Select Input-Device"));
    inputDeviceLabel->setEnabled(false);
    audioDeviceMenu->addAction(inputDeviceLabel );
    audioDeviceMenu->addSeparator();

    QAction *systemDefaultInputAction = new QAction(audioDeviceMenu);
    systemDefaultInputAction->setCheckable(true);
    systemDefaultInputAction->setText(defaultDeviceMenuText(Pa_GetDefaultInputDevice()));
    connect(systemDefaultInputAction, &QAction::triggered, this, [this]{ selectSystemDefaultInputDevice(); });
    audioDeviceMenu->addAction(systemDefaultInputAction);
    selectAudioInputDeviceGroup->addAction(systemDefaultInputAction);
    if(!backend->simulator->ap->hasExplicitInputDevice())
        systemDefaultInputAction->setChecked(true);

    for(int i = 0;i < Pa_GetDeviceCount();i++)
    {
        if(backend->simulator->ap->isInputDevice(i))
        {
            selectAudioDeviceAction= new QAction(audioDeviceMenu);
            selectAudioDeviceAction->setCheckable(true);
            di = Pa_GetDeviceInfo(i);
            selectAudioDeviceAction->setText(QString(di->name));
            connect(selectAudioDeviceAction, &QAction::triggered, this, [this, i]{ selectInputDevice(i); });
            audioDeviceMenu->addAction(selectAudioDeviceAction );
            selectAudioInputDeviceGroup->addAction(selectAudioDeviceAction);
            if(backend->simulator->ap->hasExplicitInputDevice()
               && backend->simulator->ap->getInputDevice() == i)
                selectAudioDeviceAction->setChecked(true);
        }
    }
}

/** Menu text of the "follow the system default" entry, naming the device it currently resolves to. */
QString RwaCreator::defaultDeviceMenuText(int defaultDeviceIndex)
{
    const PaDeviceInfo* di = defaultDeviceIndex >= 0 ? Pa_GetDeviceInfo(defaultDeviceIndex) : nullptr;
    if(!di)
        return tr("Follow System Default");

    return tr("Follow System Default (%1)").arg(QString(di->name));
}

/** ****************************************** Rescan the audio devices ************************************************ */

/**
  PortAudio only sees the devices that existed when it was initialized, so a headset connected
  while the app is running neither shows up in the menu nor produces sound - the stored device
  index has gone stale. Re-enumerating requires a PortAudio restart, therefore the simulation has
  to be stopped first.
*/
void RwaCreator::rescanAudioDevices()
{
    backend->simulator->rescanAudioDevices();
}

void RwaCreator::selectSystemDefaultOutputDevice()
{
    if(backend->simulator->isSimulationRunning())
        backend->simulator->stopRwaSimulation();

    backend->simulator->ap->useSystemDefaultOutputDevice();
    saveAudioDeviceSettings();
    qInfo() << "Output device follows the system default:"
            << backend->simulator->ap->getDeviceName(backend->simulator->ap->getOutputDevice());
}

void RwaCreator::selectSystemDefaultInputDevice()
{
    if(backend->simulator->isSimulationRunning())
        backend->simulator->stopRwaSimulation();

    backend->simulator->ap->useSystemDefaultInputDevice();
    saveAudioDeviceSettings();
    qInfo() << "Input device follows the system default:"
            << backend->simulator->ap->getDeviceName(backend->simulator->ap->getInputDevice());
}

/** Rebuilds the audio preferences menu after the device list has changed. */
void RwaCreator::receiveAudioDevicesChanged()
{
    // Rebuilding deletes the menu actions, among them the "Rescan Audio Devices" action whose
    // triggered() signal usually gets us here. Deleting it while that signal is still being
    // delivered would pull the ground from under QMenu, so the rebuild is deferred by one event
    // loop cycle.
    QTimer::singleShot(0, this, [this]{ initAudioPreferencesMenu(audioPreferencesMenu); });
}

/** ******************************** Set visiblity of closed RWA Views to true again ********************************** */

void RwaCreator::gatherViews()
{
    foreach(RwaDockWidget *widget, rwaDockWidgets)
    {
        if(!widget->isVisible())
            widget->setVisible(true);
    }
}

/**
 * Restore floating (detached) views hidden by Qt after restoreState() on startup
 */
void RwaCreator::restoreFloatingViews()
{
    foreach(RwaDockWidget *widget, rwaDockWidgets)
    {
        if(widget->isFloating() && !widget->isVisible())
            widget->setVisible(true);
    }
}

/** ********************************************** Init RWA View Menu ************************************************* */

void RwaCreator::initViewMenu1(QMenu *fileMenu)
{
    QAction *action = fileMenu->addAction(tr("Gather Views"));
    connect(action, SIGNAL(triggered()), this, SLOT(gatherViews()));

    action = fileMenu->addAction(tr("Map View"));
    connect(action, SIGNAL(triggered()), this, SLOT(addMapView()));

    action = fileMenu->addAction(tr("Game View"));
    connect(action, SIGNAL(triggered()), this, SLOT(addGameView()));

    action = fileMenu->addAction(tr("Scene View"));
    connect(action, SIGNAL(triggered()), this, SLOT(addSceneView()));

    action = fileMenu->addAction(tr("State View"));
    connect(action, SIGNAL(triggered()), this, SLOT(addStateView()));

    action = fileMenu->addAction(tr("History View"));
    connect(action, SIGNAL(triggered()), this, SLOT(addHistoryView()));

    action = fileMenu->addAction(tr("Log Window"));
    connect(action, SIGNAL(triggered()), this, SLOT(addLogView()));

    action = fileMenu->addAction(tr("Clear Log Window"));
    action->setShortcut(QKeySequence(tr("Ctrl+Shift+L", "Clear Log Window")));
    connect(action, SIGNAL(triggered()), this, SLOT(clearLogWindow()));
}

/** ********************************************* Init Simulation Menu ************************************************ */

void RwaCreator::initSimulationMenu(QMenu *simulationMenu)
{
    runSimulationAction = simulationMenu->addAction(tr("Run Simulation"));
    runSimulationAction->setShortcut(QKeySequence(tr("Ctrl+R", "Run Simulation")));
    connect(runSimulationAction, SIGNAL(triggered()), this, SLOT(runSimulation()));

    stopSimulationAction = simulationMenu->addAction(tr("Stop Simulation"));
    stopSimulationAction->setShortcut(QKeySequence(tr("Ctrl+K", "Stop Simulation")));
    connect(stopSimulationAction, SIGNAL(triggered()), this, SLOT(stopSimulation()));

    // The simulator announces every start and stop, whoever caused it - the toolbar
    // button, this menu or an audio device rescan stopping the simulation.
    connect(backend->simulator, SIGNAL(sendSimulationRunningChanged(bool)),
            this, SLOT(updateSimulationMenu(bool)));
    updateSimulationMenu(backend->isSimulationRunning());
}

/** ************************************************ Init RWA File Menu *********************************************** */

void RwaCreator::initFileMenu(QMenu *fileMenu)
{
    // AboutRole moves the action into the application menu ("RWA Creator") on macOS
    QAction *action = fileMenu->addAction(tr("About RWA Creator"));
    action->setMenuRole(QAction::AboutRole);
    connect(action, SIGNAL(triggered()), this, SLOT(about()));

    action = fileMenu->addAction(tr("File Path Preferences"));
    connect(action, SIGNAL(triggered()), this, SLOT(enterFilePathPreferences()));

    action = fileMenu->addAction(tr("Remove unused files from disk"));
    connect(action, SIGNAL(triggered()), this, SLOT(deleteUnusedAssetFiles()));

    action = fileMenu->addAction(tr("New"));
    action->setShortcut(QKeySequence::New);
    connect(action, SIGNAL(triggered()), this, SLOT(newProject()));

    action = fileMenu->addAction(tr("Open"));
    action->setShortcut(QKeySequence::Open);
    connect(action, SIGNAL(triggered()), this, SLOT(open()));

    action = fileMenu->addAction(tr("Save"));
    action->setShortcut(QKeySequence::Save);
    connect(action, SIGNAL(triggered()), this, SLOT(save()));

    action = fileMenu->addAction(tr("Save Version as..."));
    action->setShortcut(QKeySequence::SaveAs);
    connect(action, SIGNAL(triggered()), this, SLOT(saveAs()));

    action = fileMenu->addAction(tr("Copy Project to..."));
    action->setShortcut(QKeySequence(tr("Ctrl+Alt+S", "Copy Project to")));
    connect(action, SIGNAL(triggered()), this, SLOT(exportProject()));

    action = fileMenu->addAction(tr("Export Project for transfer to RWA Player..."));
    action->setShortcut(QKeySequence(tr("Ctrl+E", "Export Project for transfer to RWA Player")));
    connect(action, SIGNAL(triggered()), this, SLOT(exportForTransferToPlayer()));

    action = fileMenu->addAction(tr("Send Project to Sharing Server..."));
    action->setShortcut(QKeySequence(tr("Ctrl+Shift+E", "Send Project to Sharing Server")));
    connect(action, SIGNAL(triggered()), this, SLOT(exportZip()));
}

/** ******************************************** File path preferences pop-up ****************************************** */

void RwaCreator::enterFilePathPreferences()
{
    QStringList labels;
    QStringList values;
    labels << "Sharing Server Path" << "Project Export Path";
    values << backend->completeSharingServerPath << backend->completeTransferToPlayerExportPath;
    QStringList list = RwaInputDialog::getStrings(this, labels, values, tr("File Path Preferences"));
    if (!list.isEmpty()) {
        backend->completeSharingServerPath = list[0];
        backend->completeSharingServerPathWithEscape = "'"+backend->completeSharingServerPath+"'";
        backend->completeTransferToPlayerExportPath = list[1];
    }
}

/** ************************************************ About pop-up ***************************************************** */

void RwaCreator::about()
{
    QString text = tr(
        "<h3 align='center'>RWA Creator</h3>"
        "<p align='center'>Version %1 (%2)</p>"
        "<p align='center'>An open-source middleware for creating interactive soundwalks.</p>"
        "<p align='center'>Author: Thomas Resch (and contributors)</p>"
        "<p align='center'><a href=\"https://github.com/rnd-hsm-klassik/rwa-creator\">GitHub repository</a><br>"
        "<a href=\"https://www.fhnw.ch/de/musik/forschung-dienstleistungen/forschung/projekte/real-world-audio-2\">"
        "Real World Audio</a><br/>Forschung Institut Klassik<br/>Hochschule für Musik Basel FHNW</p>")
        .arg(QStringLiteral(RWA_VERSION), QStringLiteral(RWA_GIT_COMMIT_HASH));

    QDialog aboutDialog(this);
    aboutDialog.setWindowTitle(tr("About RWA Creator"));

    QVBoxLayout *layout = new QVBoxLayout(&aboutDialog);
    layout->setContentsMargins(60, 24, 60, 24);
    layout->setSpacing(2);

    // Qt can't decode the .icns app icon, use a png version
    QPixmap appIcon(backend->completeBundlePath + "images/rwa-creator.png");
    if(!appIcon.isNull())
    {
        qreal dpr = aboutDialog.devicePixelRatio();
        QPixmap scaledIcon = appIcon.scaled(QSize(96, 96) * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        scaledIcon.setDevicePixelRatio(dpr);
        QLabel *iconLabel = new QLabel(&aboutDialog);
        iconLabel->setPixmap(scaledIcon);
        iconLabel->setAlignment(Qt::AlignHCenter);
        layout->addWidget(iconLabel);
    }

    QLabel *textLabel = new QLabel(text, &aboutDialog);
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setAlignment(Qt::AlignHCenter);
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    textLabel->setOpenExternalLinks(true);
    layout->addWidget(textLabel);

    aboutDialog.exec();
}

/** ******************************************** Head tracker name pop-up ********************************************* */

void RwaCreator::enterHtName()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Set Headtracker Name"),
                                             tr("Tracker name:"), QLineEdit::Normal,
                                             headtracker->getName(), &ok);

    if (ok && !text.isEmpty())
    {
        headtracker->setName(text);
        QString menuString = QString("Headtracker Name (%1)").arg(text);
        QAction *enterHtName = headtrackerMenu->actions().at(0);
        enterHtName->setText(menuString);
    }
}

/** ******************************************** Init head tracker menu *********************************************** */

void RwaCreator::initHeadtrackerMenu(QMenu *headtrackerMenu)
{
    if(headtrackerMenu)
        headtrackerMenu->clear();

    QString menuString = QString("Headtracker Name (%1)").arg(headtracker->getName());

    QAction *enterHtName = headtrackerMenu->addAction(menuString);
    connect(enterHtName, SIGNAL(triggered()), this, SLOT(enterHtName()));

    QAction *btaction = headtrackerMenu->addAction(tr("Connect via Bluetooth"));
    connect(btaction, SIGNAL(triggered()), headtracker, SLOT(startBluetoothScanning()));

    QAction *btactionDisconnect = headtrackerMenu->addAction(tr("Disconnect from Bluetooth"));
    connect(btactionDisconnect, SIGNAL(triggered()), headtracker, SLOT(disconnectHeadtracker()));
}

/** *********************************************** Setup menu bar *************************************************** */

void RwaCreator::setupMenuBar()
{
    QMenu *selectAudioDevice = menuBar()->addMenu(tr("&Audio Preferences"));
    initAudioPreferencesMenu(selectAudioDevice);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    initViewMenu1(viewMenu);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    initFileMenu(fileMenu);

    QMenu *simulationMenu = menuBar()->addMenu(tr("&Simulation"));
    initSimulationMenu(simulationMenu);

    headtrackerMenu = menuBar()->addMenu(tr("&Headtracker"));
    initHeadtrackerMenu(headtrackerMenu);
}

/** ************************************ Writing and export functionality ******************************************** */

void RwaCreator::write1(QString writeMessage, qint32 flags, QString newCompleteFilePath)
{
    QString path = RwaUtilities::getPath(newCompleteFilePath);
    QFile file(newCompleteFilePath);
    if(file.exists())
        file.remove();

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(newCompleteFilePath)
                             .arg(file.errorString()));
        return;
    }

    RwaExport writer(this, backend->completeProjectPath, path, flags);
    if (writer.writeFile(&file))
    {
        if(!writeMessage.isEmpty())
            statusBar()->showMessage(writeMessage, 2000);
    }
}

void RwaCreator::prepareWrite1(QString fullpath, int flags)  // fullpath is /RWACreator/Games/test/test.rwa
{
    QString fileName = RwaUtilities::getFileName(fullpath);  // test.rwa
    QString path = RwaUtilities::getPath(fullpath);          // RWACreator/Games/test
    QStringList fullName = fileName.split(".rwa");           // test
    QString folderName = fullName.first();
    QString completeFilePath = QString("%1/%2/%3").arg(path).arg(folderName).arg(fileName);
    QString completeProjectPath = QString("%1/%2").arg(path).arg(folderName);
    QDir dir(path);
    dir.mkdir(folderName);
    QString completeAssetPath = QString("%1/%2/assets").arg(path).arg(folderName);
    dir.mkdir(completeAssetPath);

    if(flags & RWAEXPORT_CREATEFOLDERS)
    {
        QString completeUndoPath = QString("%1/%2/undo").arg(path).arg(folderName);
        QString completeTmpPath = QString("%1/%2/tmp").arg(path).arg(folderName);
        dir.mkdir(completeUndoPath);
        dir.mkdir(completeTmpPath);
        QStringList pieces = fileName.split( "." );
        backend->projectName = pieces.first();
        backend->completeTmpPath = completeTmpPath;
        backend->completeUndoPath = completeUndoPath;
        backend->completeAssetPath = completeAssetPath;
    }
}

void RwaCreator::exportForTransferToPlayer()
{
    qint32 flags = 0;
    flags |= RWAEXPORT_COPYASSETS
          | RWAEXPORT_EXPORTFORMOBILECLIENT;

    QString directory = backend->completeTransferToPlayerExportPath;
    if(!QDir(directory).exists())
        QDir().mkdir(directory);

    QString fileName = RwaUtilities::getFileName(backend->completeFilePath);            // for example test.rwa
    QString baseName = RwaUtilities::getFileBaseName(backend->completeFilePath);        // test                                          // /RWACreator/Games/test/test.rwa
    QString fullDirectory = directory +"/"+baseName;
    QString fullpath = fullDirectory +"/"+fileName;  // /RWACreator/Games/test

    QDir dir = QDir(fullDirectory);
    if(dir.exists())
        dir.removeRecursively();

    prepareWrite1(fullDirectory, flags);
    write1("Export zip for RWA Sharing Server", flags, fullpath);

    setWindowTitle(backend->projectName + " Successfully exported for transfer to RWA Player");
    QTimer::singleShot(2000, [this]{setWindowTitle(backend->projectName);});
}

void RwaCreator::exportZip()
{
    qint32 flags = 0;
    flags |= RWAEXPORT_COPYASSETS
          | RWAEXPORT_EXPORTFORMOBILECLIENT
          | RWAEXPORT_ZIP;

    QString directory = backend->completeSharingServerPath;
    if(!QDir(directory).exists())
        QDir().mkdir(directory);

    QString fileName = RwaUtilities::getFileName(backend->completeFilePath);            // for example test.rwa
    QString baseName = RwaUtilities::getFileBaseName(backend->completeFilePath);        // test                                          // /RWACreator/Games/test/test.rwa
    QString fullDirectory = directory +"/"+baseName;
    QString fullpath = fullDirectory +"/"+fileName;  // /RWACreator/Games/test

    QDir dir = QDir(fullDirectory);
    if(dir.exists()) // removing old version
        dir.removeRecursively();

    prepareWrite1(fullDirectory, flags);
    write1("Export zip for RWA Server", flags, fullpath);

    QFile zipFile(fullDirectory+".zip");
    if(zipFile.exists())
        zipFile.remove();

    QString zipGame = QString("cd %1 && zip -r -X %2.zip %3").arg(backend->completeSharingServerPathWithEscape).arg(baseName).arg(baseName);
    FILE* pipe = popen(zipGame.toStdString().c_str(), "w");
    if (!pipe)
    {
       qCritical() << "Could not zip";
       return;
    }

    while(pclose(pipe) != -1)
        ;

    dir.removeRecursively(); // removing new version
    QString initFolder = backend->applicationSupportPathWithEscape;
    QString createList = QString("cd %1 && ./createfilelist.sh").arg(initFolder);
    pipe = popen(createList.toStdString().c_str(), "w");
    while(pclose(pipe) != -1)
        ;

    setWindowTitle(backend->projectName + " Successfully sent ZIP to Sharing Server");
    QTimer::singleShot(2000, [this]{setWindowTitle(backend->projectName);});
}

void RwaCreator::exportProject()
{
    exportProjectAs(tr("Copy entire RWA Project Folder"));
}

/**
  Writing a complete project folder is the same operation for "Copy Project to...",
  for a new game and for the first save of a game which has never been written to disk.
  Only the title of the file dialogue differs, so that it names what the user asked for.
*/
void RwaCreator::exportProjectAs(const QString &dialogTitle)
{
    qint32 flags = 0;
    flags |= RWAEXPORT_COPYASSETS
          | RWAEXPORT_SAVEAS
          | RWAEXPORT_CREATEFOLDERS;

    QString fullpath = QFileDialog::getSaveFileName(this, dialogTitle,
                                         QDir::homePath(),
                                         tr("RWA Files (*.rwa *.xml)"));

    if(fullpath.isEmpty())
        return;

    QString fileName = RwaUtilities::getFileName(fullpath);            // for example test.rwa
    QString directory = RwaUtilities::getFileBaseName(fileName);        // test
    QString incompletePath = RwaUtilities::getPath(fullpath);
    QString fullDirectory = incompletePath +"/"+directory;
    fullpath = fullDirectory +"/"+fileName;  // /RWACreator/Games/test

    if(!QDir(fullDirectory).exists())
        QDir().mkdir(fullDirectory);

    prepareWrite1(fullDirectory, flags);
    write1("Saved full project to new folder", flags, fullpath);
    QString path = RwaUtilities::getPath(fullpath);
    backend->completeProjectPath = fullDirectory;
    backend->completeFilePath = fullpath;
    backend->projectName = directory;
    setWindowTitle(backend->projectName);
    undoCounter = 0;
    emit sendReadNewGame();
    writeUndo("Init Game");
}

void RwaCreator::saveAs()
{
    qint32 flags = 0;
    flags |= RWAEXPORT_SAVEAS;

    QString fullpath = QFileDialog::getSaveFileName(this, tr("Save Version of RWA Project File"),
                                         backend->completeProjectPath,
                                         tr("RWA Files (*.rwa *.xml)"));

    if(fullpath.isEmpty())
        return;

    backend->completeFilePath = fullpath;
    write1("File saved", flags, fullpath);
    QString projectName = RwaUtilities::getFileName(fullpath);
    QStringList pieces = projectName.split( "." );
    projectName = pieces.first();
    backend->projectName = projectName;
    setWindowTitle(backend->projectName);
}

void RwaCreator::save()
{
    if(backend->completeFilePath.isEmpty())
        exportProjectAs(tr("Save RWA Project"));
    else
      write1("File saved", 0, backend->completeFilePath);

    setWindowTitle(backend->projectName + " Successfully saved");
    QTimer::singleShot(2000, [this]{setWindowTitle(backend->projectName);});
}

void RwaCreator::checkUndoFolder()
{


}

/** ************************************************ Open Rwa game **************************************************** */

qint32 RwaCreator::open(QString fileName, bool throwDialogue)
{
    QString fullpath;
    QString projectName;
    QString assetPath;

    if(!fileName.isEmpty())
        fullpath = fileName;
    else
    {
        if(throwDialogue)
            fullpath = QFileDialog::getOpenFileName(this, tr("Open RWA File"), QDir::currentPath(),tr("RWA Files (*.rwa *.xml)"));
    }

    if (fullpath.isEmpty() || fullpath.isNull())
        return 0;

    QFile file(fullpath);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        statusBar()->showMessage(tr("Could not open file"), 2000);
        qCritical() << "Could not open File";
        return 0;
    }

    // here we could check the undo folder. If it has not been emptied, the application probably crashed,
    // and we can restore the last saved state from undo

    emptyTmpDirectories();
    backend->clearScenes();
    projectName = RwaUtilities::getFileName(fullpath);
    QStringList pieces = projectName.split( "." );
    projectName = pieces.first();

    backend->projectName = projectName;
    backend->completeFilePath = fullpath;
    backend->completeProjectPath = RwaUtilities::getPath(fullpath);

    QString completeUndoPath = QString("%1/undo").arg(backend->completeProjectPath);
    QString completeAssetPath = QString("%1/assets").arg(backend->completeProjectPath);
    QString completeTmpPath = QString("%1/tmp").arg(backend->completeProjectPath);

    backend->completeUndoPath = (completeUndoPath);
    backend->completeAssetPath = (completeAssetPath);
    backend->completeTmpPath = (completeTmpPath);
    setWindowTitle(backend->projectName);

    RwaImport reader(this, &backend->getScenes(), backend->completeProjectPath);

    if (!reader.read(&file))
         statusBar()->showMessage(tr("Could not read XML, parser error!"), 2000);
    else
    {
        statusBar()->showMessage(tr("File loaded"), 2000);
        undoCounter = 0;
        emit sendReadNewGame();
        writeUndo("Init Game");
    }

    return 1;
}

void RwaCreator::emptyTmpDirectories()
{
    if (backend->completeUndoPath.isEmpty() || backend->completeTmpPath.isEmpty())
        return;
    RwaUtilities::emtpyDirectory(backend->completeUndoPath);
    RwaUtilities::emtpyDirectory(backend->completeTmpPath);
}

void RwaCreator::newProject()
{
    qDebug();
    undoCounter = 0;
    emptyTmpDirectories();
    backend->reset();
    setWindowTitle("Not saved");
    exportProjectAs(tr("New RWA Project"));
}

/** ******************************************* Simulation and log view ************************************************ */

void RwaCreator::runSimulation()
{
    if(backend->isSimulationRunning())
        backend->startStopSimulator(false);

    backend->startStopSimulator(true);
}

void RwaCreator::stopSimulation()
{
    if(!backend->isSimulationRunning())
        return;

    backend->startStopSimulator(false);
}

void RwaCreator::updateSimulationMenu(bool running)
{
    if(!runSimulationAction || !stopSimulationAction)
        return;

    runSimulationAction->setText(running ? tr("Restart Simulation") : tr("Run Simulation"));
    stopSimulationAction->setEnabled(running);
}

void RwaCreator::clearLogWindow()
{
    if(logWindow)
        logWindow->clearLog();
}

/** *********************************************** Undo functionality *********************************************** */

void RwaCreator::writeUndo(QString undoAction)
{
    if(backend->completeUndoPath.isEmpty())
    {
        qDebug();
        return;
    }

    QString fullUndoFilepath;
    fullUndoFilepath = QString("%1/%3_%2.rwa").arg(backend->completeUndoPath).arg(undoAction).arg(undoCounter++);

    QFile file(fullUndoFilepath);
    if(file.exists())
        file.remove();

    if (!file.open(QFile::WriteOnly | QFile::Text))
         return;

    RwaExport writer(this, QString(), QString(), 0);
    writer.writeFile(&file);
}

void RwaCreator::readUndoFile(QString name)
{
    backend->clearScenes();
    QFile file(backend->completeUndoPath +"/"+name);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(name)
                             .arg(file.errorString()));
        return;
    }

    RwaImport reader(this, &backend->getScenes(), backend->completeProjectPath);
    if (!reader.read(&file))
    {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Parse error in file %1:\n\n%2")
                             .arg(name)
                             .arg(reader.errorString()));
    }

    else
    {
        statusBar()->showMessage(tr("File loaded"), 2000);
    }
}

void RwaCreator::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

void RwaCreator::deleteUnusedAssetFiles()
{
    bool isInUse = false;
    QDir dir(backend->completeAssetPath);
    for (const QFileInfo &file : dir.entryInfoList(QDir::Files))
    {
        isInUse = false;
        foreach(RwaScene *scene, backend->getScenes())
        {
            foreach(RwaState *state, scene->getStates())
            {
                foreach(RwaAsset1 *asset, state->getAssets())
                {
                    if(asset->getFileName() == file.fileName().toStdString())
                    {
                        isInUse = true;
                        break;
                    }
                }
            }
        }

        if(!isInUse)
        {
            QString completeFilePath(file.absoluteFilePath());
            QFile::remove(completeFilePath);
            qInfo() << "Removed "<< completeFilePath << " from Disk.";
        }
    }
}
