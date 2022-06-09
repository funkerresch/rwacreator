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
#include <qdebug.h>

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

RwaLogWindow *RwaCreator::logWindow;

RwaCreator::RwaCreator(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
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

    setCentralWidget(backend);   
    createInitFolder();      
    loadDefaultViews();
    setupMenuBar();
    loadLayoutAndSettings();
}

/** *************************** logMessages redirects qDebug() to rwalogview ***************************************** */

void RwaCreator::logMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(logWindow)
        logWindow->outputMessage( type, context, msg );
}

/** ************* Create RWA directory in home director and filelist of RWA games as .txt file *********************** */

void RwaCreator::createInitFolder()
{
    QString path = QString("%1%2").arg(QDir::homePath()).arg("/RWACreator/");
    if(!QDir(path).exists())
        QDir().mkdir(path);

    QString filename = path +"createfilelist.sh";
    {
        QFile file(filename);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            stream << "#!/bin/sh" << endl;
            stream << "cd $PWD/Games/" << endl;
            stream << "ls *.zip > allfiles.txt" << endl;
        }
        file.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser | QFileDevice::ReadOther);
        file.close();
   }

    path = QString("%1%2").arg(QDir::homePath()).arg("/RWACreator/Games");
    if(!QDir(path).exists())
        QDir().mkdir(path);
}

/** ******************************* Open and write rwainit.txt for global app settings ********************************** */

void RwaCreator::openInit()
{
    int lineNumber = 0;
    QString filename = QString("%1%2%3").arg(QDir::homePath()).arg("/RWACreator/").arg("rwainit.txt");
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly))
    {
        clear();
        return;
    }

    QTextStream in(&file);

    while(!in.atEnd())
    {
        QString line = in.readLine();

        if(lineNumber == 0)
        {
            if(!open(line))
                clear();
        }

        if(lineNumber == 1)
            headtracker->setName(line);

        lineNumber++;
    }

    file.close();
 }

void RwaCreator::writeInit()
{
    QString filename = QString("%1%2%3").arg(QDir::homePath()).arg("/RWACreator/").arg("rwainit.txt");
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << backend->completeFilePath << endl;
        stream << headtracker->getName() << endl;
        if(backend->showStateRadii)
            stream << "1" << endl;
        else
            stream << "0" << endl;
        if(backend->showAssets)
            stream << "1" << endl;
        else
            stream << "0";
        file.close();
    }
    else
        qDebug() << "Error writing init";
}

/** ******************************** Save and load main window layout and settings ********************************** */

void RwaCreator::saveLayoutAndSettings()
{
    QSettings settings("Intrinsic Audio", "Rwa Creator");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("lastgame", backend->completeFilePath);
    settings.setValue("headtrackerid", headtracker->getName());
    settings.sync(); // forces to write the settings to storage
}

void RwaCreator::loadLayoutAndSettings()
{
    QSettings settings("Intrinsic Audio", "Rwa Creator");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    headtracker->setName(settings.value("headtrackerid").toString());
    if(!open(settings.value("lastgame").toString()))
        clear();
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
    RwaDockWidget *dw = new RwaDockWidget(this);
    mapView = new RwaMapView(this, backend->getFirstScene(), "MapView");
    //dw->setStyleSheet("QDockWidget { background-color:lightgrey; font-size: 20px; color: black}");
    //dw->setStyleSheet("QDockWidget { font: bold }");
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
    RwaDockWidget *dw = new RwaDockWidget(this);
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
    RwaDockWidget *dw = new RwaDockWidget(this);
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
    RwaDockWidget *dw = new RwaDockWidget(this);
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
    RwaDockWidget *dw = new RwaDockWidget(this);
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
    RwaDockWidget *dw = new RwaDockWidget(this);
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
    qDebug();
}

void RwaCreator::cleanUpBeforeQuit()
{
    qDebug() << "Clean up and quit!";
    backend->simulator->stopRwaSimulation();
    maybeSave();
    saveLayoutAndSettings();
    emptyTmpDirectories();
    backend->clearScenes();
}

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

