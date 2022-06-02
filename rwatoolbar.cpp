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

#include "rwatoolbar.h"
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QSpinBox>
#include <QLabel>
#include <QSignalMapper>
#include <QStyle>
#include <stdlib.h>
#include <QToolButton>
#include "qoscclient.h"
#include "qosctypes.h"

RwaViewToolbar::RwaViewToolbar(const QString &title, qint32 flags, RwaBackend *backend,  QWidget *parent)
    : QToolBar(parent)
{

    numberOfScenes = 1;
    this->currentScene  = nullptr;
    this->currentState = nullptr;
    this->backend = backend;
    this->coordinates = std::vector<double>(2, 0.0);

    setWindowTitle(title);
    setObjectName(title);

    connect(backend, SIGNAL(sendLastTouchedState(RwaState *)),
              this, SLOT(setCurrentState(RwaState*)));

    connect(backend, SIGNAL(sendLastTouchedScene(RwaScene *)),
              this, SLOT(setCurrentScene(RwaScene*)));

    connect(backend, SIGNAL(sendClearAll()), this, SLOT(updateAll()));

    if(flags & RWATOOLBAR_MAPEDITTOOLS)
    {
        initToolButtonGroup();
        connect (this, SIGNAL(sendSelectedTool(int)), parent, SLOT(setTool(qint32)));
    }

    if(flags & RWATOOLBAR_SIMULATORTOOLS)
    {
        connect (this, SIGNAL(sendAssetsVisible(bool)), parent, SLOT(setAssetsVisible(bool)));
        connect (this, SIGNAL(sendRadiiVisible(bool)), parent, SLOT(setRadiiVisible(bool)));
        connect (this, SIGNAL(sendAssetsVisible(bool)), backend, SLOT(receiveShowAssets(bool))); // This is temporary.. not nice connected to 2 slots
        connect (this, SIGNAL(sendRadiiVisible(bool)), backend, SLOT(receiveShowStateRadii(bool)));
        connect (this, SIGNAL(sendStartStopSimulator(bool)), backend, SLOT(startStopSimulator(bool)));
        connect (this, SIGNAL(sendTrashAssets(bool)), backend, SLOT(receiveTrashAssets(bool)));
        connect (this, SIGNAL(sendCalibrateHeadtracker()), backend, SLOT(calibrateHeadtracker()));
        connect (this, SIGNAL(sendSimulateHeadtrackerStep()), backend->simulator, SLOT(receiveStep()));
        initControls();
    }

    if(flags & RWATOOLBAR_MAINVOLUME)
    {
        mainVolume = new QSlider(Qt::Horizontal, this);
        mainVolume->setRange(0, 400);
        mainVolume->setFixedWidth(60);
        mainVolume->setFixedHeight(18);
        mainVolume->setValue(100);
        addWidget(mainVolume);
        connect(mainVolume, SIGNAL(valueChanged(int)), this->backend, SLOT(setMainVolume(int)));
    }

    if(flags & RWATOOLBAR_SELECTSCENEMENU)
    {
        selectSceneGroup = new QActionGroup(this);
        selectSceneMenu = new QMenu("Select Scene", this);
        selectSceneMenu->setTitle(tr("Select Scene"));

        QToolButton *selectSceneMenuButton = new QToolButton(this);
        selectSceneMenuButton->setPopupMode(QToolButton::InstantPopup);
        selectSceneMenuButton->setText("Select Scene");
        selectSceneMenuButton->setMenu(selectSceneMenu);
        addWidget(selectSceneMenuButton);
        updateSelectSceneMenu();
        connect (this, SIGNAL(sendSelectedScene(RwaScene *)), backend, SLOT(receiveLastTouchedScene(RwaScene *)));
   }

    if(flags & RWATOOLBAR_SELECTSTATEMENU)
    {
        selectStateGroup = new QActionGroup(this);
        selectStateMenu = new QMenu("Select State", this);
        selectStateMenu->setTitle(tr("Select State"));

        QToolButton *selectStateMenuButton = new QToolButton(this);
        selectStateMenuButton->setPopupMode(QToolButton::InstantPopup);
        selectStateMenuButton->setText("Select State");
        selectStateMenuButton->setMenu(selectStateMenu);
        addWidget(selectStateMenuButton);
        updateSelectStateMenu();
        connect (this, SIGNAL(sendSelectedState(RwaState *)), backend, SLOT(receiveLastTouchedState(RwaState*)));
   }

    if(flags & RWATOOLBAR_SCENEMENU)
    {
        sceneMenu = new QMenu("Scene", this);
        sceneMenu->setTitle(tr("Scene"));

        QToolButton *sceneMenuButton = new QToolButton(this);
        sceneMenuButton->setPopupMode(QToolButton::InstantPopup);
        sceneMenuButton->setText("Scene Menu");
        sceneMenuButton->setMenu(sceneMenu);
        addWidget(sceneMenuButton);
        initSceneMenu();

        connect (this, SIGNAL(sendAppendScene()), backend, SLOT(appendScene()));
        connect (this, SIGNAL(sendRemoveScene(RwaScene *)), backend, SLOT(removeScene(RwaScene *)));
        connect (this, SIGNAL(sendDuplicateScene(RwaScene *)), backend, SLOT(duplicateScene(RwaScene *)));
        connect (this, SIGNAL(sendClearScene(RwaScene *)), backend, SLOT(clearScene(RwaScene *)));
        connect (this, SIGNAL(sendNewSceneFromSelectedStates()), backend, SLOT(newSceneFromSelectedStates()));
        connect (this, SIGNAL(sendMoveScene2NewLocation()), backend, SLOT(moveScene2CurrentMapLocation()));
        connect (this, SIGNAL(sendCopySelectedStates2Clipboard()), backend, SLOT(copySelectedStates2Clipboard()));
        connect (this, SIGNAL(sendPasteClipboardStates2CurrentScene()), backend, SLOT(pasteStatesFromClipboard()));
        connect (this, SIGNAL(sendWriteUndo(QString)), backend, SLOT(receiveWriteUndo(QString)));
    }

    if(flags & RWATOOLBAR_STATEMENU)
    {
        stateMenu = new QMenu("State", this);
        stateMenu->setTitle(tr("State"));

        QToolButton *stateMenuButton = new QToolButton(this);
        stateMenuButton->setPopupMode(QToolButton::InstantPopup);
        stateMenuButton->setText("State Menu");
        stateMenuButton->setMenu(stateMenu);
        addWidget(stateMenuButton);
        initStateMenu();
    }

    if(flags & RWATOOLBAR_VIEWMENU)
    {
        viewPreferencesMenu = new QMenu("View Prefs",this);
        viewPreferencesMenu->setTitle(tr("View Preferences"));
        initViewPreferencesMenu();

        QToolButton *viewPreferencesMenuButton = new QToolButton(this);
        viewPreferencesMenuButton->setPopupMode(QToolButton::InstantPopup);
        viewPreferencesMenuButton->setText("View");
        viewPreferencesMenuButton->setMenu(viewPreferencesMenu);
        addWidget(viewPreferencesMenuButton);
    }

    setFixedHeight(24);
}

