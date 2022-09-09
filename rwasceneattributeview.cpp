#include "rwasceneattributeview.h"

RwaSceneAttributeView::RwaSceneAttributeView(QWidget *parent, RwaScene *scene) :
    RwaAttributeView(parent, scene)
{
    RwaScene *nextScene;

    setAlignment(Qt::AlignTop);
    visitedSceneConditionCount = 0;

    QStringList areaTypes;
    areaTypes << "Undetermined" << "Circle" << "Rectangle" << "Square" << "Polygon";
    addComboBoxAndLabel(attributeGridLayout, "Area Type", areaTypes);

    QStringList nextScenes;
    nextScenes << "None";
    foreach(nextScene, backend->getScenes())
        nextScenes << QString::fromStdString(nextScene->objectName());

    addComboBoxAndLabel(attributeGridLayout, "Next Scene", nextScenes);
    addLineEditAndLabel(attributeGridLayout, "Time Out");
    QLineEdit *requiredScenes = addLineEditAndLabel(attributeGridLayout, "Required Scenes");
    setLineEditSignal2editingFinished(requiredScenes);

    addLineEditAndLabel(attributeGridLayout, "Level");
    addLineEditAndLabel(attributeGridLayout, "Scene Radius");
    addLineEditAndLabel(attributeGridLayout, "Scene Width");
    addLineEditAndLabel(attributeGridLayout, "Scene Height");
    addLineEditAndLabel(attributeGridLayout, "Exit Offset");

    addAttrCheckbox(attributeGridLayout, "States follow scene", RWASTATEATTRIBUTE_FOLLOWINGASSETS);
    addAttrCheckbox(attributeGridLayout, "Lock Position", RWASTATEATTRIBUTE_LOCKPOSITION);
    addAttrCheckbox(attributeGridLayout, "Disable Fallback", RWASCENEATTRIBUTE_DISABLEFALLBACK);

    //connect(this, SIGNAL(sendCurrentSceneRadiusEdited()), backend, SLOT(receiveCurrentSceneRadiusEdited()));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
              backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    this->setMinimumHeight((assetAttrCounter)*18);
    this->setMinimumWidth(20);
    this->setMaximumWidth(240);
}

void RwaSceneAttributeView::setCurrentScene(RwaScene *scene)
{
    this->currentScene = scene;

    if(!scene)
        return;

    QCheckBox *attrCheckBox = nullptr;
    QComboBox *attrComboBox = nullptr;
    QLineEdit *attrLineEdit = nullptr;

    updateSceneArea();

    attrLineEdit = this->findChild<QLineEdit *>("Required States");
    if(attrLineEdit)
    {
        QString requiredStatesText;
        attrLineEdit->clear();
        foreach(std::string state, currentState->requiredStates)
            requiredStatesText.append(QString::fromStdString(state)).append(", ");

        attrLineEdit->setText(requiredStatesText);
    }

    attrLineEdit = this->findChild<QLineEdit *>("Level");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentScene->getLevel()));

//    attrLineEdit = this->findChild<QLineEdit *>("Enter Offset");
//    if(attrLineEdit)
//        attrLineEdit->setText(QString::number(currentScene->getEnterOffset()));

    attrLineEdit = this->findChild<QLineEdit *>("Exit Offset");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentScene->getExitOffset()));

    attrLineEdit = this->findChild<QLineEdit *>("Time Out");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentScene->getTimeOut()));

    attrCheckBox = this->findChild<QCheckBox *>("States follow scene");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentScene->childrenDoFollowMe());

    attrCheckBox = this->findChild<QCheckBox *>("Lock Position");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentScene->positionIsLocked());

    attrCheckBox = this->findChild<QCheckBox *>("Disable Fallback");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentScene->fallbackDisabled());

    attrComboBox = this->findChild<QComboBox *>("Area Type");
    if(attrComboBox)
        attrComboBox->setCurrentIndex(currentScene->getAreaType());
}

void RwaSceneAttributeView::updateSceneArea()
{
    QLineEdit *attrLineEdit;
    attrLineEdit = this->findChild<QLineEdit *>("Scene Radius");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentScene->getRadius()));

    attrLineEdit = this->findChild<QLineEdit *>("Scene Width");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentScene->getWidth()));

    attrLineEdit = this->findChild<QLineEdit *>("Scene Height");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentScene->getHeight()));
}

void RwaSceneAttributeView::updateSceneAttr(QComboBox *attrComboBox, QString scene2compare)
{
    int index = 0;

    if(scene2compare.compare(""))
         index = attrComboBox->findText(scene2compare);

    attrComboBox->setCurrentIndex(index);
}

void RwaSceneAttributeView::updateSceneComboBox(QComboBox *attrComboBox)
{
    if(!currentScene)
        return;

    RwaScene *scene;

    disconnect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
    attrComboBox->clear();
    attrComboBox->addItem("None");

    foreach(scene, backend->getScenes())
        attrComboBox->addItem(QString::fromStdString(scene->objectName()));

    connect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
}

void RwaSceneAttributeView::receiveCheckBoxAttributeValue(int id, bool value)
{
    QButtonGroup *group = static_cast<QButtonGroup *>(QObject::sender());
    senderName = group->button(id)->text();
    QString boolString = value ? "true" : "false";
    senderValue = boolString;

    if(!currentState)
        return;

    switch(id)
    {
        case RWASTATEATTRIBUTE_FOLLOWINGASSETS:
        {
            currentScene->letChildrenFollowMe(value);
            break;
        }

        case RWASTATEATTRIBUTE_LOCKPOSITION:
        {
            currentScene->lockPosition(value);
            break;
        }

        case RWASCENEATTRIBUTE_DISABLEFALLBACK:
        {
            currentScene->setDisableFallback(value);
            break;
        }

        default:break;
    }
}

