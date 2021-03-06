#ifndef RWASCENEVIEW_H
#define RWASCENEVIEW_H

#include "rwaview.h"
#include "rwastatelist.h"
#include "rwastateattributeview.h"

class RwaSceneView : public RwaView
{
    Q_OBJECT
public:
    explicit RwaSceneView(QWidget *parent, RwaScene *scene, QString name);

public slots:
    void deleteState(const QString &stateName);
protected:
    void setCurrentState(RwaState *currentState);
    void setCurrentScene(RwaScene *scene);

private:

    RwaStateList *stateList;
    RwaStateAttributeView *stateAttributes;



signals:
    void updateAttributes();

};

#endif // RWASTATEVIEW_H
