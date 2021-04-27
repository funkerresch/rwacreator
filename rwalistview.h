#ifndef RWALISTVIEW_H
#define RWALISTVIEW_H

#include <QListWidget>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QList>
#include <QDebug>
#include <QStringList>
#include "rwautilities.h"
#include "rwabackend.h"
#include "rwaview.h"
#include <QFileInfo>
#include <QFile>

class RwaListView : public QListWidget
{
    Q_OBJECT

    public:
        RwaListView(QWidget* parent, RwaScene *scene = nullptr);
        QStringList mimeTypes() const;
        qint32 getSelectedIndex();
        RwaBackend *backend;
        RwaScene *currentScene;
        RwaState *currentState;
        RwaAsset1 *currentAsset;

    public slots:
        virtual void update();
        virtual void setCurrentAsset(RwaAsset1 *currentAsset);
        virtual void setCurrentState(RwaState *currentState);
        virtual void setCurrentState(qint32 stateNumber);
        virtual void setCurrentScene(RwaScene *currentScene);
        virtual void setCurrentScene(qint32 sceneNumber);

    protected:
        void mouseMoveEvent(QMouseEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dropEvent(QDropEvent *event);
        virtual void addItem2List(QString itemName);

        void setUndoAction(const QString &value);
private:
        bool dirty;
        QString undoAction;

signals:
    void sendCurrentState(RwaState *currentState);
    void sendCurrentScene(RwaScene *currentScene);
    void sendCurrentAsset(RwaAsset1 *currentAsset);
    void sendCurrentEntity(RwaEntity *currentEntity);
    void sendWriteUndo();
    void sendWriteUndo(QString undoAction);
protected slots:
    virtual void ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) = 0;
};

#endif // RWAASSETLIST_H
