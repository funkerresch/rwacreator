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

/** ********************************* Subclassed QDockWidget for cleanup ********************************************* */

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

/** ************************************* Rwa Creator Application Main Window ***************************************** */

class RwaCreator : public QMainWindow
{
    Q_OBJECT

public:
    RwaCreator(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
    //~RwaCreator();

public slots:

/** *************************** logMessages redirects qDebug() to rwalogview ***************************************** */

    static void logMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:

/** ************* Create RWA directory in home director and filelist of RWA games as .txt file *********************** */

    void createInitFolder();

/** ****************************** Open and write rwainit.txt for global app settings ******************************** */

    void openInit();
    void writeInit();

/** *************************************** Save and load main window layout ***************************************** */

    void saveLayoutAndSettings();
    void loadLayoutAndSettings();

/** ********************************************* Add default views ************************************************** */

private slots:

    void loadDefaultViews();
    void addMapView();
    void addSceneView();
    void addGameView();
    void addStateView();
    void addHistoryView();
    void addLogView();

/** ******************************* Save app settings and clean up before quit**************************************** */

    void closeEvent(QCloseEvent *event);
    void closeDockWidget(RwaDockWidget *dock);
    void cleanUpBeforeQuit();

public slots:

    void exportProject();
    void saveForMobileClient();
    void saveAgainForMobileClient();
    void save();
    void write(QString writeMessage, qint32 flags, QString oldAssetPath);
    void prepareWrite(QString fullpath, int flags = 0);
    qint32 open(QString fileName = QString(""));
    void writeUndo(QString undoAction);
    void selectOutputDevice(qint32 index);
    void clear();
    void saveAs();
    void enterHtName();
    void selectInputDevice(qint32 index);
    void readUndoFile(QString name);


private:
    static RwaLogWindow *logWindow;
    RwaBackend *backend = nullptr;
    RwaMapView *mapView = nullptr;
    RwaStateView *stateView = stateView;
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

    bool maybeSave();
    void initViewMenu1(QMenu *fileMenu);

    void emptyTmpDirectories();
};

#endif
