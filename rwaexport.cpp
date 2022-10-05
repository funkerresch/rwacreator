/*
*
* This file is part of RwaCreator
* an open-source cross-platform Middleware for creating interactive Soundwalks
*
* Copyright (C) 2015 - 2022 Thomas Resch
*
* License: MIT
*
* The RwaExport class is resposible for writing rwa xml scripts
*
*
*/

#include <QtWidgets>
#include "rwaexport.h"

RwaExport::RwaExport(QObject *parent, QString originalProjectPath, QString newProjectPath, qint32 flags)
    : QObject(parent)
{
    this->backend = RwaBackend::getInstance();
    this->path = originalProjectPath;
    this->copyAssets = false;
    this->newProjectPath = newProjectPath;

    xml.setAutoFormatting(true);

    if(RWAEXPORT_COPYASSETS & flags)
    {
        qDebug() << "Will copy assets";
        this->copyAssets = true;
    }
}

bool RwaExport::writeFile(QIODevice *device)
{
    xml.setDevice(device);
    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE rwa>");
    xml.writeStartElement("rwa");
    xml.writeAttribute("version", "1.0");
    xml.writeStartElement("game");
    if(backend->getLastTouchedScene())
        xml.writeAttribute("currentscene",  QString::fromStdString(backend->getLastTouchedScene()->objectName()));
    else
        xml.writeAttribute("currentscene",  QString::fromStdString(backend->getSceneAt(0)->objectName()));

    xml.writeEndElement();

    for(int i = 0;i<backend->getNumberOfScenes();i++)
        writeScene(backend->getSceneAt(i));

    xml.writeEndDocument();
    return true;
}

void RwaExport::writeAssetItem1(RwaAsset1 *item)
{
   if(this->copyAssets)
   {
       QString originalAssetPath = RwaUtilities::generateCompleteAssetPath(this->path, item->getFileName());
       QString newAssetPath = RwaUtilities::generateCompleteAssetPath(this->newProjectPath, item->getFileName());

       qDebug() << "Copy Asset from: " << originalAssetPath;
       qDebug() << "To: " << newAssetPath;
       QFile::copy(originalAssetPath, newAssetPath);
       item->setFullPath(newAssetPath.toStdString());
   }

   xml.writeStartElement("asset");
   xml.writeAttribute("url", QString::fromStdString(item->getFullPath()) );
   xml.writeAttribute("uuid", QString::fromStdString(item->getUniqueId()));
   xml.writeAttribute("type", QString::number(item->getType()));
   xml.writeAttribute("gain", QString::number(static_cast<double>(item->getGain())));
   xml.writeAttribute("elevation", QString::number(static_cast<double>(item->getElevation())));
   xml.writeAttribute("lon", QString::number(item->getCoordinates()[0], 'f', 8));
   xml.writeAttribute("lat", QString::number(item->getCoordinates()[1], 'f', 8));
   xml.writeAttribute("playbacktype", QString::number(item->getPlaybackType()));
   xml.writeAttribute("damping", QString::number(item->getDampingFunction()));
   xml.writeAttribute("dampingfactor", QString::number(static_cast<double>(item->getDampingFactor())));
   xml.writeAttribute("dampingtrim", QString::number(static_cast<double>(item->getDampingTrim())));
   xml.writeAttribute("dampingmin", QString::number(static_cast<double>(item->getDampingMin())));
   xml.writeAttribute("dampingmax", QString::number(static_cast<double>(item->getDampingMax())));
   xml.writeAttribute("smoothdist", QString::number(static_cast<double>(item->getSmoothDist())));
   xml.writeAttribute("mindistance", QString::number(static_cast<double>(item->getMinDistance())));
   xml.writeAttribute("fadein", QString::number(item->getFadeInTime()));
   xml.writeAttribute("fadeout", QString::number(item->getFadeOutTime()));
   xml.writeAttribute("crossfadetime", QString::number(item->getCrossfadeTime()));
   xml.writeAttribute("duration", QString::number(item->getDuration()));
   xml.writeAttribute("channelcount", QString::number(item->getNumberOfChannels())); //new
   xml.writeAttribute("lockposition", QString::number(item->getLockPosition()));
   xml.writeAttribute("alwaysplayfrombeginning", QString::number(item->getAlwaysPlayFromBeginning()));
   xml.writeAttribute("loop", QString::number(item->getLoop()));
   xml.writeAttribute("mute", QString::number(item->getMute()));
   xml.writeAttribute("headtrackerrelative2source", QString::number(item->getHeadtrackerRelative2Source()));
   xml.writeAttribute("hascoordinates", QString::number(item->getHasCoordinates()));
   xml.writeAttribute("loopuntilendposition", QString::number(item->getLoopUntilEndPosition()));
   //xml.writeAttribute("elevation2pd", QString::number(item->getElevation2pd()));
   xml.writeAttribute("gps2pd", QString::number(item->getGps2pd()));
   xml.writeAttribute("isexclusive", QString::number(item->getIsExclusive()));
   xml.writeAttribute("channelradius", QString::number(static_cast<double>(item->getChannelRadius())));
   //xml.writeAttribute("orientation2pd", QString::number(item->getOrientation2pd()));
   xml.writeAttribute("rawsensors2pd", QString::number(item->getRawSensors2pd()));
   xml.writeAttribute("playonlyonce", QString::number(item->getPlayOnlyOnce()));
   xml.writeAttribute("speed", QString::number(static_cast<double>(item->getMovementSpeed())));
   xml.writeAttribute("move", QString::number(item->getMoveFromStartPosition()));
   xml.writeAttribute("rotateoffset", QString::number(item->getRotateOffset()));
   xml.writeAttribute("rotatefrequency", QString::number(static_cast<double>(item->getRotateFrequency())));
   xml.writeAttribute("rotate", QString::number(item->getAutoRotate()));
   xml.writeAttribute("fixedazimuth", QString::number(static_cast<double>(item->getFixedAzimuth())));
   xml.writeAttribute("fixeddistance", QString::number(static_cast<double>(item->getFixedDistance())));
   xml.writeAttribute("fixedelevation", QString::number(static_cast<double>(item->getFixedElevation())));
   xml.writeAttribute("startpositionlon", QString::number(item->getStartPosition()[0], 'f', 8));
   xml.writeAttribute("startpositionlat", QString::number(item->getStartPosition()[1], 'f', 8));
   xml.writeAttribute("offset", QString::number(static_cast<double>(item->getOffset())));
   xml.writeAttribute("reflectioncount", QString::number(item->getReflectionCount()));
   xml.writeAttribute("individualchannelpositions", QString::number(item->customChannelPositionsEnabled()));
   xml.writeStartElement("channelpositions");
   for(int i = 0; i < 64; i++)
   {
       QString channel = QString("channel%1").arg(i);
        if(item->hasCustomChannelPosition[i])
        {
            xml.writeStartElement(channel);
            xml.writeAttribute("lon", QString::number(item->channelcoordinates[i][0], 'f', 8 ));
            xml.writeAttribute("lat", QString::number(item->channelcoordinates[i][1], 'f', 8 ));
            xml.writeEndElement();
        }
   }
   xml.writeEndElement();
   xml.writeStartElement("reflectionpositions");
   for(int i = 0; i < item->getReflectionCount(); i++)
   {
        QString channel = QString("reflection%1").arg(i);
        xml.writeStartElement(channel);
        xml.writeAttribute("lon", QString::number(item->reflectioncoordinates[i][0], 'f', 8 ));
        xml.writeAttribute("lat", QString::number(item->reflectioncoordinates[i][1], 'f', 8 ));
        xml.writeEndElement();
   }

   xml.writeEndElement();
   xml.writeEndElement();
}

