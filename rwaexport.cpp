/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include "rwaexport.h"

RwaExport::RwaExport(RwaBackend *data, QString path, qint32 flags)
{
    this->data = data;
    this->path = path;
    this->copyAssets = false;

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
    xml.writeAttribute("currentscene",  QString::fromStdString(data->getLastTouchedScene()->objectName()));
    xml.writeEndElement();

    for(int i = 0;i<data->getNumberOfScenes();i++)
        writeScene(data->getSceneAt(i));

    xml.writeEndDocument();
    return true;
}

void RwaExport::writeAssetItem(RwaAsset1 *item)
{
    RwaBackend *backend = (RwaBackend *)data;
    QString absoluteAssetPath = QString("%1/%2").arg(backend->completeAssetPath).arg(QString::fromStdString(item->getFileName()));

   if(this->copyAssets)
   {
       QString absoluteAssetPath = QString("%1/%2").arg(this->path).arg(QString::fromStdString(item->getFileName()));
       QString newAssetPath = data->completeAssetPath;
       qDebug() << "Copy Asset: " << absoluteAssetPath;
       qDebug() << "To: " << newAssetPath;
       QFile::copy(absoluteAssetPath, QString("%1/%2").arg(newAssetPath).arg(QString::fromStdString(item->getFileName())));
       item->setFullPath((QString("%1/assets/%2").arg(path).arg(QString::fromStdString(item->getFileName()))).toStdString());
   }

   xml.writeStartElement("asset");
   xml.writeAttribute("url", QString::fromStdString(item->getFullPath()) );
   xml.writeAttribute("uuid", QString::fromStdString(item->getUniqueId()));
   xml.writeAttribute("type", QString::number(item->getType()));
   xml.writeAttribute("gain", QString::number(item->getGain()));
   xml.writeAttribute("lon", QString::number(item->getCoordinates()[0], 'f', 8));
   xml.writeAttribute("lat", QString::number(item->getCoordinates()[1], 'f', 8));
   xml.writeAttribute("playbacktype", QString::number(item->getPlaybackType()));
   xml.writeAttribute("damping", QString::number(item->getDampingFunction()));
   xml.writeAttribute("dampingfactor", QString::number(item->getDampingFactor()));
   xml.writeAttribute("dampingtrim", QString::number(item->getDampingTrim()));
   xml.writeAttribute("dampingmin", QString::number(item->getDampingMin()));
   xml.writeAttribute("dampingmax", QString::number(item->getDampingMax()));
   xml.writeAttribute("mindistance", QString::number(item->getMinDistance()));
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
   xml.writeAttribute("channelradius", QString::number(item->getChannelRadius()));
   //xml.writeAttribute("orientation2pd", QString::number(item->getOrientation2pd()));
   xml.writeAttribute("rawsensors2pd", QString::number(item->getRawSensors2pd()));
   xml.writeAttribute("playonlyonce", QString::number(item->getPlayOnlyOnce()));
   xml.writeAttribute("speed", QString::number(item->getMovementSpeed()));
   xml.writeAttribute("move", QString::number(item->getMoveFromStartPosition()));
   xml.writeAttribute("rotateoffset", QString::number(item->getRotateOffset()));
   xml.writeAttribute("rotatefrequency", QString::number(item->getRotateFrequency()));
   xml.writeAttribute("rotate", QString::number(item->getAutoRotate()));
   xml.writeAttribute("fixedazimuth", QString::number(item->getFixedAzimuth()));
   xml.writeAttribute("fixeddistance", QString::number(item->getFixedDistance()));
   xml.writeAttribute("fixedelevation", QString::number(item->getFixedElevation()));
   xml.writeAttribute("startpositionlon", QString::number(item->getStartPosition()[0]));
   xml.writeAttribute("startpositionlat", QString::number(item->getStartPosition()[1]));
   xml.writeAttribute("offset", QString::number(item->getOffset()));
   xml.writeAttribute("reflectioncount", QString::number(item->getReflectionCount()));

   xml.writeStartElement("channelpositions");
   for(int i = 0; i < 64; i++)
   {
       QString channel = QString("channel%1").arg(i);
        if(item->individuellChannelPosition[i])
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
    xml.writeAttribute("timeout", QString::number(state->getTimeOut()));
    xml.writeAttribute("minstaytime", QString::number(state->getMinimumStayTime()));
    xml.writeStartElement("enterconditions");
    xml.writeStartElement("gps");
    xml.writeAttribute("isgps", QString::number(state->isGpsState) );
    xml.writeAttribute("lon", QString::number(state->getCoordinates()[0], 'f', 8));
    xml.writeAttribute("lat", QString::number(state->getCoordinates()[1], 'f', 8));
    xml.writeAttribute("radius", QString::number(state->getRadius()));
    xml.writeAttribute("width", QString::number(state->getWidth()));
    xml.writeAttribute("height", QString::number(state->getHeight()));
    xml.writeAttribute("exitoffset", QString::number(state->getExitOffset()));

    xml.writeEndElement(); // end section gps
    xml.writeStartElement("requiredstates");
    foreach(std::string state, state->getRequiredStates())
        xml.writeTextElement("requiredstate", QString::fromStdString(state));

    xml.writeEndElement(); // end section requiredstates

    xml.writeStartElement("corners");
    if(state->getAreaType() == RWAAREATYPE_POLYGON)
    {
        for(int i= 0;i< state->corners.size(); i++)
        {

            xml.writeTextElement("lon", QString::number(state->corners[i][0], 'f', 8));
            xml.writeTextElement("lat", QString::number(state->corners[i][1], 'f', 8));
        }
    }
    xml.writeEndElement(); // end section corners

    xml.writeStartElement("exitoffsetcorners");
    if(state->getExitOffset() && (state->getAreaType() == RWAAREATYPE_POLYGON) )
    {

        for(int i= 0;i< state->exitOffsetCorners.size(); i++)
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
    if(state->getNextState().compare(""));
        xml.writeTextElement("nextstate", QString::fromStdString(state->getNextState()));

    if(state->getHintState().compare(""));
        xml.writeTextElement("hintstate", QString::fromStdString(state->getHintState()));

    if(state->getNextScene().compare(""))
        xml.writeTextElement("nextscene", QString::fromStdString(state->getNextScene()));

    xml.writeEndElement(); // end section actions

    xml.writeStartElement("assets");
    foreach(RwaAsset1 *item, state->getAssets())
        writeAssetItem(item);
     xml.writeEndElement(); // end section assets

    xml.writeEndElement(); // end section state
}

void RwaExport::writeScene(RwaScene *scene)
{
    xml.writeStartElement("scene");
    if(scene->currentState)
        xml.writeAttribute("currentstate", QString::fromStdString(scene->currentState->objectName()));
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
    xml.writeAttribute("exitoffset", QString::number(scene->getExitOffset()));
    xml.writeAttribute("fallbackdisabled", QString::number(scene->fallbackDisabled()));
    xml.writeAttribute("minstaytime", QString::number(scene->getMinimumStayTime()));

    xml.writeStartElement("corners");

    for(int i= 0;i< scene->corners.size(); i++)
    {
        xml.writeTextElement("lon", QString::number(scene->corners[i][0], 'f', 8));
        xml.writeTextElement("lat", QString::number(scene->corners[i][1], 'f', 8));
    }

    xml.writeEndElement(); // end section corners

    xml.writeStartElement("exitoffsetcorners");
    if(scene->getExitOffset())
    {
         for(int i= 0;i< scene->corners.size(); i++)
        {
             xml.writeTextElement("lon", QString::number(scene->exitOffsetCorners[i][0], 'f', 8));
             xml.writeTextElement("lon", QString::number(scene->exitOffsetCorners[i][1], 'f', 8));
        }
    }
    xml.writeEndElement(); // end section corners

    for(int i = 0;i<scene->getStates().size();i++)
    {
        auto statesList = scene->getStates().begin();
        std::advance(statesList, i);
        writeState(*statesList);
    }

    xml.writeEndElement();
}

