#include "rwaassetattributeview.h"

RwaAssetAttributeView::RwaAssetAttributeView(QWidget *parent, RwaScene *scene) :
    RwaAttributeView(parent, scene)
{
    setAlignment(Qt::AlignTop);
    QGridLayout *innerLayout = new QGridLayout(this);

    innerLayout->setVerticalSpacing(2);
    innerLayout->setContentsMargins(0,4,0,4);
    innerLayout->setAlignment(this, Qt::AlignLeft);
    innerLayout->setSpacing(2);

    QStringList playbackTypes;
    playbackTypes << "Undetermined" << "Mono" << "Stereo" << "Auto" << "Binaural-Mono (Legacy)" << "Binaural-Stereo (Legacy)" \
                  << "Binaural-5Channel (Legacy)" << "Binaural-7Channel (Legacy)"<< "Binaural-Mono" << "Binaural-Stereo" << "Binaural-Auto" \
                  << "Binaural-5Channel" << "Binaural-7Channel"  << "Binaural-Space" << "Custom IR-Set 1" << "Custom IR-Set 2" << "Custom IR-Set 3";

    QComboBox *playbackTypesCombo = addComboBoxAndLabel(innerLayout, "Playback Mode", playbackTypes);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(4, true);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(5, true);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(6, true);
    qobject_cast<QListView *>(playbackTypesCombo->view())->setRowHidden(7, true);

    QStringList numberOfReflections;
    numberOfReflections << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << \
                     "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";

    addComboBoxAndLabel(innerLayout, "Reflection Count", numberOfReflections, &reflectionCount, &reflectionCountLabel);
    reflectionCount->hide();
    reflectionCountLabel->hide();

    QStringList dampingFunction;
    dampingFunction << "None" << "Exponential" << "Linear";
    addComboBoxAndLabel(innerLayout, "Damping", dampingFunction);

    QLineEdit *editingFinishedLineEdit;

    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Damping Factor");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Damping Trim");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Damping Min");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Damping Max");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Fade-In Time");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Fade-Out Time");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Crossfade Time");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Offset Time");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Gain");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Channel Radius");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Rotate Offset");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Rotate Frequency");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Moving Speed");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Fixed Orientation");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Fixed Elevation");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Fixed Distance");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "Min Distance");
    editingFinishedLineEdit = addLineEditAndLabel(innerLayout, "libPd Receiver");

    addAttrCheckbox(innerLayout, "Exclusive", RWAASSETATTRIBUTE_ISEXCLUSIVE);
    addAttrCheckbox(innerLayout,"Loop", RWAASSETATTRIBUTE_LOOP);
    addAttrCheckbox(innerLayout, "Stop Loop at End-Position", RWAASSETATTRIBUTE_LOOPUNTILENDPOSITION);
    addAttrCheckbox(innerLayout, "Raw Sensors 2 Pd", RWAASSETATTRIBUTE_RAWSENSORS2PD);
    addAttrCheckbox(innerLayout, "GPS 2 Pd", RWAASSETATTRIBUTE_GPS2PD);
    addAttrCheckbox(innerLayout, "Play only once", RWAASSETATTRIBUTE_PLAYONCE);
    addAttrCheckbox(innerLayout, "Rotate", RWAASSETATTRIBUTE_AUTOROTATE);
    addAttrCheckbox(innerLayout, "Move", RWAASSETATTRIBUTE_AUTOMOVE);
    addAttrCheckbox(innerLayout, "Mute", RWAASSETATTRIBUTE_MUTE);
    addAttrCheckbox(innerLayout, "Headtracker relative 2 source", RWAASSETATTRIBUTE_HEADTRACKERRELATIVE2SOURCE);
    addAttrCheckbox(innerLayout, "Lock Position", RWAASSETATTRIBUTE_LOCKPOSITION);
    addAttrCheckbox(innerLayout, "Allow Channel-Positioning", RWAASSETATTRIBUTE_ALLOWINDIVIDUELLCHANNELPOSITIONS);
    addAttrCheckbox(innerLayout, "Always play from beginning", RWAASSETATTRIBUTE_ALWAYSPLAYFROMBEGINNING);

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
              backend, SLOT(receiveLastTouchedState(RwaState*)));

    connect(this, SIGNAL(sendCurrentAsset(RwaAsset1*)),
              backend, SLOT(receiveLastTouchedAsset(RwaAsset1*)));

    connect(backend, SIGNAL(sendSelectedAssets(QStringList)),
              this, SLOT(receiveSelectedAssets(QStringList)));

    this->setMinimumHeight((assetAttrCounter)*17);
    this->setMinimumWidth(20);
    this->setMaximumWidth(260);
}