void RwaSceneAttributeView::receiveLineEditAttributeValue()
{
    if(!currentState)
        return;

    if(!QObject::sender()->objectName().compare("Required Scenes"))
    {
        QLineEdit *attrLineEdit = (QLineEdit *)QObject::sender();
        QStringList requiredScenes = attrLineEdit->text().split(",",QString::SkipEmptyParts);

        currentScene->requiredScenes.clear();
        QString requiredScene;
        foreach(requiredScene, requiredScenes)
        {
            foreach(RwaScene *scene, backend->getScenes() )
            {
                if(!requiredScene.trimmed().compare(QString::fromStdString(scene->objectName())))
                    currentScene->requiredScenes.push_back(scene->objectName());

            }
        }
    }
}

void RwaSceneAttributeView::receiveEditingFinished()
{
    if(QObject::sender() != this->backend)
    {
        if(senderValue != lastSenderValue)
            emit sendWriteUndo("Scene edited: "+ senderName);
        if(senderValue == lastSenderValue)
        {
            if(senderName != lastSenderName)
                emit sendWriteUndo("Scene edited: "+ senderName);
        }

        lastSenderValue = senderValue;
        lastSenderName = senderName;
        setFocus();
        emit sendCurrentScene(currentScene);
        qDebug();
    }
}

void RwaSceneAttributeView::receiveLineEditAttributeValue(const QString &value)
{
    if(!currentScene)
        return;

    senderName = QObject::sender()->objectName();
    senderValue = value;

    if(!QObject::sender()->objectName().compare("Time Out"))
    {
        currentScene->setTimeOut(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("Scene Radius"))
    {
        currentScene->setRadius(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("Scene Width"))
    {
        currentScene->setWidth(value.toFloat());
        if(currentScene->getAreaType() == RWAAREATYPE_SQUARE)
            currentScene->setHeight(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("Scene Height"))
    {
        currentScene->setHeight(value.toFloat());
        if(currentScene->getAreaType() == RWAAREATYPE_SQUARE)
            currentScene->setWidth(value.toFloat());
    }

//    if(!QObject::sender()->objectName().compare("Enter Offset"))
//    {
//        currentScene->setEnterOffset(value.toFloat());
//    }

    if(!QObject::sender()->objectName().compare("Exit Offset"))
    {
        currentScene->setExitOffset(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("Level"))
    {
        currentScene->setLevel(value.toInt());
    }
}

void RwaSceneAttributeView::receiveComboBoxAttributeValue(int index)
{
    QComboBox *attrComboBox = static_cast<QComboBox *>(QObject::sender());
    QString value = attrComboBox->itemText(index);

    QString nextState = nullptr;
    QString nextScene = nullptr;

    if(!currentScene)
        return;

    senderName = QObject::sender()->objectName();
    senderValue = value;

    if(!QObject::sender()->objectName().compare("Area Type"))
    {
        if(!value.compare("Undetermined"))
            currentScene->setAreaType(RWA_UNDETERMINED);

        if(!value.compare("Circle"))
            currentScene->setAreaType(RWAAREATYPE_CIRCLE);

        if(!value.compare("Rectangle"))
            currentScene->setAreaType(RWAAREATYPE_RECTANGLE);

        if(!value.compare("Square"))
            currentScene->setAreaType(RWAAREATYPE_SQUARE);

        if(!value.compare("Polygon"))
            currentScene->setAreaType(RWAAREATYPE_POLYGON);

        //sendCurrentSceneRadiusEdited();
    }

    if(!QObject::sender()->objectName().compare("Next Scene"))
    {
        attrComboBox = this->findChild<QComboBox *>("Next Scene");

        if(attrComboBox->currentIndex() <= 0)
            currentScene->setNextScene("");
        else
            currentScene->setNextScene(attrComboBox->currentText().toStdString());
    }
}

void RwaSceneAttributeView::receiveComboBoxAttributeValue(QString value)
{
    QString nextState = nullptr;
    QString nextScene = nullptr;

    if(!currentScene)
        return;

    QComboBox *attrComboBox = nullptr;


    if(!QObject::sender()->objectName().compare("Area Type"))
    {
        if(!value.compare("Undetermined"))
            currentScene->setAreaType(RWA_UNDETERMINED);

        if(!value.compare("Circle"))
            currentScene->setAreaType(RWAAREATYPE_CIRCLE);

        if(!value.compare("Rectangle"))
            currentScene->setAreaType(RWAAREATYPE_RECTANGLE);

        if(!value.compare("Square"))
            currentScene->setAreaType(RWAAREATYPE_SQUARE);

        if(!value.compare("Polygon"))
            currentScene->setAreaType(RWAAREATYPE_POLYGON);

        sendCurrentSceneRadiusEdited();
    }

    if(!QObject::sender()->objectName().compare("Next Scene"))
    {
        attrComboBox = this->findChild<QComboBox *>("Next Scene");

        if(attrComboBox->currentIndex() <= 0)
            currentScene->setNextScene("");
        else
            currentScene->setNextScene(attrComboBox->currentText().toStdString());
    }
}

void RwaSceneAttributeView::receiveFaderAttributeValue(int id)
{

}

