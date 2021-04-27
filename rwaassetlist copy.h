#ifndef RWAASSETLIST_H
#define RWAASSETLIST_H

#include <QListWidget>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
//#include "rwaassetitem.h"
#include <QList>
#include <QDebug>
#include <QStringList>
#include "rwautilities.h"
#include "rwabackend.h"
#include <QFileInfo>
#include <QFile>

class RwaAssetList : public QListWidget
{
    Q_OBJECT

    public:
        RwaAssetList(RwaBackend *backend, QWidget *parent = 0);
        QStringList mimeTypes() const;
        qint32 getSelectedIndex();
        RwaBackend *backend;


    public slots:
        void addTts2CurrentState(QString fullpath);


    protected:
        void keyPressEvent(QKeyEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void dragEnterEvent(QDragEnterEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dropEvent(QDropEvent *event);
        void add2ListAndCopy(QString fullpath);

    private:

signals:
        void deleteAsset(const QString &path);
        void newAsset(const QString &path);
        void newAsset(const QString &path, qint32 type);


};

#endif // RWAASSETLIST_H