void RwaExport::writeState(RwaState *state)
{
    xml.writeStartElement("state");
    if(state->getLastTouchedAsset())
        xml.writeAttribute("currentAsset", QString::fromStdString(state->getLastTouchedAsset()->objectName()));
    xml.writeAttribute("name", QString::fromStdString(state->objectName()));
    xml.writeAttribute("type", QString::number(state->getType()));
    xml.writeAttribute("areatype", QString::number(state->getAreaType()));
    xml.writeAttribute("zoom", QString::number(state->getZoom()));
    xml.writeAttribute("assetsfollowstate", QString::number(state->childrenDoFollowMe()));
    xml.writeAttribute("defaultplaybacktype", QString::number(state->getDefaultPlaybackType()));
    xml.writeAttribute("lockposition", QString::number(state->positionIsLocked()));
    xml.writeAttribute("statewithinstate", QString::number(state->stateWithinState));
    xml.writeAttribute("leaveafterassetsfinish", QString::number(state->getLeaveAfterAssetsFinish()));
    xml.writeAttribute("leaveonlyafterassetsfinish", QString::number(state->getLeaveOnlyAfterAssetsFinish()));
    xml.writeAttribute("enteronlyonce", QString::number(state->getEnterOnlyOnce() ));
    xml.writeAttribute("timeout", QString::number(static_cast<double>(state->getTimeOut())));
    xml.writeAttribute("minstaytime", QString::number(static_cast<double>(state->getMinimumStayTime())));

    xml.writeStartElement("enterconditions");
    xml.writeStartElement("gps");

    xml.writeAttribute("isgps", QString::number(state->isGpsState) );
    xml.writeAttribute("lon", QString::number(state->getCoordinates()[0], 'f', 8));
    xml.writeAttribute("lat", QString::number(state->getCoordinates()[1], 'f', 8));
    xml.writeAttribute("radius", QString::number(state->getRadius()));
    xml.writeAttribute("width", QString::number(state->getWidth()));
    xml.writeAttribute("height", QString::number(state->getHeight()));
    xml.writeAttribute("exitoffset", QString::number(static_cast<double>(state->getExitOffset())));

    xml.writeEndElement(); // end section gps
    xml.writeStartElement("requiredstates");
    foreach(std::string state, state->getRequiredStates())
        xml.writeTextElement("requiredstate", QString::fromStdString(state));

    xml.writeEndElement(); // end section requiredstates

    xml.writeStartElement("corners");
    if(state->getAreaType() == RWAAREATYPE_POLYGON)
    {
        for(uint32_t i= 0;i< state->corners.size(); i++)
        {
            xml.writeTextElement("lon", QString::number(state->corners[i][0], 'f', 8));
            xml.writeTextElement("lat", QString::number(state->corners[i][1], 'f', 8));
        }
    }
    xml.writeEndElement(); // end section corners   

    xml.writeStartElement("exitoffsetcorners");
    if((state->getExitOffset() > 0) && (state->getAreaType() == RWAAREATYPE_POLYGON) )
    {
        for(uint32_t i= 0;i< state->exitOffsetCorners.size(); i++)
        {
            QPointF corner;
            corner.setX(state->exitOffsetCorners[i][0]);
            corner.setY(state->exitOffsetCorners[i][1]);
            xml.writeTextElement("lon", QString::number(corner.x(), 'f', 8));
            xml.writeTextElement("lat", QString::number(corner.y(), 'f', 8));
        }

    }
    xml.writeEndElement(); // end section corners

    xml.writeEndElement(); // end section enterconditions

    xml.writeStartElement("actions");
    if(state->getNextState().compare(""))
        xml.writeTextElement("nextstate", QString::fromStdString(state->getNextState()));

    if(state->getHintState().compare(""))
        xml.writeTextElement("hintstate", QString::fromStdString(state->getHintState()));

    if(state->getNextScene().compare(""))
        xml.writeTextElement("nextscene", QString::fromStdString(state->getNextScene()));

    xml.writeEndElement(); // end section actions

    xml.writeStartElement("assets");
    foreach(RwaAsset1 *item, state->getAssets())
            writeAssetItem1(item);

    xml.writeEndElement(); // end section assets

    xml.writeEndElement(); // end section state
}

