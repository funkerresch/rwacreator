#ifndef RWASCENEVIEW_H
#define RWASCENEVIEW_H

#include "rwaview.h"
#include "rwastatelist.h"
#include "rwastateattributeview.h"

class RwaSceneView : public RwaView
{
    Q_OBJECT
public:
    explicit RwaSceneView(QWidget *parent, RwaScene *scene);


public slots:
    void adaptSize(qint32 width, qint32 height);

    void deleteState(const QString &stateName);
protected:
    void setCurrentState(RwaState *currentState);
    void setCurrentScene(RwaScene *currentScene);

private:

    RwaStateList *stateList;
    RwaStateAttributeView *stateAttributes;



signals:
    void updateAttributes();

};

#endif // RWASTATEVIEW_H
