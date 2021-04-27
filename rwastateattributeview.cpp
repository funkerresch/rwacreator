#include "rwastateattributeview.h"



RwaStateAttributeView::RwaStateAttributeView(QWidget *parent, RwaScene *scene) :
    RwaAttributeView(parent, scene)
{
    RwaState *nextState;
    RwaState *hintState;
    RwaScene *nextScene;

    mother = (RwaView *)parent;
    innerLayout = new QGridLayout(this);
    innerLayout->setVerticalSpacing(2);
    innerLayout->setContentsMargins(0,4,0,4);
    innerLayout->setAlignment(this, Qt::AlignLeft);

    visitedStateConditionCount = 0;

    QStringList stateTypes;
    stateTypes << "Undetermined" << "Fallback" << "Background" << "GPS" << "Bluetooth" << "Combined" << "Random GPS" << "Hint" << "Other";
    addComboBoxAndLabel(innerLayout, "State Type", stateTypes);

    QStringList defaultPlaybackTypes;
    defaultPlaybackTypes << "Undetermined" << "Mono" << "Stereo" << "Auto" << "Binaural-Mono" << "Binaural-Stereo" << "Binaural-Auto" << "Binaural-5Channel";
    addComboBoxAndLabel(innerLayout, "Default Playback Mode", defaultPlaybackTypes);

    QStringList areaTypes;
    areaTypes << "Undetermined" << "Circle" << "Rectangle" << "Square" << "Polygon";
    addComboBoxAndLabel(innerLayout, "Area Type", areaTypes);

    QStringList nextStates;
    nextStates << "None";
    foreach(nextState, mother->getCurrentScene()->getStates())
        nextStates << QString::fromStdString(nextState->objectName());

    QStringList hintStates;
    hintStates << "None";
    foreach(hintState, mother->getCurrentScene()->getStates())
            hintStates << QString::fromStdString(hintState->objectName());

    QStringList nextScenes;
    nextScenes << "None";
    foreach(nextScene, backend->getScenes())
        nextScenes << QString::fromStdString(nextScene->objectName());

    QLineEdit *editingFinishedLineEdit;

    addComboBoxAndLabel(innerLayout, "Next State", nextStates);
    addComboBoxAndLabel(innerLayout, "Hint State", hintStates);
    addComboBoxAndLabel(innerLayout, "Next Scene", nextScenes);
    addLineEditAndLabel(innerLayout, "Time Out");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Min stay time");

    QLineEdit *requiredStates = addLineEditAndLabel(innerLayout, "Required States");
    setLineEditSignal2editingFinished(requiredStates);

    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Longitude");
    connect(editingFinishedLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Latitude");
    connect(editingFinishedLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));
    editingFinishedLineEdit =addLineEditAndLabel(innerLayout, "State Radius");
    connect(editingFinishedLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));
    editingFinishedLineEdit =addLineEditAndLabel(innerLayout, "State Width");
    connect(editingFinishedLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));
    editingFinishedLineEdit =addLineEditAndLabel(innerLayout, "State Height");
    connect(editingFinishedLineEdit, SIGNAL(editingFinished()), this, SLOT(receiveEditingFinished()));
    addLineEditAndLabel(innerLayout, "Enter Offset");
    addLineEditAndLabel(innerLayout, "Exit Offset");

    addAttrCheckbox(innerLayout, "Assets follow state", RWASTATEATTRIBUTE_FOLLOWINGASSETS);
    addAttrCheckbox(innerLayout, "Enter state only once", RWASTATEATTRIBUTE_ENTERONLYONCE);
    addAttrCheckbox(innerLayout, "Exclusive for one entity", RWASTATEATTRIBUTE_EXCLUSIVE);
    addAttrCheckbox(innerLayout, "Leave after assets finish", RWASTATEATTRIBUTE_LEAVEAFTERASSETSFINISH);
    addAttrCheckbox(innerLayout, "Leave only after assets finish", RWASTATEATTRIBUTE_LEAVEONLYAFTERASSETSFINISH);
    addAttrCheckbox(innerLayout, "Lock Position", RWASTATEATTRIBUTE_LOCKPOSITION);
    addAttrCheckbox(innerLayout, "State within state", RWASTATEATTRIBUTE_STATEWITHINSTATE);

    connect(this, SIGNAL(sendCurrentStateRadiusEdited()), backend, SLOT(receiveCurrentStateRadiusEdited()));
   // connect(backend, SIGNAL(sendCurrentStateRadiusEdited()),
     //        this, SLOT(updateStateArea()));

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
              backend, SLOT(receiveLastTouchedState(RwaState*)));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
              backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    this->setMinimumHeight((assetAttrCounter)*16);
   // this->setMaximumHeight((assetAttrCounter)*16);
    this->setMinimumWidth(20);
    this->setMaximumWidth(240);
}

