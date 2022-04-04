#include <QtWidgets>
#include "rwaimport.h"

RwaImport::RwaImport(QList<RwaScene *> *scenes, QString projectPath)
{
    this->backend = RwaBackend::getInstance();
    this->currentScene = nullptr;
    this->projectPath = projectPath;
    this->scenes = scenes;
}

RwaImport::RwaImport(std::list<RwaScene *> &scenes, QString projectPath)
{
    this->backend = RwaBackend::getInstance();
    this->currentScene = nullptr;
    this->projectPath = projectPath;
    this->scenes = new QList<RwaScene *>(QList<RwaScene *>::fromStdList( scenes));
}

QString RwaImport::readRwaInit()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "rwa");

    QString name;

    while (!xml.atEnd())
    {
        xml.readNext();
        if(xml.isStartElement())
        {
            if(xml.name() == "currentproject")
            {
                name =  xml.attributes().value("name").toString();
                return name;
            }
        }
    }
    if (xml.hasError())
          qDebug() << "Error in XML file.";

    return QString();
}

QString RwaImport::readinit(QIODevice *device)
{
    xml.setDevice(device);
    QString currentProject = "";
    if (xml.readNextStartElement()) {
        if (xml.name() == "rwa" && xml.attributes().value("version") == "1.0")
        {
            qDebug() << "Found Init File";
            currentProject = readRwaInit();
        }
        else
            xml.raiseError(QObject::tr("The file is not an RWA version 1.0 file."));
    }

    return currentProject;
}

bool RwaImport::read(QIODevice *device)
{
    xml.setDevice(device);

    if (xml.readNextStartElement()) {
        if (xml.name() == "rwa" && xml.attributes().value("version") == "1.0")
        {
            scenes->clear();
            readRwa();
        }
        else
            xml.raiseError(QObject::tr("The file is not an RWA version 1.0 file."));
    }

    return !xml.error();
}

QString RwaImport::errorString() const
{
    return QObject::tr("%1\nLine %2, column %3")
            .arg(xml.errorString())
            .arg(xml.lineNumber())
            .arg(xml.columnNumber());
}

void RwaImport::readRwa()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "rwa");

    std::vector<double> gps(2, 0.0);
    QString name;
    QString currentState;
    double lon;
    double lat;
    qint32 zoom;
    int areaType = RWA_UNDETERMINED;
    QString currentSceneName;
    bool hasFallback = false;
    bool hasBackground = false;

    while (!xml.atEnd())
    {
        xml.readNext();

        if(xml.isStartElement())
        {
            if(xml.name() == "game")
            {
                if(xml.attributes().hasAttribute("currentscene"))
                {
                    currentSceneName = xml.attributes().value("currentscene").toString();
                }
            }

            if(xml.name() == "scene")
            {
                hasFallback = false;
                hasBackground = false;

                states.clear();
                findSurroundingSceneArea = false;
                name =  xml.attributes().value("name").toString();
                lon = xml.attributes().value("lon").toDouble();
                lat = xml.attributes().value("lat").toDouble();
                zoom = xml.attributes().value("zoom").toInt();
                gps[0] = lon;
                gps[1] = lat;
                RwaScene *scene = new RwaScene(name.toStdString(), gps, zoom);
                scenes->append(scene);
                currentScene = scene;

                if(xml.attributes().hasAttribute("currentstate"))
                {
                    currentState = xml.attributes().value("currentstate").toString();
                }

                if(xml.attributes().hasAttribute("areatype"))
                {
                    areaType = xml.attributes().value("areatype").toInt();
                }
                else
                    findSurroundingSceneArea = true;

                if(xml.attributes().hasAttribute("radius"))
                {
                    scene->setRadius(xml.attributes().value("radius").toInt());
                }

                if(xml.attributes().hasAttribute("width"))
                {
                   scene->setWidth(xml.attributes().value("width").toInt());
                }

                if(xml.attributes().hasAttribute("height"))
                {
                    scene->setHeight(xml.attributes().value("height").toInt());
                }

                if(xml.attributes().hasAttribute("exitoffset"))
                {
                    scene->setExitOffset(xml.attributes().value("exitoffset").toInt());
                }

                if(xml.attributes().hasAttribute("minstaytime"))
                {
                    scene->setMinimumStayTime(xml.attributes().value("minstaytime").toFloat());
                }

                if(xml.attributes().hasAttribute("fallbackdisabled"))
                {
                    scene->setDisableFallback(xml.attributes().value("fallbackdisabled").toInt());
                }

                if(xml.attributes().hasAttribute("level"))
                {
                    scene->setLevel(xml.attributes().value("level").toInt());
                }
                else
                    scene->setLevel(backend->getScenes().count()-1);

                readSceneCorners();
                readState();

                if(findSurroundingSceneArea)
                {
                    scene->findStateSurroundingArea();
                    scene->setAreaType(RWAAREATYPE_RECTANGLE);
                }
                else
                    scene->setAreaType(areaType);

                if(!scene->getStates().empty())
                {
                    scene->getStates().front()->isGpsState = false;
                }

                foreach(RwaState *state, scene->getStates())
                {
                    if(state->objectName() == "FALLBACK")
                        hasFallback = true;
                    else if(state->objectName() == "BACKGROUND")
                         hasBackground = true;
                }
                if(!hasFallback)
                    scene->InsertDefaultFallbackState();
                if(!hasBackground)
                    scene->InsertDefaultBackgroundState();

                if(currentState != "")
                {
                    scene->setCurrentState(currentState.toStdString());
                }
            }
        }
    }
    if (xml.hasError())
          qDebug() << "Error in XML file.";

    if(currentSceneName == "")
        backend->receiveLastTouchedScene(backend->getScenes().first());
    else
        backend->receiveLastTouchedScene(backend->getScene(currentSceneName));
}

