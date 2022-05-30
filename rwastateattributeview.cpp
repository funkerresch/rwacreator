#include "rwastateattributeview.h"

RwaStateAttributeView::RwaStateAttributeView(QWidget *parent, RwaScene *scene) :
    RwaAttributeView(parent, scene)
{
    RwaState *nextState;
    RwaState *hintState;
    RwaScene *nextScene;

    setAlignment(Qt::AlignTop);

    QStringList stateTypes;
    stateTypes << "Undetermined" << "Fallback" << "Background" << "GPS" << "Bluetooth" << "Combined" << "Random GPS" << "Hint" << "Other";
    addComboBoxAndLabel(attributeGridLayout, "State Type", stateTypes);

    QStringList defaultPlaybackTypes;

    defaultPlaybackTypes << "Undetermined" << "Mono" << "Stereo" << "Auto" << "Binaural-Mono (Legacy)" << "Binaural-Stereo (Legacy)" \
                  << "Binaural-5Channel (Legacy)" << "Binaural-7Channel (Legacy)"<< "Binaural-Mono" << "Binaural-Stereo" << "Binaural-Auto" \
                  << "Binaural-5Channel" << "Binaural-7Channel"  << "Binaural-Space" << "Custom IR-Set 1" << "Custom IR-Set 2" << "Custom IR-Set 3";

    QComboBox *playbackTypesCombo = addComboBoxAndLabel(attributeGridLayout, "Default Playback Mode", defaultPlaybackTypes);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(4, true);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(5, true);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(6, true);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(7, true);

    QStringList areaTypes;
    areaTypes << "Undetermined" << "Circle" << "Rectangle" << "Square" << "Polygon";
    addComboBoxAndLabel(attributeGridLayout, "Area Type", areaTypes);

    QStringList nextStates;
    nextStates << "None";
    foreach(nextState, getCurrentScene()->getStates())
        nextStates << QString::fromStdString(nextState->objectName());

    QStringList hintStates;
    hintStates << "None";
    foreach(hintState, getCurrentScene()->getStates())
            hintStates << QString::fromStdString(hintState->objectName());

    QStringList nextScenes;
    nextScenes << "None";
    foreach(nextScene, backend->getScenes())
        nextScenes << QString::fromStdString(nextScene->objectName());

    QLineEdit *editingFinishedLineEdit;

    addComboBoxAndLabel(attributeGridLayout, "Next State", nextStates);
    addComboBoxAndLabel(attributeGridLayout, "Hint State", hintStates);
    addComboBoxAndLabel(attributeGridLayout, "Next Scene", nextScenes);
    addLineEditAndLabel(attributeGridLayout, "Time Out");
    editingFinishedLineEdit = addLineEditAndLabel(attributeGridLayout, "Min stay time");

    QLineEdit *requiredStates = addLineEditAndLabel(attributeGridLayout, "Required States");
    setLineEditSignal2editingFinished(requiredStates);

    editingFinishedLineEdit = addLineEditAndLabel(attributeGridLayout, "Longitude");
    editingFinishedLineEdit = addLineEditAndLabel(attributeGridLayout, "Latitude");
    editingFinishedLineEdit =addLineEditAndLabel(attributeGridLayout, "State Radius");
    editingFinishedLineEdit =addLineEditAndLabel(attributeGridLayout, "State Width");
    editingFinishedLineEdit =addLineEditAndLabel(attributeGridLayout, "State Height");
    addLineEditAndLabel(attributeGridLayout, "Exit Offset");

    addAttrCheckbox(attributeGridLayout, "Assets follow state", RWASTATEATTRIBUTE_FOLLOWINGASSETS);
    addAttrCheckbox(attributeGridLayout, "Enter state only once", RWASTATEATTRIBUTE_ENTERONLYONCE);
    addAttrCheckbox(attributeGridLayout, "Exclusive for one entity", RWASTATEATTRIBUTE_EXCLUSIVE);
    addAttrCheckbox(attributeGridLayout, "Leave after assets finish", RWASTATEATTRIBUTE_LEAVEAFTERASSETSFINISH);
    addAttrCheckbox(attributeGridLayout, "Leave only after assets finish", RWASTATEATTRIBUTE_LEAVEONLYAFTERASSETSFINISH);
    addAttrCheckbox(attributeGridLayout, "Lock Position", RWASTATEATTRIBUTE_LOCKPOSITION);
    addAttrCheckbox(attributeGridLayout, "State within state", RWASTATEATTRIBUTE_STATEWITHINSTATE);

    connect(this, SIGNAL(sendCurrentStateRadiusEdited()), backend, SLOT(receiveCurrentStateRadiusEdited()));

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
              backend, SLOT(receiveLastTouchedState(RwaState*)));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
              backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    connect(backend, SIGNAL(sendSelectedStates(QStringList)),
              this, SLOT(receiveSelectedStates(QStringList)));

    this->setMinimumHeight((assetAttrCounter)*18);
    this->setMinimumWidth(20);
    this->setMaximumWidth(240);
}

void RwaStateAttributeView::receiveEditingFinished()
{
    if(QObject::sender() != this->backend)
    {
        if(senderValue != lastSenderValue)
            emit sendWriteUndo("State edited: "+ senderName);
        if(senderValue == lastSenderValue)
        {
            if(senderName != lastSenderName)
                emit sendWriteUndo("State edited: "+ senderName);
        }

        lastSenderValue = senderValue;
        lastSenderName = senderName;
        setFocus();

        emit sendCurrentScene(currentScene);
    }
}

