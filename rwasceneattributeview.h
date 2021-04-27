#ifndef RWASCENEATTRIBUTEVIEW_H
#define RWASCENEATTRIBUTEVIEW_H

#include "rwaattributeview.h"

class RwaSceneAttributeView : public RwaAttributeView
{
    Q_OBJECT
public:
    RwaSceneAttributeView(QWidget *parent, RwaScene *scene);

public slots:
    void updateSceneArea();
    void adaptSize(qint32 width, qint32 height);
    void setCurrentScene(RwaScene *currentScene);

    void receiveEditingFinished();
private slots:
    void receiveCheckBoxAttributeValue(int id, bool value);
    void receiveLineEditAttributeValue(const QString &value);
    void receiveLineEditAttributeValue();
    void receiveComboBoxAttributeValue(QString value);
    void receiveFaderAttributeValue(int id);

signals:
    void sendCurrentSceneRadiusEdited();

private:
   RwaView *mother;
   QList <QComboBox *> visitedSceneComboBoxes;
   qint32 visitedSceneConditionCount;

   void updateSceneComboBox(QComboBox *attrComboBox);
   void updateSceneAttr(QComboBox *attrComboBox, QString scene2compare);
protected:
   QGridLayout *innerLayout;
};

#endif // RWASCENEATTRIBUTEVIEW_H