void RwaImport::readActions()
{
    QString nextState = "";
    QString hintState = "";
    QString nextScene = "";

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "nextstate")
            {
                nextState = xml.readElementText();
                if(nextState.compare(""))
                    currentState->setNextState(nextState.toStdString());
            }

            if (xml.name() == "hintstate")
            {
                hintState = xml.readElementText();
                if(hintState.compare(""))
                    currentState->setHintState(hintState.toStdString());
            }

            if (xml.name() == "nextscene")
            {
                nextScene = xml.readElementText();
                if(nextScene.compare(""))
                    currentState->setNextScene(nextScene.toStdString());
            }
        }

        if(xml.isEndElement() && xml.name() == "actions")
            break;

        xml.readNext();
    }
}

void RwaImport::readState()
{
    QString name;
    bool existsAlready = false;
    int stateListIndex = 0;

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "state")
            {
                existsAlready = false;
                name = xml.attributes().value("name").toString();
                foreach(QString stateName, states) // for broken games with doppelgänger states
                {
                    if(stateName == name)
                        existsAlready = true;
                }

                if(!existsAlready)
                {
                    states.append(name);

                    RwaState *state = new RwaState(name.toStdString());
                    state->setScene(currentScene);
                    state->setType(xml.attributes().value("type").toInt());

                    if(xml.attributes().hasAttribute(QString("zoom")))
                        state->setZoom(xml.attributes().value("zoom").toInt());

                    currentStateAreaType = xml.attributes().value("areatype").toInt();
                    qint32 defaultPlaybackType = xml.attributes().value("defaultplaybacktype").toInt();
                    if(defaultPlaybackType == RWAPLAYBACKTYPE_BINAURALMONO)
                        defaultPlaybackType = RWAPLAYBACKTYPE_BINAURALMONO_FABIAN;
                    if(defaultPlaybackType == RWAPLAYBACKTYPE_BINAURALSTEREO)
                        defaultPlaybackType = RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN;

                    state->setDefaultPlaybackType(defaultPlaybackType);
                    state->letChildrenFollowMe(xml.attributes().value("assetsfollowstate").toInt());
                    state->setEnterOnlyOnce(xml.attributes().value("enteronlyonce").toInt());
                    if(xml.attributes().hasAttribute("lockposition"))
                        state->lockPosition(xml.attributes().value("lockposition").toInt());
                    if(xml.attributes().hasAttribute("statewithinstate"))
                        state->stateWithinState = (xml.attributes().value("statewithinstate").toInt());
                    if(xml.attributes().hasAttribute("minstaytime"))
                        state->setMinimumStayTime(xml.attributes().value("minstaytime").toFloat());
                    state->setLeaveAfterAssetsFinish(xml.attributes().value("leaveafterassetsfinish").toInt());
                    state->setLeaveOnlyAfterAssetsFinish(xml.attributes().value("leaveonlyafterassetsfinish").toInt());
                    state->setTimeOut(xml.attributes().value("timeout").toInt());

                    currentState = state;

                    readEnterconditions();
                    readActions();
                    state->setAreaType(currentStateAreaType);
                    if(!name.compare("FALLBACK"))
                         currentState->setMinimumStayTime(0);

                   /* if(!name.compare("FALLBACK") || !name.compare("BACKGROUND"))
                    {
                       // qDebug() << name;
                        state->setCoordinates(currentScene->getCoordinates());
                        state->isGpsState = false;
                    }*/

                    readAssets();
                    if(state->objectName() == "FALLBACK" && stateListIndex != 0)
                        currentScene->InsertStateAtIndex(state, 0);
                    else if(state->objectName() == "BACKGROUND" && stateListIndex != 1)
                        currentScene->InsertStateAtIndex(state, 1);
                    else
                    currentScene->addState(state);

                    stateListIndex++;
                }
            }
        }

        if(xml.isEndElement() && xml.name() == "scene")
            break;

        xml.readNext();
    }
}