/** ***************************** Event filter calls resize events to the graphic views ********************************** */

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

    return QWidget::eventFilter(obj, event);
}

void RwaCreator::selectOutputDevice(qint32 index)
{
    if(backend->simulator->isSimulationRunning())
        backend->simulator->stopRwaSimulation();

    backend->simulator->ap->setOutputDevice(index);
}

void RwaCreator::selectInputDevice(qint32 index)
{
    if(backend->simulator->isSimulationRunning())
        backend->simulator->stopRwaSimulation();

    backend->simulator->ap->setInputDevice(index);
}

void RwaCreator::initAudioPreferencesMenu(QMenu *audioDeviceMenu)
{
    QActionGroup *selectAudioOutputDeviceGroup = new QActionGroup(audioDeviceMenu);
    QActionGroup *selectAudioInputDeviceGroup = new QActionGroup(audioDeviceMenu);
    QSignalMapper* outputDeviceSignalMapper = new QSignalMapper(audioDeviceMenu);
    QSignalMapper* inputDeviceSignalMapper = new QSignalMapper(audioDeviceMenu);

    QAction *outputDeviceLabel = new QAction(audioDeviceMenu);
    outputDeviceLabel->setCheckable(false);
    outputDeviceLabel->setText(QString("Select Output-Device"));
    outputDeviceLabel->setEnabled(false);
    audioDeviceMenu->addAction(outputDeviceLabel );
    audioDeviceMenu->addSeparator();

    const PaDeviceInfo* di;
    for(int i = 0;i < Pa_GetDeviceCount();i++)
    {
        if(backend->simulator->ap->isOutputDevice(i))
        {
            selectAudioDeviceAction= new QAction(audioDeviceMenu);
            selectAudioDeviceAction->setCheckable(true);
            di = Pa_GetDeviceInfo(i);
            selectAudioDeviceAction->setText(QString(di->name));
            connect (selectAudioDeviceAction, SIGNAL(triggered()), outputDeviceSignalMapper, SLOT(map())) ;
            outputDeviceSignalMapper->setMapping (selectAudioDeviceAction, i) ;
            audioDeviceMenu->addAction(selectAudioDeviceAction );
            selectAudioOutputDeviceGroup->addAction(selectAudioDeviceAction);
            if(backend->simulator->ap->getOutputDevice() == i)
                selectAudioDeviceAction->setChecked(true);
        }
    }
    selectAudioOutputDeviceGroup->setExclusive(true);
    connect (outputDeviceSignalMapper, SIGNAL(mapped(int)), this, SLOT(selectOutputDevice(qint32))) ;

    audioDeviceMenu->addSeparator();
    QAction *inputDeviceLabel = new QAction(audioDeviceMenu);
    inputDeviceLabel->setCheckable(false);
    inputDeviceLabel->setText(QString("Select Input-Device"));
    inputDeviceLabel->setEnabled(false);
    audioDeviceMenu->addAction(inputDeviceLabel );
    audioDeviceMenu->addSeparator();

    for(int i = 0;i < Pa_GetDeviceCount();i++)
    {
        if(backend->simulator->ap->isInputDevice(i))
        {
            selectAudioDeviceAction= new QAction(audioDeviceMenu);
            selectAudioDeviceAction->setCheckable(true);
            di = Pa_GetDeviceInfo(i);
            selectAudioDeviceAction->setText(QString(di->name));
            connect (selectAudioDeviceAction, SIGNAL(triggered()), inputDeviceSignalMapper, SLOT(map())) ;
            inputDeviceSignalMapper->setMapping (selectAudioDeviceAction, i) ;
            audioDeviceMenu->addAction(selectAudioDeviceAction );
            selectAudioInputDeviceGroup->addAction(selectAudioDeviceAction);
            if(backend->simulator->ap->getInputDevice() == i)
                selectAudioDeviceAction->setChecked(true);
        }
    }
    selectAudioInputDeviceGroup->setExclusive(true);
    connect (inputDeviceSignalMapper, SIGNAL(mapped(int)), this, SLOT(selectInputDevice(qint32))) ;
}

