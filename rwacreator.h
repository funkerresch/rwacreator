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

#ifndef AFX_WINDOWPOSITIONS
#define AFX_WINDOWPOSITIONS

#define AFX_WINDOWPOSITION_TOP 1
#define AFX_WINDOWPOSITION_RIGHT 2
#define AFX_WINDOWPOSITION_BOTTOM 3
#define AFX_WINDOWPOSITION_LEFT 4

#define AFX_WINDOWPOSITION_FREE 0
#define AFX_WINDOWPOSITION_OCCUPIED 1
#endif

#ifndef AFX_WINDOWTYPES
#define AFX_WINDOWTYPES

#define AFX_WINDOWTYPE_SCRIPTTEXT 1
#define AFX_WINDOWTYPE_SCRIPTGUI 2
#define AFX_WINDOWTYPE_ASSET 3
#define AFX_WINDOWTYPE_MAPVIEW 4

#define RWA_MAPVIEW 1
#define RWA_STATEVIEW 2
#define RWA_TTSVIEW 3

#define RWA_EXPORT_FOR_DEVCLIENT 1


#endif



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

class RwaCreator : public QMainWindow
{
    Q_OBJECT

public:
    RwaCreator(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
    //~RwaCreator();

public slots:
    static void logMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void mousePressEvent(QMouseEvent *event);
    void saveLayout();
    void exportProject();
    void saveForMobileClient();
    void saveAgainForMobileClient();
    void newProject();
    void save();
    void write(QString writeMessage, qint32 flags, QString oldAssetPath);
    void prepareWrite(QString fullpath, int flags = 0);
    void open(QString fileName = QString(""));
    void writeUndo(QString undoAction);
    void loadLayout();
    void setCorner(int id);
    void cleanUpBeforeQuit();
    void selectOutputDevice(qint32 index);
    void createDockWidget();
    void loadDefaultViews();
    void destroyDockWidget(QAction *action);
    void scanSerialPorts();
    void clear();
    void saveAs();
    void enterHtName();
    void selectInputDevice(qint32 index);
    void openInit();
    void writeInit();

    void readUndoFile(QString name);

private:
    static RwaLogWindow *logWindow;
    RwaBackend *backend = nullptr;
    RwaHeadtrackerConnect *headtracker = nullptr;
    QList<QDockWidget *> rwaDockWidgets = QList<QDockWidget *>();
    QSignalMapper *mapper = nullptr;
    QMenu *destroyDockWidgetMenu = nullptr;
    QMenu *headtrackerMenu = nullptr;
    QAction *createDockWidgetAction = nullptr;
    QAction *defaultViewWidgetAction = nullptr;
    QAction *selectAudioDeviceAction = nullptr;
    QAction *selectHeadtrackerAction = nullptr;
    QString headtrackerSerialPort = QString();
    qint32 headtrackerSerialPortIndex = -1;
    bool allViewsLoaded = false;
    quint32 undoCounter = 0;

    void showEvent(QShowEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void setupMenuBar();
    void initViewMenu(QMenu *viewMenu);
    void initFileMenu(QMenu *fileMenu);
    void initEditMenu(QMenu *fileMenu);
    void setWindowPositionOccupied(int position, char occupied);
    void initAudioPreferencesMenu(QMenu *audioDeviceMenu);
    void initHeadtrackerMenu(QMenu *headtrackerMenu);
    void createInitFolder();

};

#endif
