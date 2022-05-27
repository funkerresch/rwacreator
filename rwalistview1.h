#ifndef RWALISTVIEW1_H
#define RWALISTVIEW1_H

#include "rwabackend.h"
#include "QListView"
#include "QStringListModel"

class RwaListView1 : public QListView
{
    Q_OBJECT
public:
    RwaListView1(QWidget *parent);

public slots:
    virtual void update() = 0;
    virtual void receiveNewGameSignal() = 0;
    virtual void setCurrentState(RwaState *state) = 0;

protected:

    RwaBackend *backend;
    QStringListModel *listModel;
};

#endif // RWALISTVIEW1_H