void RwaStateAttributeView::receiveSelectedStates(QStringList states)
{
    selectedStates = states;
}

void RwaStateAttributeView::setCurrentScene(RwaScene *scene)
{
    this->currentScene = scene;
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

void RwaStateAttributeView::setCurrentState(RwaState *state)
{
    if(!state)
        return;

    this->currentState = state;

    QCheckBox *attrCheckBox = nullptr;
    QComboBox *attrComboBox = nullptr;
    QLineEdit *attrLineEdit = nullptr;

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

//    attrLineEdit = this->findChild<QLineEdit *>("Enter Offset");
//    if(attrLineEdit)
//        attrLineEdit->setText(QString::number(currentState->getEnterOffset()));

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

    //disconnect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
    attrComboBox->clear();
    attrComboBox->addItem("None");

    foreach(state, currentScene->getStates())
        attrComboBox->addItem(QString::fromStdString(state->objectName()));

    //connect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
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

    //disconnect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
    attrComboBox->clear();
    attrComboBox->addItem("None");

    foreach(scene, backend->getScenes())
        attrComboBox->addItem(QString::fromStdString(scene->objectName()));

   // connect(attrComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(receiveComboBoxAttributeValue(QString)));
}

void RwaStateAttributeView::receiveCheckBoxAttributeValue(int id, bool value)
{
    if(!currentScene)
        return;

    if(selectedStates.empty())
        return;

    QButtonGroup *group = static_cast<QButtonGroup *>(QObject::sender());
    senderName = group->button(id)->text();
    QString boolString = value ? "true" : "false";
    senderValue = boolString;

    RwaState *state = nullptr;

    foreach(QString stateName, selectedStates)
    {
        state = currentScene->getState(stateName.toStdString());
        state->setAttribute(id, value);
    }

    return;
}

void RwaStateAttributeView::receiveLineEditAttributeValue()
{
    if(!currentState)
        return;

    if(!QObject::sender()->objectName().compare("Required States"))
    {
        lastSenderValue = "";
        lastSenderName = senderName;

        QLineEdit *attrLineEdit = (QLineEdit *)QObject::sender();
        QStringList requiredStates = attrLineEdit->text().split(",",QString::SkipEmptyParts);

        currentState->requiredStates.clear();
        QString requiredState;

        foreach(requiredState, requiredStates)
        {
            foreach(RwaState *state, currentScene->getStates() )
            {
                if(!requiredState.trimmed().compare(QString::fromStdString(state->objectName())))
                {
                    currentState->requiredStates.push_back(state->objectName());
                }
            }
        }

        emit sendWriteUndo("State edited: "+ senderName);
        disconnect(attrLineEdit, SIGNAL(editingFinished()) , this, SLOT(receiveLineEditAttributeValue()));
        setFocus();
        connect(attrLineEdit, SIGNAL(editingFinished()) , this, SLOT(receiveLineEditAttributeValue()));
    }
}

void RwaStateAttributeView::receiveLineEditAttributeValue(const QString &value)
{
    if(!currentState)
        return;

    senderName = QObject::sender()->objectName();
    senderValue = value;

    if(!QObject::sender()->objectName().compare("Time Out"))
    {
        currentState->setTimeOut(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("Min stay time"))
    {
        currentState->setMinimumStayTime(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("State Radius"))
    {
        currentState->setRadius(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("Longitude"))
    {
        std::vector<double> coordinates = currentState->getCoordinates();
        coordinates[0] = (value.toDouble());
        currentState->setCoordinates(coordinates);
    }

    if(!QObject::sender()->objectName().compare("Latitude"))
    {
        std::vector<double> coordinates = currentState->getCoordinates();
        coordinates[1] = (value.toDouble());
        currentState->setCoordinates(coordinates);
    }

    if(!QObject::sender()->objectName().compare("State Width"))
    {
        currentState->setWidth(value.toFloat());
        if(currentState->getAreaType() == RWAAREATYPE_SQUARE)
            currentState->setHeight(value.toFloat());
    }

    if(!QObject::sender()->objectName().compare("State Height"))
    {
        currentState->setHeight(value.toFloat());
        if(currentState->getAreaType() == RWAAREATYPE_SQUARE)
            currentState->setWidth(value.toFloat());
    }

//    if(!QObject::sender()->objectName().compare("Enter Offset"))
//    {
//        currentState->setEnterOffset(value.toFloat());
//    }

    if(!QObject::sender()->objectName().compare("Exit Offset"))
    {
        currentState->setExitOffset(value.toFloat());
    }
}

void RwaStateAttributeView::receiveComboBoxAttributeValue(int index)
{
    QComboBox *attrComboBox = static_cast<QComboBox *>(QObject::sender());
    QString value = attrComboBox->itemText(index);

    senderName = QObject::sender()->objectName();
    senderValue = value;

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

    if(!QObject::sender()->objectName().compare("Default Playback Mode"))
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
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURALMONO_FABIAN);

        if(!value.compare("Binaural-Stereo"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN);

        if(!value.compare("Binaural-Auto"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURALAUTO_FABIAN);

        if(!value.compare("Binaural-5Channel"))
            currentState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN);
    }
}

void RwaStateAttributeView::receiveFaderAttributeValue(int id)
{

}