void RwaViewToolbar::showFindPlacesDialog()
{
     searchDialog->hide();
     searchDialog->show();
}

void RwaViewToolbar::setCurrentState(RwaState *currentState)
{
    this->currentState = currentState;
    updateSelectStateMenu();
    if(!(QObject::sender() == this->backend))
    {
        emit sendSelectedState(currentState);
        setMapCoordinates(currentState->getCoordinates()[0], currentState->getCoordinates()[1]);
    }
}

void RwaViewToolbar::setCurrentState(qint32 stateNumber)
{
    if(!currentScene)
        return;
    if(currentScene->getStates().empty())
        return;

    auto statesList = currentScene->getStates().begin();
    std::advance(statesList, stateNumber);

    currentState = *statesList;
    setMapCoordinates(currentState->getCoordinates()[0], currentState->getCoordinates()[1]);
    updateSelectStateMenu();
    if(!(QObject::sender() == this->backend))
    {
        emit sendSelectedState(currentState);
    }
}

void RwaViewToolbar::setCurrentScene(RwaScene *currentScene)
{
    this->currentScene = currentScene;

    updateSelectSceneMenu();
    if(!(QObject::sender() == this->backend))
    {
        emit sendSelectedScene(currentScene);
        qDebug();
    }
}

void RwaViewToolbar::setCurrentScene(qint32 sceneNumber)
{
    if(sceneNumber <= backend->getNumberOfScenes())
        setCurrentScene(backend->getSceneAt(sceneNumber));

    updateSelectSceneMenu();

    if(!(QObject::sender() == this->backend))
    {
        emit sendSelectedScene(currentScene);
    }
}