void RwaCreator::initViewMenu1(QMenu *fileMenu)
{
    QAction *action = fileMenu->addAction(tr("Default Views"));
    connect(action, SIGNAL(triggered()), this, SLOT(loadDefaultViews()));

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
}

void RwaCreator::initFileMenu(QMenu *fileMenu)
{
    QAction *action = fileMenu->addAction(tr("Clear"));
    connect(action, SIGNAL(triggered()), this, SLOT(clear()));

    action = fileMenu->addAction(tr("Open"));
    connect(action, SIGNAL(triggered()), this, SLOT(open()));

    action = fileMenu->addAction(tr("Save"));
    connect(action, SIGNAL(triggered()), this, SLOT(save()));

    action = fileMenu->addAction(tr("Save as"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveAs()));

    action = fileMenu->addAction(tr("Export Project"));
    connect(action, SIGNAL(triggered()), this, SLOT(exportProject()));

    action = fileMenu->addAction(tr("Export for Client"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveForMobileClient()));

    action = fileMenu->addAction(tr("Export again for Client"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveAgainForMobileClient()));
}

void RwaCreator::enterHtName()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
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

void RwaCreator::setupMenuBar()
{
    QMenu *selectAudioDevice = menuBar()->addMenu(tr("&Select Audio Device"));
    initAudioPreferencesMenu(selectAudioDevice);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    initViewMenu1(viewMenu);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    initFileMenu(fileMenu);

    headtrackerMenu = menuBar()->addMenu(tr("&Headtracker"));
    initHeadtrackerMenu(headtrackerMenu);
}

void RwaCreator::write(QString writeMessage, qint32 flags, QString oldAssetPath)
{
    QString completeFilePath;
    QString completeProjectPath;

    completeFilePath = backend->completeFilePath;
    completeProjectPath = backend->completeProjectPath;

    if (completeFilePath.isEmpty())
        return;

    QFile file(completeFilePath);
    if(file.exists())
        file.remove();

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(completeFilePath)
                             .arg(file.errorString()));
        return;
    }

    RwaExport writer(backend, oldAssetPath, flags);
    if (writer.writeFile(&file))
    {
        if(!writeMessage.isEmpty())
            statusBar()->showMessage(writeMessage, 2000);
    }
}

void RwaCreator::prepareWrite(QString fullpath, int flags)
{
    QString fileName = RwaUtilities::getFileName(fullpath);
    QString path = RwaUtilities::getPath(fullpath);
    QStringList fullName = fileName.split(".rwa");
    QString folderName = fullName.first();
    QString completeFilePath = QString("%1/%2/%3").arg(path).arg(folderName).arg(fileName);
    QString completeProjectPath = QString("%1/%2").arg(path).arg(folderName);

    QDir dir(path);
    dir.mkdir(folderName);
    QString completeAssetPath = QString("%1/%2/assets").arg(path).arg(folderName);
    dir.mkdir(completeAssetPath);

    if(flags & RWAEXPORT_SAVEAS)
    {
        QString completeUndoPath = QString("%1/%2/undo").arg(path).arg(folderName);
        QString completeTmpPath = QString("%1/%2/tmp").arg(path).arg(folderName);
        dir.mkdir(completeUndoPath);
        dir.mkdir(completeTmpPath);
        QStringList pieces = fileName.split( "." );
        backend->projectName = pieces.first();
        backend->completeTmpPath = completeTmpPath;
        backend->completeUndoPath = completeUndoPath;
    }

    backend->completeAssetPath = completeAssetPath;
    backend->completeFilePath = completeFilePath;
    backend->completeProjectPath = completeProjectPath;
}