void RwaExport::writeScene(RwaScene *scene)
{
    xml.writeStartElement("scene");
    if(scene->lastTouchedState)
        xml.writeAttribute("currentstate", QString::fromStdString(scene->lastTouchedState->objectName()));
    xml.writeAttribute("name", QString::fromStdString(scene->objectName()));
    xml.writeAttribute("lon", QString::number(scene->getCoordinates()[0], 'f', 8));
    xml.writeAttribute("lat", QString::number(scene->getCoordinates()[1], 'f', 8));
    xml.writeAttribute("zoom", QString::number(scene->getZoom()));
    xml.writeAttribute("numberofstates", QString::number(scene->getStates().size()));
    xml.writeAttribute("areatype", QString::number(scene->getAreaType()));
    xml.writeAttribute("radius", QString::number(scene->getRadius()));
    xml.writeAttribute("width", QString::number(scene->getWidth()));
    xml.writeAttribute("height", QString::number(scene->getHeight()));
    xml.writeAttribute("level", QString::number(scene->getLevel()));
    xml.writeAttribute("exitoffset", QString::number(static_cast<double>(scene->getExitOffset())));
    xml.writeAttribute("fallbackdisabled", QString::number(scene->fallbackDisabled()));
    xml.writeAttribute("minstaytime", QString::number(static_cast<double>(scene->getMinimumStayTime())));

    xml.writeStartElement("corners");

    for(uint32_t i= 0;i< scene->corners.size(); i++)
    {
        xml.writeTextElement("lon", QString::number(scene->corners[i][0], 'f', 8));
        xml.writeTextElement("lat", QString::number(scene->corners[i][1], 'f', 8));
    }

    xml.writeEndElement(); // end section corners

    xml.writeStartElement("exitoffsetcorners");
    if(scene->getExitOffset() > 0)
    {
         for(uint32_t i= 0;i< scene->corners.size(); i++)
        {
             xml.writeTextElement("lon", QString::number(scene->exitOffsetCorners[i][0], 'f', 8));
             xml.writeTextElement("lon", QString::number(scene->exitOffsetCorners[i][1], 'f', 8));
        }
    }
    xml.writeEndElement(); // end section corners

    for(uint32_t i = 0;i<scene->getStates().size();i++)
    {
        auto statesList = scene->getStates().begin();
        std::advance(statesList, i);
        writeState(*statesList);
    }

    xml.writeEndElement();
}