void RwaStateAttributeView::receiveEditingFinished()
{
    if(QObject::sender() != this->backend)
        emit sendCurrentScene(currentScene);
}

void RwaStateAttributeView::setCurrentScene(RwaScene *currentScene)
{
    this->currentScene = currentScene;
}

void RwaStateAttributeView::updateStateArea()
{
    QLineEdit *attrLineEdit;

    attrLineEdit = this->findChild<QLineEdit *>("Longitude");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getCoordinates()[0], 'g', 10));

    attrLineEdit = this->findChild<QLineEdit *>("Latitude");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getCoordinates()[1], 'g', 10));

    attrLineEdit = this->findChild<QLineEdit *>("State Radius");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getRadius()));


    attrLineEdit = this->findChild<QLineEdit *>("State Width");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getWidth()));

    attrLineEdit = this->findChild<QLineEdit *>("State Height");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getHeight()));
}

void RwaStateAttributeView::setCurrentState(RwaState *currentState)
{
    //qDebug() << ":StateAttributeView: SET CURRENT STATE";
    this->currentState = currentState;

    if(!currentState)
        return;

    QCheckBox *attrCheckBox = NULL;
    QComboBox *attrComboBox = NULL;
    QLineEdit *attrLineEdit = NULL;

    updateStateArea();

    attrComboBox = this->findChild<QComboBox *>("Next State");
    updateStateComboBox(attrComboBox);
    updateStateAttr(attrComboBox, QString::fromStdString(currentState->getNextState()));

    attrComboBox = this->findChild<QComboBox *>("Hint State");
    updateStateComboBox(attrComboBox);
    updateStateAttr(attrComboBox, QString::fromStdString(currentState->getHintState()));

    attrComboBox = this->findChild<QComboBox *>("Next Scene");
    updateSceneComboBox(attrComboBox);
    updateSceneAttr(attrComboBox, QString::fromStdString(currentState->getNextScene()));

    attrLineEdit = this->findChild<QLineEdit *>("Required States");
    if(attrLineEdit)
    {
        QString requiredStatesText;
        attrLineEdit->clear();
        foreach(std::string state, currentState->requiredStates)
            requiredStatesText.append(QString::fromStdString(state)).append(", ");

        attrLineEdit->setText(requiredStatesText);
    }

    attrLineEdit = this->findChild<QLineEdit *>("Enter Offset");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getEnterOffset()));

    attrLineEdit = this->findChild<QLineEdit *>("Exit Offset");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getExitOffset()));

    attrLineEdit = this->findChild<QLineEdit *>("Time Out");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getTimeOut()));

    attrLineEdit = this->findChild<QLineEdit *>("Min stay time");
    if(attrLineEdit)
        attrLineEdit->setText(QString::number(currentState->getMinimumStayTime()));

    attrCheckBox = this->findChild<QCheckBox *>("Assets follow state");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentState->childrenDoFollowMe());

    attrCheckBox = this->findChild<QCheckBox *>("Enter state only once");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentState->getEnterOnlyOnce());

    attrCheckBox = this->findChild<QCheckBox *>("Leave after assets finish");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentState->getLeaveAfterAssetsFinish());

    attrCheckBox = this->findChild<QCheckBox *>("Leave only after assets finish");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentState->getLeaveOnlyAfterAssetsFinish());

    attrCheckBox = this->findChild<QCheckBox *>("Lock Position");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentState->positionIsLocked());

    attrCheckBox = this->findChild<QCheckBox *>("State within state");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentState->stateWithinState);

    attrCheckBox = this->findChild<QCheckBox *>("Exclusive for one entity");
    if(attrCheckBox)
        attrCheckBox->setChecked(currentState->getIsExclusive());

    attrComboBox = this->findChild<QComboBox *>("State Type");
    if(attrComboBox)
        attrComboBox->setCurrentIndex(currentState->getType());

    attrComboBox = this->findChild<QComboBox *>("Area Type");
    if(attrComboBox)
        attrComboBox->setCurrentIndex(currentState->getAreaType());

    attrComboBox = this->findChild<QComboBox *>("Default Playback Mode");
    if(attrComboBox)
        attrComboBox->setCurrentIndex(currentState->getDefaultPlaybackType());



    /*if(!(QObject::sender() == this->backend))
    {
        emit sendCurrentState(currentState);
    }*/
}

