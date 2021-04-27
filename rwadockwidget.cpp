#include "rwadockwidget.h"

RwaDockWidget::RwaDockWidget(QWidget *parent, RwaBackend *backend):
    QGraphicsView(parent)
{
    this->backend = backend;
}

