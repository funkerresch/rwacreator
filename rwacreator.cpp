/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

    QObject::connect(backend, SIGNAL(sendSave()), this, SLOT(save()));
    QObject::connect(backend, SIGNAL(sendWriteUndo(QString)), this, SLOT(writeUndo(QString)));
    QObject::connect(backend, SIGNAL(readUndoFile(QString )), this, SLOT(readUndoFile(QString )));
    QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit()),this, SLOT(cleanUpBeforeQuit()));

    setCentralWidget(backend);   
    createInitFolder();
    loadDefaultViews();
    openInit();
    setupMenuBar();   
    //StartHttpServer(8088);

//    int blubs = system("python3 -m http.server 8088 --directory /Users/harveykeitel/Desktop & echo $!");
//    qDebug() << "Started python server " << blubs;
}

//RwaCreator::~RwaCreator()
//{
//    backend->deleteLater();
//}


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

void RwaCreator::cleanUpBeforeQuit()
{
    qDebug() << "Clean up and quit!";
    writeInit();
    saveLayout();
    //save();
    backend->emptyTmpDirectories();
    backend->getScenes().clear();
    //system("kill -9");
}

bool RwaCreator::eventFilter(QObject *obj, QEvent *event)
{
    QDockWidget *myWidget;
    RwaView *myView;

    if (event->type() == QEvent::Resize)
    {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
        myWidget = ((QDockWidget *)obj);
        myView = (RwaView *)myWidget->widget();
        myView->adaptSize(resizeEvent->size().width(),resizeEvent->size().height());
    }
    else if (event->type() == QEvent::Close)
    {
        qDebug("Mainwindow: yet to implement: destroyDockWidget()");
      //yet to do
    }
    else if(event->type() == QEvent::Close)
    {
        qDebug("Mainwindow: Quit");
       // backend->clear();
    }

    return QWidget::eventFilter(obj, event);
}