void RwaStateAttributeView::updateStateAttr(QComboBox *attrComboBox, QString state2compare)
{
    int index = 0;

    if(state2compare.compare(""))   
         index = attrComboBox->findText(state2compare);

    attrComboBox->setCurrentIndex(index);
}

void RwaStateAttributeView::updateStateComboBox(QComboBox *attrComboBox)
{
    if(!currentState)
        return;

    RwaState *state;

    disconnect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
    attrComboBox->clear();
    attrComboBox->addItem("None");

    foreach(state, currentScene->getStates())
        attrComboBox->addItem(QString::fromStdString(state->objectName()));

    connect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
}

void RwaStateAttributeView::updateSceneAttr(QComboBox *attrComboBox, QString scene2compare)
{
    int index = 0;

    if(scene2compare.compare(""))   
       index = attrComboBox->findText(scene2compare);

    attrComboBox->setCurrentIndex(index);
}

void RwaStateAttributeView::updateSceneComboBox(QComboBox *attrComboBox)
{
    RwaScene *scene;

    disconnect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
    attrComboBox->clear();
    attrComboBox->addItem("None");

    foreach(scene, backend->getScenes())
        attrComboBox->addItem(QString::fromStdString(scene->objectName()));

    connect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
}

void RwaStateAttributeView::receiveCheckBoxAttributeValue(int id, bool value)
{
    if(!currentState)
        return;

    switch(id)
    {
        case RWASTATEATTRIBUTE_FOLLOWINGASSETS:
        {
            currentState->letChildrenFollowMe(value);
            break;
        }

        case RWASTATEATTRIBUTE_ENTERONLYONCE:
        {
            currentState->setEnterOnlyOnce(value);
            break;
        }

        case RWASTATEATTRIBUTE_EXCLUSIVE:
        {
            currentState->setIsExclusive(value);
            break;
        }

        case RWASTATEATTRIBUTE_LEAVEAFTERASSETSFINISH:
        {
            currentState->setLeaveAfterAssetsFinish(value);
            break;
        }
        case RWASTATEATTRIBUTE_LEAVEONLYAFTERASSETSFINISH:
        {
            currentState->setLeaveOnlyAfterAssetsFinish(value);
            break;
        }

        case RWASTATEATTRIBUTE_LOCKPOSITION:
        {
            currentState->lockPosition(value);
            break;
        }

        case RWASTATEATTRIBUTE_STATEWITHINSTATE:
        {
            currentState->stateWithinState = value;
            break;
        }

        default:break;
    }
}

void RwaStateAttributeView::receiveLineEditAttributeValue()
{
    if(!currentState)
        return;

    if(!QObject::sender()->objectName().compare("Required States"))
    {
        QLineEdit *attrLineEdit = (QLineEdit *)QObject::sender();
        QStringList requiredStates = attrLineEdit->text().split(",",QString::SkipEmptyParts);

        currentState->requiredStates.clear();
        QString requiredState;
        foreach(requiredState, requiredStates)
        {
            foreach(RwaState *state, currentScene->getStates() )
            {
                if(!requiredState.trimmed().compare(QString::fromStdString(state->objectName())))
                    currentState->requiredStates.push_back(state->objectName());
            }
        }
    }
}