void RwaViewToolbar::receiveMapCoordinates(double lon, double lat)
{
    coordinates[0] = lon;
    coordinates[1] = lat;
}

void RwaViewToolbar::setMapCoordinates(double lon, double lat)
{
    emit sendMapCoordinates(lon, lat);
}

void RwaViewToolbar::initToolButtonGroup()
{
    QString path = backend->completeBundlePath;
    selectTool = new QButtonGroup(this);

    QToolButton *arrow = new QToolButton(this);
    arrow->setCheckable(true);
    arrow->setObjectName("arrow");
    arrow->setIcon(QIcon(path+"images/arrow.png"));
    arrow->setIconSize(QSize(20,20));
    arrow->setFixedSize(QSize(20,20));
    arrow->setToolTip("Arrow Tool: Move Map, Double Click for new State, Click for selecting State, Drag for moving State.");
    selectTool->addButton(arrow);
    selectTool->setId(arrow, 1);
    addWidget(arrow);

    QToolButton *pen = new QToolButton(this);
    pen->setCheckable(true);
    pen->setObjectName("pen");
    pen->setIcon(QIcon(path+"images/pen.png"));
    pen->setIconSize(QSize(20,20));
    pen->setFixedSize(QSize(20,20));
    pen->setToolTip("Selection Tool: Draw state selection into map.");
    selectTool->addButton(pen);
    selectTool->setId(pen, 2);
    addWidget(pen);

    QToolButton *rubber = new QToolButton(this);
    rubber->setCheckable(true);
    rubber->setObjectName("rubber");
    rubber->setIcon(QIcon(path+"images/rubber.png"));
    rubber->setIconSize(QSize(20,20));
    rubber->setFixedSize(QSize(20,20));
    rubber->setToolTip("Rubber Tool: Click for removing States.");
    selectTool->addButton(rubber);
    selectTool->setId(rubber, 3);
    addWidget(rubber);

//    QToolButton *markee = new QToolButton(this);
//    markee->setCheckable(true);
//    markee->setObjectName("markee");
//    markee->setIcon(QIcon(path+"images/markee.png"));
//    markee->setIconSize(QSize(20,20));
//    markee->setFixedSize(QSize(20,20));
//    selectTool->addButton(markee);
//    selectTool->setId(markee, 4);
//    addWidget(markee);
//    selectTool->setExclusive(true);

    connect(selectTool, SIGNAL(buttonClicked(int)), this, SLOT(receiveClickedTool(int)));
}

void RwaViewToolbar::receiveClickedTool(int tool)
{
    QAbstractButton *button;
    button = selectTool->button(tool);
    emit sendSelectedTool(tool);
}