void RwaCreator::saveAgainForMobileClient()
{
    if(backend->completeClientExportPath.isEmpty())
    {
        saveForMobileClient();
        return;
    }

    qint32 flags = 0;
    flags |= RWAEXPORT_COPYASSETS
           | RWAEXPORT_EXPORTFORMOBILECLIENT;

    QString oldFilePath = backend->completeFilePath;
    QString oldProjectPath = backend->completeProjectPath;
    QString oldAssetPath = backend->completeAssetPath;
    QString fullpath = backend->completeClientExportPath;
    prepareWrite(fullpath, flags);
    write("Exported for mobile Client", flags, oldAssetPath);
    backend->completeClientExportPath = fullpath;
    backend->completeAssetPath = oldAssetPath;
    backend->completeFilePath = oldFilePath;
    backend->completeProjectPath = oldProjectPath;
}

void RwaCreator::saveForMobileClient()
{
    qint32 flags = 0;
    flags |= RWAEXPORT_COPYASSETS
           | RWAEXPORT_EXPORTFORMOBILECLIENT;

    QString oldFilePath = backend->completeFilePath;
    QString oldProjectPath = backend->completeProjectPath;
    QString oldAssetPath = backend->completeAssetPath;

    QString fullpath = QFileDialog::getSaveFileName(this, tr("Save Rwa File"),
                                         QDir::currentPath(),
                                         tr("RWA Files (*.rwa *.xml)"));

    if(fullpath.isEmpty())
        return;

    qDebug();
    prepareWrite(fullpath, flags);
    write("Exported for mobile Client", flags, oldAssetPath);
    backend->completeClientExportPath = fullpath;
    backend->completeAssetPath = oldAssetPath;
    backend->completeFilePath = oldFilePath;
    backend->completeProjectPath = oldProjectPath;
}

void RwaCreator::exportProject()
{
    qint32 flags = 0;
    flags |= RWAEXPORT_COPYASSETS
           | RWAEXPORT_SAVEAS;

    QString fullpath = QFileDialog::getSaveFileName(this, tr("Save Rwa File"),
                                         QDir::homePath(),
                                         tr("RWA Files (*.rwa *.xml)"));
    if(fullpath.isEmpty())
        return;

    QString oldAssetPath = backend->completeAssetPath;
    prepareWrite(fullpath, flags);
    write("File saved", flags, oldAssetPath);
    setWindowTitle(backend->projectName);
    writeUndo("Init Game");
}

void RwaCreator::saveAs()
{
    qint32 flags = 0;
    flags |= RWAEXPORT_SAVEAS;

    QString fullpath = QFileDialog::getSaveFileName(this, tr("Save Rwa File"),
                                         backend->completeProjectPath,
                                         tr("RWA Files (*.rwa *.xml)"));

    if(fullpath.isEmpty())
        return;

    backend->completeFilePath = fullpath;
    write("File saved", flags, fullpath);
    QString projectName = RwaUtilities::getFileName(fullpath);
    QStringList pieces = projectName.split( "." );
    projectName = pieces.first();
    backend->projectName = projectName;
    setWindowTitle(backend->projectName);
}

void RwaCreator::save()
{
    if(backend->completeFilePath.isEmpty())
        exportProject();
    else
      write("File saved", 0, "");
}

qint32 RwaCreator::open(QString fileName)
{
    QString fullpath;
    QString projectName;
    QString assetPath;

    if(!fileName.isEmpty())
        fullpath = fileName;
    else
        fullpath = QFileDialog::getOpenFileName(this, tr("Open Bookmark File"), QDir::currentPath(),tr("RWA Files (*.rwa *.xml)"));

    if (fullpath.isEmpty())
        clear();

    QFile file(fullpath);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        statusBar()->showMessage(tr("Could not open file"), 2000);
        return 0;
    }

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

    RwaImport reader(&backend->getScenes(), backend->completeProjectPath);

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
    RwaUtilities::emtpyDirectory(backend->completeUndoPath);
    RwaUtilities::emtpyDirectory(backend->completeTmpPath);
}

void RwaCreator::clear()
{
    qDebug();
    undoCounter = 0;
    emptyTmpDirectories();      
    backend->reset();
    setWindowTitle("Not saved");
    exportProject();
}

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

    RwaExport writer(backend, backend->completeUndoPath,0);
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

    RwaImport reader(&backend->getScenes(), backend->completeProjectPath);
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