void RwaStateAttributeView::receiveLineEditAttributeValue(const QString &value)
{
    if(!currentState)
        return;

    if(!QObject::sender()->objectName().compare("Time Out"))
    {
        currentState->setTimeOut(value.toFloat());
        //setCurrentState(currentState);
        //does not need any redraw
    }

    if(!QObject::sender()->objectName().compare("Min stay time"))
    {
        currentState->setMinimumStayTime(value.toFloat());
        //setCurrentState(currentState);
        //does not need any redraw
    }

    if(!QObject::sender()->objectName().compare("State Radius"))
    {
        currentState->setRadius(value.toFloat());
        //setCurrentState(currentState);
    }

    if(!QObject::sender()->objectName().compare("Longitude"))
    {
        std::vector<double> coordinates = currentState->getCoordinates();
        coordinates[0] = (value.toDouble());
        currentState->setCoordinates(coordinates);
        //setCurrentState(currentState);
    }

    if(!QObject::sender()->objectName().compare("Latitude"))
    {
        std::vector<double> coordinates = currentState->getCoordinates();
        coordinates[1] = (value.toDouble());
        currentState->setCoordinates(coordinates);
        //setCurrentState(currentState);
    }

    if(!QObject::sender()->objectName().compare("State Width"))
    {
        currentState->setWidth(value.toFloat());
        if(currentState->getAreaType() == RWAAREATYPE_SQUARE)
            currentState->setHeight(value.toFloat());
        //setCurrentState(currentState);
    }

    if(!QObject::sender()->objectName().compare("State Height"))
    {
        currentState->setHeight(value.toFloat());
        if(currentState->getAreaType() == RWAAREATYPE_SQUARE)
            currentState->setWidth(value.toFloat());
        //setCurrentState(currentState);
    }

    if(!QObject::sender()->objectName().compare("Enter Offset"))
    {
        currentState->setEnterOffset(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("Exit Offset"))
    {
        currentState->setExitOffset(value.toFloat());
    }
}

void RwaStateAttributeView::receiveComboBoxAttributeValue(QString value)
{
    QString nextState = NULL;
    QString nextScene = NULL;

    if(!currentState)
        return;

    QComboBox *attrComboBox = NULL;

    if(!QObject::sender()->objectName().compare("State Type"))
    {
        if(!value.compare("Undetermined"))
            currentState->setType(RWA_UNDETERMINED);

        if(!value.compare("Fallback"))
            currentState->setType(RWASTATETYPE_FALLBACK);

        if(!value.compare("Background"))
            currentState->setType(RWASTATETYPE_BACKGROUND);

        if(!value.compare("GPS"))
            currentState->setType(RWASTATETYPE_GPS);

        if(!value.compare("Bluetooth"))
            currentState->setType(RWASTATETYPE_BLUETOOTH);

        if(!value.compare("Combined"))
            currentState->setType(RWASTATETYPE_COMBINED);

        if(!value.compare("Random GPS"))
            currentState->setType(RWASTATETYPE_RANDOMGPS);

        if(!value.compare("Hint"))
            currentState->setType(RWASTATETYPE_HINT);

        if(!value.compare("Other"))
            currentState->setType(RWASTATETYPE_OTHER);
    }

    if(!QObject::sender()->objectName().compare("Area Type"))
    {
        if(!value.compare("Undetermined"))
            currentState->setAreaType(RWA_UNDETERMINED);

        if(!value.compare("Circle"))
            currentState->setAreaType(RWAAREATYPE_CIRCLE);

        if(!value.compare("Rectangle"))
            currentState->setAreaType(RWAAREATYPE_RECTANGLE);

        if(!value.compare("Square"))
            currentState->setAreaType(RWAAREATYPE_SQUARE);

        if(!value.compare("Polygon"))
            currentState->setAreaType(RWAAREATYPE_POLYGON);

        sendCurrentStateRadiusEdited();
    }

    if(!QObject::sender()->objectName().compare("Next State"))
    {
        attrComboBox = this->findChild<QComboBox *>("Next State");

        if(attrComboBox->currentIndex() <= 0)
            currentState->setNextState("");
        else
            currentState->setNextState(attrComboBox->currentText().toStdString());
    }

    if(!QObject::sender()->objectName().compare("Hint State"))
    {
        attrComboBox = this->findChild<QComboBox *>("Hint State");

        if(attrComboBox->currentIndex() <= 0)
            currentState->setHintState("");
        else
            currentState->setHintState(attrComboBox->currentText().toStdString());
    }

    if(!QObject::sender()->objectName().compare("Next Scene"))
    {
        attrComboBox = this->findChild<QComboBox *>("Next Scene");

        if(attrComboBox->currentIndex() <= 0)
            currentState->setNextScene("");
        else
            currentState->setNextScene(attrComboBox->currentText().toStdString());
    }

    if(!QObject::sender()->objectName().compare("Playback Mode"))
    {
        if(!value.compare("Undetermined"))
            currentState->setDefaultPlaybackType(RWA_UNDETERMINED);

        if(!value.compare("Mono"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_MONO);

        if(!value.compare("Stereo"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_STEREO);

        if(!value.compare("Auto"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_NATIVE);

        if(!value.compare("Binaural-Mono"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURALMONO);

        if(!value.compare("Binaural-Stereo"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURALSTEREO);

        if(!value.compare("Binaural-Auto"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURALAUTO);

        if(!value.compare("Binaural-5Channel"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURAL5CHANNEL);
    }
}

void RwaStateAttributeView::receiveFaderAttributeValue(int id)
{

}

void RwaStateAttributeView::adaptSize(qint32 width, qint32 height)
{
   qDebug() << height;

}