void RwaAssetAttributeView::adaptSize(qint32 width, qint32 height)
{
    //qDebug() << "adaptSize";
}

void RwaAssetAttributeView::receiveSelectedAssets(QStringList assets)
{
    selectedAssets = assets;
}

void RwaAssetAttributeView::setCurrentAsset(RwaAsset1 *asset)
{
    if(!asset)
        return;

    lastAsset = currentAsset;
    currentAsset = asset;

    QCheckBox *attrCheckBox = nullptr;
    QComboBox *attrComboBox = nullptr;
    QLineEdit *attrLineEdit = nullptr;

    /*------------------------------- Comboboxes -----------------------------------*/

    attrComboBox = this->findChild<QComboBox *>("Playback Mode");
    if(attrComboBox)
    {
        attrComboBox->setCurrentIndex(asset->getPlaybackType());
        if(asset->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSPACE)
        {
            reflectionCount->show();
            reflectionCountLabel->show();
            scrollArea->update();
        }
        else
        {
            reflectionCount->hide();
            reflectionCountLabel->hide();
            scrollArea->update();
        }
    }

    attrComboBox = this->findChild<QComboBox *>("Reflection Count");
    if(attrComboBox)
        attrComboBox->setCurrentIndex(asset->getReflectionCount());

    attrComboBox = this->findChild<QComboBox *>("Damping");
    if(attrComboBox)
        attrComboBox->setCurrentIndex(asset->getDampingFunction());

     /*------------------------------- Lineedits ----------------------------------*/

    attrLineEdit = this->findChild<QLineEdit *>("Damping Factor");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getDampingFactor()));

    attrLineEdit = this->findChild<QLineEdit *>("Damping Trim");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getDampingTrim()));

    attrLineEdit = this->findChild<QLineEdit *>("Damping Min");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getDampingMin()));

    attrLineEdit = this->findChild<QLineEdit *>("Damping Max");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getDampingMax()));

    attrLineEdit = this->findChild<QLineEdit *>("Fade-In Time");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getFadeInTime()));

    attrLineEdit = this->findChild<QLineEdit *>("Fade-Out Time");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getFadeOutTime()));

    attrLineEdit = this->findChild<QLineEdit *>("Crossfade Time");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getCrossfadeTime()));

    attrLineEdit = this->findChild<QLineEdit *>("Offset Time");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getOffset()));

    attrLineEdit = this->findChild<QLineEdit *>("Gain");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getGain()));

    attrLineEdit = this->findChild<QLineEdit *>("Channel Radius");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getChannelRadius()));

    attrLineEdit = this->findChild<QLineEdit *>("Rotate Offset");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getRotateOffset()));

    attrLineEdit = this->findChild<QLineEdit *>("Rotate Frequency");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getRotateFrequency()));

    attrLineEdit = this->findChild<QLineEdit *>("Moving Speed");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getMovementSpeed()));

    attrLineEdit = this->findChild<QLineEdit *>("Fixed Orientation");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getFixedAzimuth()));

    attrLineEdit = this->findChild<QLineEdit *>("Fixed Elevation");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getFixedElevation()));

    attrLineEdit = this->findChild<QLineEdit *>("Fixed Distance");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getFixedDistance()));

    attrLineEdit = this->findChild<QLineEdit *>("Min Distance");
    if(attrLineEdit)
        attrLineEdit->setText(QString().setNum(asset->getMinDistance()));

    attrLineEdit = this->findChild<QLineEdit *>("libPd Receiver");
    if(attrLineEdit)
        attrLineEdit->setText(QString("NOT IN USE YET"));

    /*------------------------------- Checkboxes -----------------------------------*/

    attrCheckBox = this->findChild<QCheckBox *>("Exclusive");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getIsExclusive());

    attrCheckBox = this->findChild<QCheckBox *>("Loop");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getLoop());

    attrCheckBox = this->findChild<QCheckBox *>("Stop Loop at End-Position");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getLoopUntilEndPosition());

    attrCheckBox = this->findChild<QCheckBox *>("Raw Sensors 2 Pd");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getRawSensors2pd());

    attrCheckBox = this->findChild<QCheckBox *>("GPS 2 Pd");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getGps2pd());

    attrCheckBox = this->findChild<QCheckBox *>("Play only once");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getPlayOnlyOnce());

    attrCheckBox = this->findChild<QCheckBox *>("Rotate");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getAutoRotate());

    attrCheckBox = this->findChild<QCheckBox *>("Move");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getMoveFromStartPosition());

    attrCheckBox = this->findChild<QCheckBox *>("Mute");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getMute());

    attrCheckBox = this->findChild<QCheckBox *>("Headtracker relative 2 source");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getHeadtrackerRelative2Source());

    attrCheckBox = this->findChild<QCheckBox *>("Lock Position");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getLockPosition());

    attrCheckBox = this->findChild<QCheckBox *>("Allow Channel-Positioning");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->individuellChannelPositionsAllowed());

    attrCheckBox = this->findChild<QCheckBox *>("Always play from beginning");
    if(attrCheckBox)
        attrCheckBox->setChecked(asset->getAlwaysPlayFromBeginning());
}