void RwaImport::readSceneCornerLonAndLat()
{
    vector<double> corner(2, 0.0);

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "lon")
                 corner[0] = (xml.readElementText().toDouble());
            if (xml.name() == "lat")
            {
                 corner[1] = (xml.readElementText().toDouble());
                 currentScene->corners.push_back(corner);
            }
        }
        xml.readNext();

        if(xml.isEndElement() && xml.name() == "corners")
            break;
    }
}

void RwaImport::readSceneCorners()
{
    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "corners")
            {
                readSceneCornerLonAndLat();
                return;
            }

            if (xml.name() == "state")
            {
                return;
            }
        }

        xml.readNext();
    }
}

void RwaImport::readReflectionPositions(RwaAsset1 *asset)
{
    double lon = 0, lat = 0;
    std::vector<double> tmp(2, 0.0);

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            for(int i = 0;i < asset->getReflectionCount(); i++)
            {
                QString reflection = QString("reflection%1").arg(i);
                if(xml.name() == reflection)
                {
                    if(xml.attributes().hasAttribute("lon"))
                        lon = xml.attributes().value("lon").toDouble();
                    if(xml.attributes().hasAttribute("lat"))
                        lat = xml.attributes().value("lat").toDouble();

                    tmp[0] = lon;
                    tmp[1] = lat;
                    asset->setReflectionCoordinate(i, tmp);
                    asset->reflectionCoordinateIsSet[i] = true;
                    //qDebug() << asset->channelcoordinates[i].x();
                    break;
                }
            }
        }

        xml.readNext();

        if(xml.isEndElement() && xml.name() == "asset")
            break;
    }
}

void RwaImport::readChannelPositions(RwaAsset1 *asset)
{
    double lon = 0, lat = 0;
    std::vector<double> tmp(2, 0.0);

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            for(int i = 0;i < 64; i++)
            {
                QString channel = QString("channel%1").arg(i);
                if(xml.name() == channel)
                {
                    if(xml.attributes().hasAttribute("lon"))
                        lon = xml.attributes().value("lon").toDouble();
                    if(xml.attributes().hasAttribute("lat"))
                        lat = xml.attributes().value("lat").toDouble();

                    tmp[0] = lon;
                    tmp[1] = lat;
                    asset->setChannelCoordinate(i, tmp);
                    asset->individuellChannelPosition[i] = true;
                    //qDebug() << asset->channelcoordinates[i].x();
                    break;
                }
            }
        }

        xml.readNext();

        if(xml.isEndElement() && xml.name() == "channelpositions")
            break;
    }
}

