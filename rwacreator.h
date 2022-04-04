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

class RwaDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    RwaDockWidget(QWidget *parent = nullptr) : QDockWidget(parent) {

        QObject::connect(this, SIGNAL(closeWindow(RwaDockWidget *)), parent, SLOT(closeDockWidget(RwaDockWidget *)));
    }

signals:
    void closeWindow(RwaDockWidget *me);

protected:
    void closeEvent(QCloseEvent *event) override
    {
        qDebug() << "Close DockWidget";
        emit closeWindow(this);
        event->ignore();
    }
};

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
    void cleanUpBeforeQuit();
    void selectOutputDevice(qint32 index);
    void loadDefaultViews();
    void scanSerialPorts();
    void clear();
    void saveAs();
    void enterHtName();
    void selectInputDevice(qint32 index);
    void openInit();
    void writeInit();
    void readUndoFile(QString name);

private slots:
    void addMapView();
    void addSceneView();
    void addGameView();
    void addStateView();
    void addHistoryView();
    void addLogView();
    void closeEvent(QCloseEvent *event);
    void closeDockWidget(RwaDockWidget *dock);
private:
    static RwaLogWindow *logWindow;
    RwaBackend *backend = nullptr;
    RwaHeadtrackerConnect *headtracker = nullptr;
    QList<RwaDockWidget *> rwaDockWidgets = QList<RwaDockWidget *>();
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
    void initFileMenu(QMenu *fileMenu);
    void initEditMenu(QMenu *fileMenu);
    void setWindowPositionOccupied(int position, char occupied);
    void initAudioPreferencesMenu(QMenu *audioDeviceMenu);
    void initHeadtrackerMenu(QMenu *headtrackerMenu);
    void createInitFolder();

    bool maybeSave();
    void initViewMenu1(QMenu *fileMenu);
};

#endif
