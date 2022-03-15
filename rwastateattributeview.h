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
    void setCurrentScene(RwaScene *scene);
    void receiveEditingFinished();
    void receiveSelectedStates(QStringList states);

private slots:
    void receiveCheckBoxAttributeValue(int id, bool value);
    void receiveLineEditAttributeValue(const QString &value);
    void receiveLineEditAttributeValue();
    void receiveComboBoxAttributeValue(int index);
    void receiveFaderAttributeValue(int id);

signals:
    void sendCurrentStateRadiusEdited();

private:
   RwaView *mother;
   QStringList selectedStates;

   void updateStateComboBox(QComboBox *attrComboBox);
   void updateStateAttr(QComboBox *attrComboBox, QString state2compare);
   void updateSceneComboBox(QComboBox *attrComboBox);
   void updateSceneAttr(QComboBox *attrComboBox, QString scene2compare);
protected:
   QGridLayout *innerLayout;
};

#endif // RWASTATEATTRIBUTEVIEW_H