void RwaImport::readCorners()
{
    std::vector<double> corner(2, 0.0);

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "lon")
                corner[0] = (xml.readElementText().toDouble());
            if (xml.name() == "lat")
            {
                corner[1] = (xml.readElementText().toDouble());
                currentState->corners.push_back(corner);
            }
        }

        xml.readNext();

        if(xml.isEndElement() && xml.name() == "corners")
            break;
    }
}

void RwaImport::readRequiredStates()
{
    QString requiredState;

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "requiredstate")
            {
                requiredState = xml.readElementText();
                currentState->requiredStates.push_back(requiredState.toStdString());
            }
        }

        xml.readNext();

        if(xml.isEndElement() && xml.name() == "requiredstates")
            break;
    }
}

void RwaImport::readEnterconditions()
{
    vector<double> gps(2, 0.0);
    double lon;
    double lat;
    double enterOffset = -6;
    double exitOffset = 0;

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "gps")
            {
                lon = xml.attributes().value("lon").toDouble();
                lat = xml.attributes().value("lat").toDouble();
                gps[0] = lon;
                gps[1] = lat;

                currentState->setCoordinates(gps);
                currentState->setRadius(xml.attributes().value("radius").toDouble());
                currentState->setWidth(xml.attributes().value("width").toDouble());
                currentState->setHeight(xml.attributes().value("height").toDouble());
                currentState->isGpsState = xml.attributes().value("isgps").toInt();

//                if(xml.attributes().hasAttribute("enteroffset"))
//                {
//                    enterOffset = xml.attributes().value("enteroffset").toDouble();
//                    currentState->setEnterOffset(enterOffset);
//                }
                if(xml.attributes().hasAttribute("exitoffset"))
                {
                    exitOffset = xml.attributes().value("exitoffset").toDouble();
                    currentState->setExitOffset(exitOffset);
                }
            }

            if (xml.name() == "corners")
            {
                readCorners();
                //currentState->calculateEnterOffsetCorners();
                //currentState->calculateExitOffsetCorners();
            }

            if (xml.name() == "requiredstates")
                readRequiredStates();
        }

        if(xml.isEndElement() && xml.name() == "enterconditions")
            break;

        xml.readNext();
    }
}

