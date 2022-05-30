#ifndef RWAGAMEVIEW_H
#define RWAGAMEVIEW_H

#include "rwaview.h"
#include "rwascenelist.h"
#include "rwasceneattributeview.h"

class RwaGameView : public RwaView
{
    Q_OBJECT
public:
    explicit RwaGameView(QWidget *parent = nullptr, RwaScene *scene = nullptr, QString name = "");

//public slots:
//    void adaptSize(qint32 width, qint32 height);

private:

    RwaSceneList *sceneList;
    RwaSceneAttributeView *sceneAttributes;

signals:
    void updateAttributes();

};

#endif // RWASTATEVIEW_H
