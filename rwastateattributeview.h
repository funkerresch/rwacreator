#ifndef RWASTATEATTRIBUTEVIEW_H
#define RWASTATEATTRIBUTEVIEW_H


#include "rwaattributeview.h"


class RwaStateAttributeView : public RwaAttributeView
{
    Q_OBJECT
public:
    RwaStateAttributeView(QWidget *parent, RwaScene *scene);

public slots:
    void updateStateArea();
    void adaptSize(qint32 width, qint32 height);
    void setCurrentState(RwaState *currentState);
    void setCurrentScene(RwaScene *currentScene);
    void receiveEditingFinished();

private slots:
    void receiveCheckBoxAttributeValue(int id, bool value);
    void receiveLineEditAttributeValue(const QString &value);
    void receiveLineEditAttributeValue();
    void receiveComboBoxAttributeValue(QString value);
    void receiveFaderAttributeValue(int id);

signals:
    void sendCurrentStateRadiusEdited();

private:
   RwaView *mother;
   QList <QComboBox *> visitedStateComboBoxes;
   qint32 visitedStateConditionCount;

   void updateStateComboBox(QComboBox *attrComboBox);
   void updateStateAttr(QComboBox *attrComboBox, QString state2compare);
   void updateSceneComboBox(QComboBox *attrComboBox);
   void updateSceneAttr(QComboBox *attrComboBox, QString scene2compare);
protected:
   QGridLayout *innerLayout;
};

#endif // RWASTATEATTRIBUTEVIEW_H