void RwaImport::readAssets()
{
    std::vector<double> gps(2, 0.0);
    QPointF startPosition;
    QString url;
    QString path;
    QString fileName;
    int type;
    string id;
    int dampingFunction = 1;
    float dampingFactor = 30;
    float dampingTrim = 2;
    float damingMin = 0;
    float dampingMax = 1;
    double lon;
    double lat;

    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            if (xml.name() == "asset")
            {
                url = xml.attributes().value("url").toString();
                fileName = RwaUtilities::getFileName(url);

                if(xml.attributes().hasAttribute("uuid"))
                    id = std::string(xml.attributes().value("uuid").toString().toLatin1());
                else
                    id = std::string(QUuid::createUuid().toString().toLatin1());

                lon = xml.attributes().value("lon").toDouble();
                lat = xml.attributes().value("lat").toDouble();
                type = xml.attributes().value("type").toInt();
                gps[0] = lon;
                gps[1] = lat;
                lon = xml.attributes().value("startpositionlon").toDouble();
                lat = xml.attributes().value("startpositionlat").toDouble();
                startPosition = QPointF(lon, lat);

                path = QString("%1").arg(url);
                path = QString("%1/%2").arg(backend->completeAssetPath).arg(fileName);

                RwaAsset1 *item = new RwaAsset1(path.toStdString(),gps, type, id);
                item->setPlaybackType(xml.attributes().value("playbacktype").toInt());

                if(xml.attributes().hasAttribute("damping"))
                     item->setDampingFunction(xml.attributes().value("damping").toInt());
                else
                {
                    if(item->getPlaybackType() == RWAPLAYBACKTYPE_MONO || item->getPlaybackType() == RWAPLAYBACKTYPE_STEREO || item->getPlaybackType() == RWAPLAYBACKTYPE_NATIVE)
                        item->setDampingFunction(0);
                }

                if(xml.attributes().hasAttribute("reflectioncount"))
                        item->setReflectionCount(xml.attributes().value("reflectioncount").toInt());

                if(xml.attributes().hasAttribute("dampingfactor"))
                        item->setDampingFactor(xml.attributes().value("dampingfactor").toDouble());

                if(xml.attributes().hasAttribute("dampingtrim"))
                    item->setDampingTrim(xml.attributes().value("dampingtrim").toDouble());

                if(xml.attributes().hasAttribute("dampingmin"))
                    item->setDampingMin(xml.attributes().value("dampingmin").toDouble());

                if(xml.attributes().hasAttribute("dampingmax"))
                    item->setDampingMax(xml.attributes().value("dampingmax").toDouble());

                if(xml.attributes().hasAttribute("mindistance"))
                    item->setMinDistance(xml.attributes().value("mindistance").toDouble());

                if(xml.attributes().hasAttribute("offset"))
                    item->setOffset(xml.attributes().value("offset").toDouble());

                if(xml.attributes().hasAttribute("lockposition"))
                    item->setLockPosition(xml.attributes().value("lockposition").toInt());

                if(xml.attributes().hasAttribute("alwaysplayfrombeginning"))
                    item->setAlwaysPlayFromBeginning(xml.attributes().value("alwaysplayfrombeginning").toInt());

                if(xml.attributes().hasAttribute("channelcount"))
                    item->setNumberOfChannels(xml.attributes().value("channelcount").toInt());
                else
                {
                    if(type != RWAASSETTYPE_PD)
                    {
                        backend->simulator->runtime->openFile4MetaData(path.toLatin1());
                        item->setNumberOfChannels(RwaRuntime::assetChannelCount);
                    }
                }

                item->setFadeInTime(xml.attributes().value("fadein").toInt());
                item->setFadeOutTime(xml.attributes().value("fadeout").toInt());
                item->setCrossfadeTime(xml.attributes().value("crossfadetime").toInt());
                item->setDuration(xml.attributes().value("duration").toInt());

                item->setLoop(xml.attributes().value("loop").toInt());
                item->setMute(xml.attributes().value("mute").toInt());
                item->setHeadtrackerRelative2Source(xml.attributes().value("headtrackerrelative2source").toInt());
                item->setLoopUntilEndPosition(xml.attributes().value("loopuntilendposition").toInt());
                item->setIsExclusive(xml.attributes().value("isexclusive").toInt());
                item->setGps2pd(xml.attributes().value("gps2pd").toInt());
                item->setPlayOnlyOnce(xml.attributes().value("playonlyonce").toInt());
                item->setRawSensors2pd(xml.attributes().value("rawsensors2pd").toInt());
                item->setGain(xml.attributes().value("gain").toDouble());
                item->setAutoRotate(xml.attributes().value("rotate").toInt());
                item->setRotateOffset(xml.attributes().value("rotateoffset").toDouble());
                item->setRotateFrequency(xml.attributes().value("rotatefrequency").toDouble());
                item->setChannelRadius(xml.attributes().value("channelradius").toDouble());
                item->setMoveFromStartPosition(xml.attributes().value("move").toInt());
                item->setMovementSpeed(xml.attributes().value("speed").toDouble());
                item->setFixedAzimuth(xml.attributes().value("fixedazimuth").toInt());
                item->setFixedElevation(xml.attributes().value("fixedelevation").toInt());
                item->setFixedDistance(xml.attributes().value("fixeddistance").toInt());
                std::vector<double> tmp(2, 0.0);
                tmp[0] = startPosition.x();
                tmp[1] = startPosition.y();
                item->setStartPosition(tmp);
                readChannelPositions(item);
                readReflectionPositions(item);

                currentState->addAsset(item);
            }
        }

        if(xml.isEndElement() && xml.name() == "state")
            break;

        xml.readNext();
    }
}