void RwaViewToolbar::initControls()
{
    QString path = backend->completeBundlePath;
    assetsVisibleButton = new QToolButton(this);
    assetsVisibleButton->setCheckable(true);
    connect (assetsVisibleButton, SIGNAL(clicked(bool)), this, SLOT(receiveAssetsVisible(bool)));
    assetsVisibleButton->setObjectName("assetsVisibleButton");
    assetsVisibleButton->setIcon(QIcon(path+"images/audiosource.png"));
    assetsVisibleButton->setIconSize(QSize(20,20));
    assetsVisibleButton->setFixedSize(QSize(20,20));
    assetsVisibleButton->setToolTip("Show Assets.");
    addWidget(assetsVisibleButton);

    radiiVisibleButton = new QToolButton(this);
    radiiVisibleButton->setCheckable(true);
    connect (radiiVisibleButton, SIGNAL(clicked(bool)), this, SLOT(receiveRadiiVisible(bool)));
    radiiVisibleButton->setObjectName("radiiVisibleButton");
    radiiVisibleButton->setIcon(QIcon(path+"images/radiiVisibleButton.png"));
    radiiVisibleButton->setIconSize(QSize(20,20));
    radiiVisibleButton->setFixedSize(QSize(20,20));
    radiiVisibleButton->setToolTip("Show State radii.");
    addWidget(radiiVisibleButton);

    startSimulatorButton = new QToolButton(this);
    startSimulatorButton->setCheckable(true);
    connect (startSimulatorButton, SIGNAL(clicked(bool)), this, SLOT(receiveStartSimulator(bool)));
    startSimulatorButton->setObjectName("startStopSimulatorButton");
    startSimulatorButton->setIcon(QIcon(path+"images/start.png"));
    startSimulatorButton->setIconSize(QSize(20,20));
    startSimulatorButton->setFixedSize(QSize(20,20));
    startSimulatorButton->setToolTip("Start simulation.");
    addWidget(startSimulatorButton);

    stopSimulatorButton = new QToolButton(this);
    stopSimulatorButton->setCheckable(false);
    connect (stopSimulatorButton, SIGNAL(clicked(bool)), this, SLOT(receiveStopSimulator(bool)));
    stopSimulatorButton->setObjectName("startStopSimulatorButton");
    stopSimulatorButton->setIcon(QIcon(path+"images/stop.png"));
    stopSimulatorButton->setIconSize(QSize(20,20));
    stopSimulatorButton->setFixedSize(QSize(20,20));
    stopSimulatorButton->setToolTip("Stop simulation.");
    addWidget(stopSimulatorButton);

    calibrateHeadtrackerButton = new QToolButton(this);
    calibrateHeadtrackerButton->setCheckable(false);
    connect (calibrateHeadtrackerButton, SIGNAL(clicked(bool)), this, SLOT(receiveCalibrateHeadtracker(bool)));
    calibrateHeadtrackerButton->setObjectName("calibrateHeadtrackerButton");
    calibrateHeadtrackerButton->setIcon(QIcon(path+"images/calibrateHeadtrackerButton.png"));
    calibrateHeadtrackerButton->setIconSize(QSize(20,20));
    calibrateHeadtrackerButton->setFixedSize(QSize(20,20));
    calibrateHeadtrackerButton->setToolTip("Calibrate head tracker.");
    addWidget(calibrateHeadtrackerButton);

    simulateHeadtrackerStepButton = new QToolButton(this);
    simulateHeadtrackerStepButton->setCheckable(false);
    simulateHeadtrackerStepButton->setChecked(false);
    connect (simulateHeadtrackerStepButton, SIGNAL(clicked(bool)), this, SLOT(receiveSendHeadtrackerStep(bool)));
    simulateHeadtrackerStepButton->setObjectName("headtrackerStepButton");
    simulateHeadtrackerStepButton->setIcon(QIcon(path+"images/headtrackerStepButton.png"));
    simulateHeadtrackerStepButton->setIconSize(QSize(15,15));
    simulateHeadtrackerStepButton->setFixedSize(QSize(20,20));
    simulateHeadtrackerStepButton->setToolTip("Send Step");
    addWidget(simulateHeadtrackerStepButton);

    trashAssetsButton = new QToolButton(this);
    trashAssetsButton->setCheckable(true);
    trashAssetsButton->setChecked(false);
    connect (trashAssetsButton, SIGNAL(clicked(bool)), this, SLOT(receiveTrashAssets(bool)));
    trashAssetsButton->setObjectName("trashAssetsButton");
    trashAssetsButton->setIcon(QIcon(path+"images/donttrashassets.png"));
    trashAssetsButton->setIconSize(QSize(20,20));
    trashAssetsButton->setFixedSize(QSize(20,20));
    trashAssetsButton->setToolTip("On asset delete: keep/remove Assets on/from Disk.");
    addWidget(trashAssetsButton);

    findButton = new QToolButton(this);
    findButton->setCheckable(false);
    connect(findButton, SIGNAL(clicked(bool)), this, SLOT(showFindPlacesDialog()));
    findButton->setObjectName("FindMapLocation");
    findButton->setIcon(QIcon(path+"images/findlocation.png"));
    findButton->setIconSize(QSize(20,20));
    findButton->setFixedSize(QSize(20,20));
    findButton->setToolTip("Search for new map location.");
    addWidget(findButton);
    searchDialog = new RwaSearchDialog(this);
    connect (searchDialog, SIGNAL(sendMapCoordinates(double, double)), this, SLOT(setMapCoordinates(double, double)));
}

