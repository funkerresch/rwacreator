#ifndef RWAGAMEVIEW_H
#define RWAGAMEVIEW_H

#include "rwaview.h"
#include "rwascenelist.h"
#include "rwasceneattributeview.h"

class RwaGameView : public RwaView
{
    Q_OBJECT
public:
    explicit RwaGameView(QWidget *parent, RwaScene *scene);


public slots:
    void adaptSize(qint32 width, qint32 height);

    void deleteScene(const QString &sceneName);
protected:
    void setCurrentScene(RwaScene *scene);

private:

    RwaSceneList *sceneList;
    RwaSceneAttributeView *sceneAttributes;



signals:
    void updateAttributes();

};

#endif // RWASTATEVIEW_H
