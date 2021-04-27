#ifndef RWAHISTORY_H
#define RWAHISTORY_H

#include "rwabackend.h"
#include "QListView"
#include "QFileSystemModel"
#include "QStringListModel"

class RwaHistory : public QListView
{
    Q_OBJECT
public:
    RwaHistory(QWidget *parent);
    void keyPressEvent(QKeyEvent *event);

public slots:
    virtual void update();

    void receiveNewGameSignal();
protected:

    void mousePressEvent(QMouseEvent *event);
    RwaBackend *backend;
    QFileSystemModel *listModel;
 signals:
    void readUndoFile(QString name);

};

#endif // RWAHISTORY_H