void RwaAssetAttributeView::receiveEditingFinished()
{
    if(QObject::sender() != this->backend)
    {
        if(senderValue != lastSenderValue)
            emit sendWriteUndo("Asset edited: "+ senderName);
        if(senderValue == lastSenderValue)
        {
            if(senderName != lastSenderName)
                emit sendWriteUndo("Asset edited: "+ senderName);
            if(senderName == lastSenderName)
            {
                if(currentAsset != lastAsset)
                    emit sendWriteUndo("Asset edited: "+ senderName);
            }
        }

        lastSenderValue = senderValue;
        lastSenderName = senderName;
        emit sendCurrentState(currentState);
        setFocus();
    }
}

void RwaAssetAttributeView::receiveCheckBoxAttributeValue(int id, bool value)
{
    if(!currentState)
        return;
    if(selectedAssets.empty())
        return;

    QButtonGroup *group = static_cast<QButtonGroup *>(QObject::sender());
    senderName = group->button(id)->text();
    QString boolString = value ? "true" : "false";
    senderValue = boolString;

    RwaAsset1 *asset = nullptr;

    switch(id)
    {
        case RWAASSETATTRIBUTE_ISEXCLUSIVE:
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                {
                    asset->setIsExclusive(value);
                }
           }
           break;
        }

        case RWAASSETATTRIBUTE_RAWSENSORS2PD:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setRawSensors2pd(value);

            }
            break;
        }
        case RWAASSETATTRIBUTE_GPS2PD:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setGps2pd(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_LOOP:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setLoop(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_PLAYONCE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setPlayOnlyOnce(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_AUTOROTATE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setAutoRotate(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_AUTOMOVE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                 {
                    asset->setMoveFromStartPosition(value);
                    if(QObject::sender() != this->backend)
                    {
                        emit sendCurrentState(currentState);
                    }
                 }
            }
            break;
        }

        case RWAASSETATTRIBUTE_LOOPUNTILENDPOSITION:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setLoopUntilEndPosition(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_MUTE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setMute(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_HEADTRACKERRELATIVE2SOURCE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setHeadtrackerRelative2Source(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_LOCKPOSITION:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setLockPosition(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_ALLOWINDIVIDUELLCHANNELPOSITIONS:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setAllowIndividuellChannelPositions(value);
            }
            break;
        }

    case RWAASSETATTRIBUTE_ALWAYSPLAYFROMBEGINNING:
    {
        foreach(QString assetName, selectedAssets)
        {
             asset = currentState->getAsset(assetName.toStdString());
             if(asset)
                asset->setAlwaysPlayFromBeginning(value);
        }
        break;
    }


    default:break;
    }
}

void RwaAssetAttributeView::receiveCheckBoxAttributeValue(int id)
{

    bool value = assetAttributeGroup->button(id)->isChecked();
    if(!currentState)
        return;
    if(selectedAssets.empty())
        return;

    senderName = QObject::sender()->objectName();
    QString boolString = value ? "true" : "false";
    senderValue = boolString;

    RwaAsset1 *asset = nullptr;

    switch(id)
    {
        case RWAASSETATTRIBUTE_ISEXCLUSIVE:
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                {
                    asset->setIsExclusive(value);
                }
           }
           break;
        }

        case RWAASSETATTRIBUTE_RAWSENSORS2PD:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setRawSensors2pd(value);

            }
            break;
        }
        case RWAASSETATTRIBUTE_GPS2PD:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setGps2pd(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_LOOP:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setLoop(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_PLAYONCE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setPlayOnlyOnce(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_AUTOROTATE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setAutoRotate(value);
            }
            break;
        }

        case RWAASSETATTRIBUTE_AUTOMOVE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                 {
                    asset->setMoveFromStartPosition(value);
                    if(QObject::sender() != this->backend)
                    {
                        emit sendCurrentState(currentState);
                    }
                 }
            }
            break;
        }

        case RWAASSETATTRIBUTE_LOOPUNTILENDPOSITION:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setLoopUntilEndPosition(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_MUTE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setMute(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_HEADTRACKERRELATIVE2SOURCE:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setHeadtrackerRelative2Source(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_LOCKPOSITION:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setLockPosition(value);
             }
            break;
        }

        case RWAASSETATTRIBUTE_ALLOWINDIVIDUELLCHANNELPOSITIONS:
        {
            foreach(QString assetName, selectedAssets)
            {
                 asset = currentState->getAsset(assetName.toStdString());
                 if(asset)
                    asset->setAllowIndividuellChannelPositions(value);
            }
            break;
        }

    case RWAASSETATTRIBUTE_ALWAYSPLAYFROMBEGINNING:
    {
        foreach(QString assetName, selectedAssets)
        {
             asset = currentState->getAsset(assetName.toStdString());
             if(asset)
                asset->setAlwaysPlayFromBeginning(value);
        }
        break;
    }


    default:break;
    }