void RwaViewToolbar::receiveCurrentScene(RwaScene *scene)
{
    currentScene = scene;
    updateSelectStateMenu();
    updateSelectSceneMenu();
}

void RwaViewToolbar::receiveAssetsVisible(bool assetsVisible)
{
    emit sendAssetsVisible(assetsVisible);
}

void RwaViewToolbar::receiveRadiiVisible(bool radiiVisible)
{
    emit sendRadiiVisible(radiiVisible);
}

void RwaViewToolbar::receiveStartSimulator(bool startStopSimulator)
{
    startSimulatorButton->setChecked(startStopSimulator);
    emit sendStartStopSimulator(startStopSimulator);
}

void RwaViewToolbar::receiveTrashAssets(bool onOff)
{
    QString path = backend->completeBundlePath;
    trashAssetsButton->setChecked(onOff);
    emit sendTrashAssets(onOff);
    if(onOff)
        trashAssetsButton->setIcon(QIcon(path+"images/trashassets.png"));
    else
        trashAssetsButton->setIcon(QIcon(path+"images/donttrashassets.png"));
}

void RwaViewToolbar::receiveStopSimulator(bool startStopSimulator)
{
    startSimulatorButton->setChecked(startStopSimulator);
    emit sendStartStopSimulator(startStopSimulator);
}

void RwaViewToolbar::receiveCalibrateHeadtracker(bool onOff)
{
    calibrateHeadtrackerButton->setChecked(false);
    emit sendCalibrateHeadtracker();
}

void RwaViewToolbar::receiveSendHeadtrackerStep(bool onOff)
{
    simulateHeadtrackerStepButton->setChecked(false);
    emit sendSimulateHeadtrackerStep();
}

void RwaViewToolbar::updateSelectSceneMenu()
{
   if(!backend->getNumberOfScenes())
        return;

    if(selectSceneMenu)
        selectSceneMenu->clear();

    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    selectSceneGroup->setExclusive(true);

    for (int i=0; i<backend->getNumberOfScenes(); i++)
    {
        selectSceneAction = new QAction(this);
        selectSceneAction->setCheckable(true);
        RwaScene *scene = backend->getSceneAt(i);
        selectSceneAction->setText(QString::fromLatin1("%1").arg(QString::fromStdString(scene->objectName())));
        connect (selectSceneAction, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
        signalMapper->setMapping (selectSceneAction, i) ;
        selectSceneMenu->addAction(selectSceneAction );
        selectSceneGroup->addAction(selectSceneAction);
        if(backend->getSceneAt(i) == currentScene)
            selectSceneAction->setChecked(true);
    }

    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(selectScene(qint32))) ;
}

