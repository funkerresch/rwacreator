#ifndef RWADOCKWIDGET_H
#define RWADOCKWIDGET_H

#include <QtWidgets>
#include "toolbar.h"
#include "rwabackend.h"
#include "rwaentity.h"

class RwaDockWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit RwaDockWidget(QWidget *parent = 0,  RwaBackend *backend = NULL);
    virtual void adaptSize(qint32 width, qint32 height) = 0;

protected:
    RwaBackend *backend;
};

#endif // RWADOCKWIDGET_H