void RwaCreator::mousePressEvent(QMouseEvent *event)
{
    qDebug("Mainwindow::mousePressEvent: '%d' ", event->type());
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

void RwaCreator::initViewMenu(QMenu *viewMenu)
{
    createDockWidgetAction = new QAction(tr("Add View"), this);
    connect(createDockWidgetAction, SIGNAL(triggered()), this, SLOT(createDockWidget()));
    viewMenu->addAction(createDockWidgetAction);

    defaultViewWidgetAction = new QAction(tr("Default Views"), this);
    connect(defaultViewWidgetAction, SIGNAL(triggered()), this, SLOT(loadDefaultViews()));
    viewMenu->addAction(defaultViewWidgetAction);
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

void RwaCreator::initEditMenu(QMenu *fileMenu)
{
//    QAction *action = fileMenu->addAction(tr("Undo"));
//    connect(action, SIGNAL(triggered()), this, SLOT(undo()));

//    action = fileMenu->addAction(tr("Redo"));
//    connect(action, SIGNAL(triggered()), this, SLOT(redo()));
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

void RwaCreator::scanSerialPorts()
{
    initHeadtrackerMenu(headtrackerMenu);
}

void RwaCreator::setupMenuBar()
{
    QMenu *selectAudioDevice = menuBar()->addMenu(tr("&Select Audio Device"));
    initAudioPreferencesMenu(selectAudioDevice);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    initViewMenu(viewMenu);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    initFileMenu(fileMenu);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    initEditMenu(editMenu);

    headtrackerMenu = menuBar()->addMenu(tr("&Headtracker"));
    initHeadtrackerMenu(headtrackerMenu);
}

void RwaCreator::saveLayout()
{
    QString fileName = QString("%1/layouts.ini").arg(backend->completeProjectPath);
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QString msg = tr("Failed to open %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    QByteArray geo_data = saveGeometry();
    QByteArray layout_data = saveState();  

    bool ok = file.putChar((uchar)geo_data.size());
    if (ok)
        ok = file.write(geo_data) == geo_data.size();
    if (ok)
        ok = file.write(layout_data) == layout_data.size();

    if (!ok) {
        QString msg = tr("Error writing to %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

void RwaCreator::loadLayout()
{
    QString fileName = QString("%1/layouts.ini").arg(backend->completeProjectPath);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QString msg = tr("Failed to open %1\n%2")
                        .arg(fileName)
                        .arg(file.errorString());
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    uchar geo_size;
    QByteArray geo_data;
    QByteArray layout_data;

    bool ok = file.getChar((char*)&geo_size);
    if (ok) {
        geo_data = file.read(geo_size);
        ok = geo_data.size() == geo_size;
    }
    if (ok) {
        layout_data = file.readAll();
        ok = layout_data.size() > 0;
    }

    if (ok)
        ok = restoreGeometry(geo_data);
    if (ok)
        ok = restoreState(layout_data);

    if (!ok) {
        QString msg = tr("Error reading %1")
                        .arg(fileName);
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }
}

void RwaCreator::writeInit()
{
    QString filename = QString("%1%2%3").arg(QDir::homePath()).arg("/RWACreator/").arg("rwainit.txt");
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << backend->completeFilePath << endl;
        stream << headtracker->getName();
        file.close();
    }
    else
        qDebug() << "Error writing init";
}

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
            open (line);

        if(lineNumber == 1)
            headtracker->setName(line);

        lineNumber++;
    }

    file.close();
 }

void RwaCreator::write(QString writeMessage, qint32 flags, QString oldAssetPath)
{
    QString completeFilePath = backend->completeFilePath;
    QString completeProjectPath = backend->completeProjectPath;

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

    if(!(flags & RWAEXPORT_EXPORTFORMOBILECLIENT))
    {
        writeInit();
        saveLayout();
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
    backend->newGameLoaded();
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

void RwaCreator::open(QString fileName)
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
        return;
    }

    backend->clear();
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
        emit backend->newGameLoaded();
        writeUndo("Init Game");
    }

    QString layoutsFullPath = QString("%1/layouts.ini").arg(backend->completeProjectPath);
    QFile layouts(layoutsFullPath);

    if(layouts.exists())
    {
         loadLayout();
         foreach(QDockWidget *widget, rwaDockWidgets)
         {
             if(widget->isFloating())
                 widget->show();
         }
    }
}

void RwaCreator::clear()
{
    qDebug() << "Clear";
    undoCounter = 0;
    backend->projectName = QString();
    backend->completeFilePath = QString();
    backend->completeProjectPath = QString();
    backend->clearData();
    setWindowTitle("Not saved");
    exportProject();
}

void RwaCreator::newProject()
{
    QString fullpath = QFileDialog::getSaveFileName(this, tr("New Rwa Project"), QDir::homePath(), tr("RWA Files (*.rwa *.xml)"));
    if(fullpath.isEmpty())
    {
        open(QString("defaultproject/default.rwa"));
        return;
    }
    prepareWrite(fullpath);
    write("Successfully created new Rwa Project", 0, "");
    writeUndo("Init Game");
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
        emit backend->undoGameLoaded();
    }
}

QAction *addAction(QMenu *menu, const QString &text, QActionGroup *group, QSignalMapper *mapper,
                    int id)
{
    bool first = group->actions().isEmpty();
    QAction *result = menu->addAction(text);
    result->setCheckable(true);
    result->setChecked(first);
    group->addAction(result);
    QObject::connect(result, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(result, id);
    return result;
}

void RwaCreator::setCorner(int id)
{
    switch (id) {
        case 0:
            QMainWindow::setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
            break;
        case 1:
            QMainWindow::setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
            break;
        case 2:
            QMainWindow::setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
            break;
        case 3:
            QMainWindow::setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
            break;
        case 4:
            QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
            break;
        case 5:
            QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
            break;
        case 6:
            QMainWindow::setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
            break;
        case 7:
            QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
            break;
    }
}

void RwaCreator::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

class CreateDockWidgetDialog : public QDialog
{
public:
    CreateDockWidgetDialog(QWidget *parent = 0);

    //QString objectName() const;
    Qt::DockWidgetArea location() const;
    int type() const;

private:
    //QLineEdit *m_objectName;

    QComboBox *m_type;
    QComboBox *m_location;
};

CreateDockWidgetDialog::CreateDockWidgetDialog(QWidget *parent)
    : QDialog(parent)
{
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("View:")), 0, 0);
    m_type = new QComboBox;
    m_type->setEditable(false);
    m_type->addItem(tr("Script Text View"));
    m_type->addItem(tr("Script GUI View"));
    m_type->addItem(tr("Asset View"));
    m_type->addItem(tr("Map View"));
    layout->addWidget(m_type, 0, 1);

    layout->addWidget(new QLabel(tr("Location:")), 1, 0);
    m_location = new QComboBox;
    m_location->setEditable(false);
    m_location->addItem(tr("Top"));
    m_location->addItem(tr("Left"));
    m_location->addItem(tr("Right"));
    m_location->addItem(tr("Bottom"));
    layout->addWidget(m_location, 1, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    layout->addLayout(buttonLayout, 2, 0, 1, 2);
    buttonLayout->addStretch();

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancelButton);
    QPushButton *okButton = new QPushButton(tr("Ok"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(okButton);

    okButton->setDefault(true);
}

Qt::DockWidgetArea CreateDockWidgetDialog::location() const
{
    switch (m_location->currentIndex()) {
        case 0: return Qt::TopDockWidgetArea;
        case 1: return Qt::LeftDockWidgetArea;
        case 2: return Qt::RightDockWidgetArea;
        case 3: return Qt::BottomDockWidgetArea;
        default:
            break;
    }
    return Qt::NoDockWidgetArea;
}

int CreateDockWidgetDialog::type() const
{
    switch (m_type->currentIndex()) {
        case 0: return AFX_WINDOWTYPE_SCRIPTTEXT;
        case 1: return AFX_WINDOWTYPE_SCRIPTGUI;
        case 2: return AFX_WINDOWTYPE_ASSET;
        case 3: return AFX_WINDOWTYPE_MAPVIEW;
        default:
            break;
    }
    return 0;
}

void RwaCreator::createDockWidget()
{
    CreateDockWidgetDialog dialog(this);
    int ret = dialog.exec();
    if (ret == QDialog::Rejected)
        return;

    Qt::DockWidgetArea area = dialog.location();
    int viewType = dialog.type();
    RwaStateView *listWidget;


    QDockWidget *dw = new QDockWidget;

    switch (viewType)
    {
        case AFX_WINDOWTYPE_SCRIPTTEXT:
        dw->setObjectName(tr("State Text Editor"));
        dw->setWindowTitle(tr("State Text Editor"));
        dw->setWidget(new QTextEdit);
        break;

        case AFX_WINDOWTYPE_ASSET:
        dw->setObjectName(tr("Asset View"));
        dw->setWindowTitle(tr("Asset View"));
        listWidget = new RwaStateView(this);

        dw->setWidget(listWidget);
        break;

        case AFX_WINDOWTYPE_MAPVIEW:
        dw->setObjectName(tr("State Map View"));
        dw->setWindowTitle(tr("State Map View"));
        dw->setGeometry(0,0,100,100);
        dw->setWidget(new RwaMapView(this, backend->getFirstScene()));
        dw->installEventFilter(this);
        break;

        default:
        return;
    }

    switch (area) {
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
            addDockWidget(area, dw);

            break;
        default:
            if (!restoreDockWidget(dw)) {
                QMessageBox::warning(this, QString(), tr("Failed to restore dock widget"));
                delete dw;
                return;
            }
            break;
    }

    rwaDockWidgets.append(dw);
}

void RwaCreator::logMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(logWindow)
        logWindow->outputMessage( type, context, msg );
}

void RwaCreator::loadDefaultViews()
{
    QDockWidget *dw = new QDockWidget;
    RwaMapView *newMapView;
    newMapView = new RwaMapView(this, backend->getFirstScene());

    dw->setObjectName(tr("Map View"));
    dw->setWindowTitle(tr("Map View"));
    dw->setGeometry(0,0,1000,300);
    dw->setWidget(newMapView);
    dw->installEventFilter(this);
    dw->setWindowFlags(Qt::WindowStaysOnTopHint );

    addDockWidget(Qt::TopDockWidgetArea, dw);
    rwaDockWidgets.append(dw);

    QDockWidget *dw1 = new QDockWidget;
    dw1->setObjectName(tr("State View"));
    dw1->setWindowTitle(tr("State View"));
    dw1->setGeometry(0,0,1000,300);
    dw1->setWidget(new RwaStateView(this, backend->getFirstScene()));
    dw1->installEventFilter(this);
    addDockWidget(Qt::BottomDockWidgetArea, dw1);
    rwaDockWidgets.append(dw1);

    QDockWidget *dw2 = new QDockWidget;
    dw2->setObjectName(tr("Scene View"));
    dw2->setWindowTitle(tr("Scene View"));
    dw2->setGeometry(0,0,1000,300);
    dw2->setWidget(new RwaSceneView(this, backend->getFirstScene()));
    dw2->installEventFilter(this);
    addDockWidget(Qt::LeftDockWidgetArea, dw2);
    rwaDockWidgets.append(dw2);

    logWindow = new RwaLogWindow(this);
    QDockWidget *dw3 = new QDockWidget;
    dw3->setObjectName(tr("Log Window"));
    dw3->setWindowTitle(tr("Log Window"));
    dw3->setGeometry(0,0,1000,300);
    dw3->setWidget(logWindow);
    addDockWidget(Qt::RightDockWidgetArea, dw3);
    rwaDockWidgets.append(dw3);

    QDockWidget *dw4 = new QDockWidget;
    dw4->setObjectName(tr("Game View"));
    dw4->setWindowTitle(tr("Game View"));
    dw4->setGeometry(0,0,1000,300);
    dw4->setWidget(new RwaGameView(this, backend->getFirstScene()));
    dw4->installEventFilter(this);
    addDockWidget(Qt::LeftDockWidgetArea, dw4);
    rwaDockWidgets.append(dw4);

    QDockWidget *dw5 = new QDockWidget;
    dw5->setObjectName(tr("History"));
    dw5->setWindowTitle(tr("History"));
    dw5->setGeometry(0,0,1000,300);
    dw5->setWidget(new RwaHistory(this));
    addDockWidget(Qt::BottomDockWidgetArea, dw5);
    rwaDockWidgets.append(dw5);
    allViewsLoaded = true;
}

void RwaCreator::destroyDockWidget(QAction *action)
{
    int index = destroyDockWidgetMenu->actions().indexOf(action);
    delete rwaDockWidgets.takeAt(index);
    destroyDockWidgetMenu->removeAction(action);
    action->deleteLater();
    qDebug("deleteDockWidget");

    if (destroyDockWidgetMenu->isEmpty())
        destroyDockWidgetMenu->setEnabled(false);
}