void RwaViewToolbar::updateSelectStateMenu()
{
    if(selectStateMenu)
        selectStateMenu->clear();

    if(!backend->getNumberOfScenes() || !currentScene)
    {
        selectStateAction = new QAction(this);
        selectStateAction->setText("No active Scene");
        selectStateMenu->addAction(selectStateAction );
        return;
    }

    if(currentScene->getStates().empty())
    {
        selectStateAction = new QAction(this);
        selectStateAction->setText("No States");
        selectStateMenu->addAction(selectStateAction );
        return;
    }

    QSignalMapper* signalMapper = new QSignalMapper (this) ;

    for (int32_t i=0; i<currentScene->getStates().size(); i++)
    {
        selectStateAction = new QAction(this);
        selectStateAction->setCheckable(true);
        auto statesList = currentScene->getStates().begin();
        std::advance(statesList, i);
        RwaState *state = *statesList;
        selectStateAction->setText(QString::fromLatin1("%1").arg(QString::fromStdString(state->objectName())));
        connect (selectStateAction, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
        signalMapper->setMapping (selectStateAction, i) ;
        selectStateMenu->addAction(selectStateAction );
        selectStateGroup->addAction(selectStateAction);
        if(state == currentState)
            selectStateAction->setChecked(true);
    }
    selectStateGroup->setExclusive(true);
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(selectState(qint32))) ;
}

void RwaViewToolbar::updateAll()
{
    updateSelectSceneMenu();
    updateSelectStateMenu();
}

void RwaViewToolbar::initViewPreferencesMenu()
{
    showMapAction = new QAction(this);
    showMapAction->setText(tr("Show Map"));
    connect (showMapAction, SIGNAL(triggered()), this, SLOT(showMap())) ;
    viewPreferencesMenu->addAction(showMapAction );
}

void RwaViewToolbar::initStateMenu()
{
    if(stateMenu)
        stateMenu->clear();

    newGpsStateAction = new QAction(this);
    newGpsStateAction->setText(tr("New State"));
    connect (newGpsStateAction, SIGNAL(triggered()), this, SLOT(newGpsState())) ;
    stateMenu->addAction(newGpsStateAction );

    newStateFromCurrentAction = new QAction(this);
    newStateFromCurrentAction->setText(tr("New State from Current"));
    connect (newStateFromCurrentAction, SIGNAL(triggered()), this, SLOT(newStateFromCurrent())) ;
    stateMenu->addAction(newStateFromCurrentAction );

//    newBluetoothStateAction = new QAction(this);
//    newBluetoothStateAction->setText(tr("New Bluetooth State"));
//    connect (newBluetoothStateAction, SIGNAL(triggered()), this, SLOT(newBluetoothState())) ;
//    stateMenu->addAction(newBluetoothStateAction );
}

