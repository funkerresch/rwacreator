#ifndef RWAATTRIBUTEVIEW_H
#define RWAATTRIBUTEVIEW_H

#include "rwatoolbar.h"
#include "rwabackend.h"
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QCheckBox>
#include <QFont>
#include "rwaview.h"
#include <QEvent>

class LineEditAndLabel  : public QObject
{
    Q_OBJECT
    QLineEdit *lineEdit;
    QLabel *label;
    explicit LineEditAndLabel(QWidget* parent);
};

class RwaAttributeView  : public RwaView
{
    Q_OBJECT

public:
    explicit RwaAttributeView(QWidget* parent, RwaScene *scene);
    QScrollArea *scrollArea;

public slots:

    void addComboBoxAndLabel(QGridLayout *layout, QString name, QStringList values, QComboBox **attrComboBox, QLabel **attrLabel);
protected:

    int assetAttrCounter;
    QButtonGroup *assetAttributeGroup;
    QFont attributeFont;
    QFont dynamicAddButtonFont;

    QString senderName, lastSenderName;
    QString senderValue, lastSenderValue;


    //void setCurrentState(RwaState *currentState);
    //void setCurrentScene(RwaScene *currentScene);
    void setLineEditSignal2editingFinished(QLineEdit *attrLineEdit);
    void addAttrCheckbox(QGridLayout *layout, QString name, int type);
    QLineEdit *addLineEditAndLabel(QGridLayout *layout, QString name);
    void addLineEditAndLabel(QGridLayout *layout, QString name, QLineEdit **attrLineEdit, QLabel **attrLabel);
    QComboBox *addComboBoxAndLabel(QGridLayout *layout, QString name, QStringList values);

protected slots:
    virtual void receiveCheckBoxAttributeValue(int id, bool) = 0;
    virtual void receiveLineEditAttributeValue(const QString &text) = 0;
    virtual void receiveComboBoxAttributeValue(int index) = 0;
    virtual void receiveFaderAttributeValue(int id) = 0;
    virtual void receiveEditingFinished() = 0;
};

#endif // RWAATTRIBUTEVIEW_H