//    if(QObject::sender() != this->backend)
//    {
//        emit sendCurrentState(currentState);
//    }
}

void RwaAssetAttributeView::receiveLineEditAttributeValue(const QString &text)
{
    if(!currentState)
        return;
    if(selectedAssets.empty())
        return;

    RwaAsset1 *asset = nullptr;

    senderName = QObject::sender()->objectName();
    senderValue = text;

    if(!QObject::sender()->objectName().compare("Damping Factor"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setDampingFactor(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("Damping Trim"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setDampingTrim(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("Damping Min"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setDampingMin(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("Damping Max"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setDampingMax(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("Fade-In Time"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setFadeInTime(text.toInt());
       }
    }

    if(!QObject::sender()->objectName().compare("Fade-Out Time"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setFadeOutTime(text.toInt());
       }
    }

    if(!QObject::sender()->objectName().compare("Crossfade Time"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setCrossfadeTime(text.toInt());
       }
    }

    if(!QObject::sender()->objectName().compare("Offset Time"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setOffset(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("Gain"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setGain(text.toFloat());
       }
    }
    if(!QObject::sender()->objectName().compare("Channel Radius"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setChannelRadius(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("Rotate Offset"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setRotateOffset(text.toInt());
       }
    }

    if(!QObject::sender()->objectName().compare("Rotate Frequency"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setRotateFrequency(text.toFloat());
       }
    }
    if(!QObject::sender()->objectName().compare("Moving Speed"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setMovementSpeed(text.toFloat());
       }
    }
    if(!QObject::sender()->objectName().compare("Fixed Orientation"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setFixedAzimuth(text.toFloat());
       }
    }
    if(!QObject::sender()->objectName().compare("Fixed Elevation"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setFixedElevation(text.toFloat());
       }
    }
    if(!QObject::sender()->objectName().compare("Fixed Distance"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setFixedDistance(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("Min Distance"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                asset->setMinDistance(text.toFloat());
       }
    }

    if(!QObject::sender()->objectName().compare("libPd Receiver"))
    {
       foreach(QString assetName, selectedAssets)
       {
            asset = currentState->getAsset(assetName.toStdString());
            if(asset)
                ;//asset->setReceiverPatcher(value.toStdString());
       }
    }
}

void RwaAssetAttributeView::receiveComboBoxAttributeValue(int index)
{
    qDebug();
    QComboBox *box = static_cast<QComboBox *>(QObject::sender());
    QString value = box->itemText(index);

    if(!currentState)
        return;
    if(selectedAssets.empty())
        return;

    senderName = QObject::sender()->objectName();
    senderValue = value;

    RwaAsset1 *asset = nullptr;

    if(!QObject::sender()->objectName().compare("Playback Mode"))
    {
        if(!value.compare("Binaural-Space"))
        {
            reflectionCount->show();
            reflectionCountLabel->show();
            scrollArea->update();
        }
        else
        {
            reflectionCount->hide();
            reflectionCountLabel->hide();
            scrollArea->update();
        }

        if(!value.compare("Undetermined"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWA_UNDETERMINED);
            }
        }

        if(!value.compare("Mono"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_MONO);
           }
        }

        if(!value.compare("Stereo"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_STEREO);
           }
        }

        if(!value.compare("Auto"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_NATIVE);
           }
        }

        if(!value.compare("Binaural-Mono"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                 {
                    asset->setPlaybackType(RWAPLAYBACKTYPE_BINAURALMONO_FABIAN);
                 }
           }
        }

        if(!value.compare("Binaural-Stereo"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN);
           }
        }

        if(!value.compare("Binaural-Auto"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_BINAURALAUTO_FABIAN);
           }
        }

        if(!value.compare("Binaural-5Channel"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN);
           }
        }

        if(!value.compare("Binaural-7Channel"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN);
           }
        }

        if(!value.compare("Binaural-Space"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_BINAURALSPACE);
           }
        }

        if(!value.compare("Custom IR-Set 1"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_CUSTOM1);
           }
        }

        if(!value.compare("Custom IR-Set 2"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_CUSTOM2);
           }
        }

        if(!value.compare("Custom IR-Set 3"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setPlaybackType(RWAPLAYBACKTYPE_CUSTOM3);
           }
        }
    }

    if(!QObject::sender()->objectName().compare("Reflection Count"))
    {
        foreach(QString assetName, selectedAssets)
        {
             asset = currentState->getAsset(assetName.toStdString());
             if(asset)
             {
                 asset->setReflectionCount(reflectionCount->currentIndex());
                 asset->calculateReflectionPositions();
             }
        }
    }

    if(!QObject::sender()->objectName().compare("Damping"))
    {
        if(!value.compare("None"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setDampingFunction(0);
           }
        }

        if(!value.compare("Exponential"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setDampingFunction(1);
           }
        }

        if(!value.compare("Linear"))
        {
           foreach(QString assetName, selectedAssets)
           {
                asset = currentState->getAsset(assetName.toStdString());
                if(asset)
                    asset->setDampingFunction(2);
           }
        }
    }
}

void RwaAssetAttributeView::receiveFaderAttributeValue(int id)
{

}