void RwaViewToolbar::initSceneMenu()
{
    if(sceneMenu)
        sceneMenu->clear();

    appendSceneAction = new QAction(this);
    appendSceneAction->setText(tr("Append"));
    connect (appendSceneAction, SIGNAL(triggered()), this, SLOT(appendScene())) ;
    sceneMenu->addAction(appendSceneAction );

    removeSceneAction = new QAction(this);
    removeSceneAction->setText(tr("Remove"));
    connect (removeSceneAction, SIGNAL(triggered()), this, SLOT(removeScene())) ;
    sceneMenu->addAction(removeSceneAction );

    copySceneAction = new QAction(this);
    copySceneAction->setText(tr("Duplicate"));
    connect (copySceneAction, SIGNAL(triggered()), this, SLOT(duplicateScene())) ;
    sceneMenu->addAction(copySceneAction );

    clearSceneAction = new QAction(this);
    clearSceneAction->setText(tr("Clear"));
    connect (clearSceneAction, SIGNAL(triggered()), this, SLOT(clearScene())) ;
    sceneMenu->addAction(clearSceneAction );

    newSceneFromSelectedStatesAction = new QAction(this);
    newSceneFromSelectedStatesAction->setText(tr("New Scene from Selected States"));
    connect (newSceneFromSelectedStatesAction, SIGNAL(triggered()), this, SLOT(newSceneFromSelectedStates())) ;
    sceneMenu->addAction(newSceneFromSelectedStatesAction );

    moveScene2NewLocationAction = new QAction(this);
    moveScene2NewLocationAction->setText(tr("Move Scene to Map Coordinates"));
    connect (moveScene2NewLocationAction, SIGNAL(triggered()), this, SLOT(moveScene2NewLocation())) ;
    sceneMenu->addAction(moveScene2NewLocationAction );

    copySelectedStates2ClipboardAction = new QAction(this);
    copySelectedStates2ClipboardAction->setText(tr("Copy selected States to Clipboard"));
    connect (copySelectedStates2ClipboardAction, SIGNAL(triggered()), this, SIGNAL(sendCopySelectedStates2Clipboard()));
    sceneMenu->addAction(copySelectedStates2ClipboardAction );

    pasteClipboardStates2CurrentSceneAction = new QAction(this);
    pasteClipboardStates2CurrentSceneAction->setText(tr("Paste States to Current Scene"));
    connect (pasteClipboardStates2CurrentSceneAction, SIGNAL(triggered()), this, SIGNAL(sendPasteClipboardStates2CurrentScene()));
    sceneMenu->addAction(pasteClipboardStates2CurrentSceneAction );
}

void RwaViewToolbar::selectScene(qint32 sceneNumber)
{
    setCurrentScene(sceneNumber);
}

void RwaViewToolbar::selectState(qint32 stateNumber)
{
    setCurrentState(stateNumber);
}

void RwaViewToolbar::receiveCurrentState(RwaState *state)
{
    currentState = state;
    updateSelectStateMenu();
}

void RwaViewToolbar::newStateFromCurrent()
{
    if(currentState)
    {
        std::string stateName(currentState->objectName() + " copy");
        RwaState *newState = currentScene->addState(stateName, currentScene->getCoordinates());
        currentState->copyAttributes(newState);
        newState->setCoordinates(coordinates);
        qDebug() << QString::number(coordinates[0], 'f', 8);
        RwaBackend::generateUuidsForClipboardState(newState);
        setCurrentScene(currentScene);
        setCurrentState(newState);
        emit sendWriteUndo("State from current");
    }
}

void RwaViewToolbar::newSceneFromSelectedStates()
{
    if(currentScene)
    {
        emit sendNewSceneFromSelectedStates();
        emit sendWriteUndo("Scene from selected states");
    }
}

void RwaViewToolbar::moveScene2NewLocation()
{
    emit sendMoveScene2NewLocation();
    emit sendWriteUndo("Move Scene to new Location");
}

void RwaViewToolbar::newGpsState()
{
    std::string stateName("State " + std::to_string(RwaBackend::getStateNameCounter(currentScene->getStates())));
    RwaState *newState = currentScene->addState(stateName, coordinates);
    setCurrentScene(currentScene);
    setCurrentState(newState);
    emit sendWriteUndo("New State");
}

void RwaViewToolbar::newBluetoothState()
{
    qDebug() << "New Bluetooth State not implemented yet!";
}

void RwaViewToolbar::appendScene()
{
    emit sendAppendScene();
    emit sendWriteUndo("New Scene");
}

void RwaViewToolbar::removeScene()
{
    emit sendRemoveScene(currentScene);
    emit sendWriteUndo("Remove Scene");
}

void RwaViewToolbar::duplicateScene()
{
    qDebug("duplicate Scene");
}

void RwaViewToolbar::clearScene()
{
    emit sendClearScene(currentScene);
    emit sendWriteUndo("Clear Scene");
    qDebug("clear Scene");
}

