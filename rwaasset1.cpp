/*
*
* This file is part of RwaCreator
* an open-source cross-platform Middleware for creating interactive Soundwalks
*
* Copyright (C) 2015 - 2022 Thomas Resch
*
* License: MIT
*
*/

#include "rwaasset1.h"
#include <qdebug.h>
#include <quuid.h>

RwaAsset1::RwaAsset1(const std::string &data, std::vector<double> gps, qint32 type, const string uid)
    : RwaLocation1()
{
    this->currentPosition = std::vector<double>(2, 0.0);
    this->currentPosition = gps;
    this->startPosition = std::vector<double>(2, 0.0);
    this->startPosition = gps;
    this->fullPath = data;
    this->type = type;
    this->fileName = RwaUtilities::getFileName(data);
    this->gpsLocation = gps;
    this->uniqueId = uid;

    for(int i = 0; i < 64; i++)
    {
        this->channelcoordinates[i].resize(2);
        this->channelcoordinates[i][0] = 0.0;
        this->channelcoordinates[i][1] = 0.0;
        this->reflectioncoordinates[i].resize(2);
        this->reflectioncoordinates[i][0] = 0.0;
        this->reflectioncoordinates[i][1] = 0.0;
        this->customchannelcoordinates[i].resize(2);
        this->customchannelcoordinates[i][0] = 0.0;
        this->customchannelcoordinates[i][1] = 0.0;
        this->lastChannelBearing[i] = 0;
        this->channelBearing[i] = 0;
        this->channelDistance[i] = 0;
        this->channelGain[i] = 1.;
        this->channelRotate[i] = false;
        this->channelRotateFreq[i] = 0;
        this->hasCustomChannelPosition[i] = false;
        this->reflectionCoordinateIsSet[i] = false;
    }

    reflectionCount = 0;
    currentReflection = 0;
    setObjectName(this->fileName);
    calculateReflectionPositions();
}

RwaAsset1::~RwaAsset1()
{

}

void RwaAsset1::moveMyChildren(double dx, double dy)
{
    std::vector<double> newCoordinates(2, 0.0);

    for(int i = 0; i< 64; i++)
    {
        newCoordinates[0] = (channelcoordinates[i][0] + dx);
        newCoordinates[1] = (channelcoordinates[i][1] + dy);
        setChannelCoordinate(i, newCoordinates);

        newCoordinates[0] = (reflectioncoordinates[i][0] + dx);
        newCoordinates[1] = (reflectioncoordinates[i][1] + dy);
        setReflectionCoordinate(i, newCoordinates);
    }
}

void RwaAsset1::copyAttributes(RwaAsset1 *dest)
{
    dest->fullPath = this->fullPath;
    dest->type = this->type;
    dest->fileName = this->fileName;
    dest->gpsLocation[0] = this->gpsLocation[0];
    dest->gpsLocation[1] = this->gpsLocation[1];
    dest->blocked = this->blocked;
    dest->isExclusive = this->isExclusive;
    dest->isActive = this->isActive;
    dest->isAlive = this->isAlive;
    dest->loop = this->loop;
    dest->rawSensors2pd = this->rawSensors2pd;
    dest->gps2pd = this->gps2pd;
    dest->autoRotate = this->autoRotate;
    dest->lockPosition = this->lockPosition;
    dest->rotateOffset = this->rotateOffset;
    dest->moveFromStartPosition = this->moveFromStartPosition;
    dest->playbackType = this->playbackType;
    dest->gain = this->gain;
    dest->duration = this->duration;
    dest->numberOfChannels = this->numberOfChannels;
    dest->offset = this->offset;
    dest->minDistance = this->minDistance;
    dest->maxDistance = this->maxDistance;
    dest->dampingFunction = this->dampingFunction;
    dest->dampingFactor = this->dampingFactor;
    dest->dampingTrim = this->dampingTrim;
    dest->dampingMin = this->dampingMin;
    dest->dampingMax = this->dampingMax;
    dest->fadeOutAfter = this->fadeOutAfter;
    dest->channelRadius = this->channelRadius;
    dest->playOnlyOnce = this->playOnlyOnce;
    dest->loopUntilEndPosition = this->loopUntilEndPosition;
    dest->surviveAfterLeavingState = this->surviveAfterLeavingState;
    dest->reachedEndPosition = this->reachedEndPosition;
    dest->fixedAzimuth = this->fixedAzimuth;
    dest->fixedDistance = this->fixedDistance;
    dest->fixedElevation = this->fixedElevation;
    dest->minDistance = this->minDistance;
    dest->mute = this->mute;
    dest->headtrackerRelative2Source = this->headtrackerRelative2Source;
    dest->currentRotateAngleOffset = this->currentRotateAngleOffset;
    dest->fadeInTime = this->fadeInTime;
    dest->fadeOutTime = this->fadeOutTime;
    dest->crossfadeTime = this->crossfadeTime;
    dest->rotateFrequency = this->rotateFrequency;
    dest->movementSpeed = this->movementSpeed;
    dest->waitTimeBeforeMovement = this->waitTimeBeforeMovement;
    dest->moveFromStartPosition = this->moveFromStartPosition;
    dest->startPosition = this->startPosition;
    dest->currentPosition = this->currentPosition;
    dest->distanceForMovement = this->distanceForMovement;
    dest->movingDistancePerTick = this->movingDistancePerTick;
    dest->rotateOffsetPerTick = this->rotateOffsetPerTick;
    dest->myScene = this->myScene;
    dest->myState = this->myState;

    for(int i = 0; i < 64; i++)
    {
        dest->channelcoordinates[i][0] = this->channelcoordinates[i][0];
        dest->channelcoordinates[i][1] = this->channelcoordinates[i][1];
        dest->reflectioncoordinates[i][0] = this->reflectioncoordinates[i][0];
        dest->reflectioncoordinates[i][1] = this->reflectioncoordinates[i][1];
        dest->customchannelcoordinates[i][0] = this->customchannelcoordinates[i][0];
        dest->customchannelcoordinates[i][1] = this->customchannelcoordinates[i][1];
        dest->lastChannelBearing[i] = this->lastChannelBearing[i];
        dest->channelBearing[i] = this->channelBearing[i];
        dest->channelDistance[i] = this->channelDistance[i];
        dest->channelGain[i] = this->channelGain[i];
        dest->channelRotate[i] = this->channelRotate[i];
        dest->channelRotateFreq[i] = this->channelRotateFreq[i];
        dest->hasCustomChannelPosition[i] = this->hasCustomChannelPosition[i];
    }

    string uid = std::string(QUuid::createUuid().toString().toLatin1());
    dest->uniqueId = uid;
}

void RwaAsset1::calculateReflectionPositions()
{
    std::vector<double> tmp(2, 0.0);

    for(int i=0;i<reflectionCount;i++)
    {
        if(!reflectionCoordinateIsSet[i])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), 10.0, (360/reflectionCount)*i);
            reflectioncoordinates[i] = tmp;
            //setReflectionCoordinate(i, tmp);
        }
    }
}

int32_t RwaAsset1::getReflectionCount() const
{
    return reflectionCount;
}

void RwaAsset1::setReflectionCount(const int32_t &value)
{
    reflectionCount = value;
}

void RwaAsset1::calculateChannelPositions()
{
    std::vector<double> tmp(2, 0.0);
    int offset = 360-rotateOffset;

    if(this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALMONO ||
       this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALMONO_FABIAN)
    {
        if(!hasCustomChannelPosition[0])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (0+offset)%360);
            setChannelCoordinate(0, tmp);
        }
    }

    if(this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSTEREO ||
       this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN)
    {
        if(!hasCustomChannelPosition[0])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (-60+offset)%360);
            setChannelCoordinate(0, tmp);
        }
        if(!hasCustomChannelPosition[1])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (60+offset)%360);
            setChannelCoordinate(1, tmp);
        }
    }

    if(this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL5CHANNEL ||
       this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN)
    {
        if(!hasCustomChannelPosition[0])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (-60 + offset)%360);
            setChannelCoordinate(0, tmp);
        }
        if(!hasCustomChannelPosition[1])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (0 + offset)%360);
            setChannelCoordinate(1, tmp);
        }
        if(!hasCustomChannelPosition[2])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (60 + offset)%360);
            setChannelCoordinate(2, tmp);
        }
        if(!hasCustomChannelPosition[3])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (-120 + offset)%360);
            setChannelCoordinate(3, tmp);
        }
        if(!hasCustomChannelPosition[4])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (120 + offset)%360);
            setChannelCoordinate(4, tmp);
        }
    }

    if(this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN)
    {
        if(!hasCustomChannelPosition[0])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (-40 + offset)%360);
            setChannelCoordinate(0, tmp);
        }
        if(!hasCustomChannelPosition[1])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (0 + offset)%360);
            setChannelCoordinate(1, tmp);
        }
        if(!hasCustomChannelPosition[2])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (40 + offset)%360);
            setChannelCoordinate(2, tmp);
        }
        if(!hasCustomChannelPosition[3])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (-80 + offset)%360);
            setChannelCoordinate(3, tmp);
        }
        if(!hasCustomChannelPosition[4])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (80 + offset)%360);
            setChannelCoordinate(4, tmp);
        }
        if(!hasCustomChannelPosition[5])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (-120 + offset)%360);
            setChannelCoordinate(5, tmp);
        }
        if(!hasCustomChannelPosition[6])
        {
            tmp = RwaUtilities::calculateDestination1(getCoordinates(), static_cast<double>(getChannelRadius()), (120 + offset)%360);
            setChannelCoordinate(6, tmp);
        }
    }
}

void RwaAsset1::setChannelCoordinate(int32_t channelNumber, std::vector<double> coordinate)
{
    channelcoordinates[channelNumber] = coordinate;
}

void RwaAsset1::setCustomChannelCoordinate(int32_t channelNumber, std::vector<double> coordinate)
{
    customchannelcoordinates[channelNumber] = coordinate;
    hasCustomChannelPosition[channelNumber] = true;
}

void RwaAsset1::setReflectionCoordinate(int32_t channelNumber, std::vector<double> coordinate)
{
    reflectionCoordinateIsSet[channelNumber] = true;
    reflectioncoordinates[channelNumber] = coordinate;
}

void RwaAsset1::setIndividuellChannelPosition(int32_t channel, std::vector<double> position)
{
    channelcoordinates[channel]= position;
    hasCustomChannelPosition[channel] = true;
}

void RwaAsset1::setIndividualChannelRotateFrequency(int32_t channel, float frequency)
{
    if(channel < 0)
        return;
    if(channel >= 64)
        return;
    if(frequency < 0)
        return;

    channelRotateFreq[channel] = frequency;
    channelRotate[channel] = true;
}

void RwaAsset1::resetIndividualChannelPositions()
{
    for(int i = 0; i<64; i++)
        hasCustomChannelPosition[i] = false;

    calculateChannelPositions();
}

void RwaAsset1::setChannelRotateFrequency(int channel, float frequency)
{
    if(channel < 0)
        return;
    if(channel >= 64)
        return;
    if(frequency < 0)
        return;

    channelRotateFreq[channel] = frequency;
}

void RwaAsset1::setChannelGain(int32_t channel, float gain)
{
    if(channel < 0)
        return;
    if(channel >= 64)
        return;
    if(gain < 0)
        return;

    channelGain[channel] = gain;
}

string RwaAsset1::getFileName() const
{
    return fileName;
}

void RwaAsset1::setFileName(const string &value)
{
    fileName = value;
}

string RwaAsset1::getFullPath() const
{
    return fullPath;
}

void RwaAsset1::setFullPath(const string &value)
{
    fullPath = value;
}

string RwaAsset1::getUniqueId() const
{
    return uniqueId;
}

void RwaAsset1::setUniqueId(const string &value)
{
    uniqueId = value;
}

float RwaAsset1::getMinDistance() const
{
    return minDistance;
}

void RwaAsset1::setMinDistance(float value)
{
    minDistance = value;
    if(minDistance < 0)
        minDistance = -1;
}

float RwaAsset1::getDampingMax() const
{
    return dampingMax;
}

void RwaAsset1::setDampingMax(float value)
{
    dampingMax = value;
}

float RwaAsset1::getDampingMin() const
{
    return dampingMin;
}

void RwaAsset1::setDampingMin(float value)
{
    dampingMin = value;
}

float RwaAsset1::getDampingTrim() const
{
    return dampingTrim;
}

void RwaAsset1::setDampingTrim(float value)
{
    dampingTrim = value;
}

float RwaAsset1::getDampingFactor() const
{
    return dampingFactor;
}

void RwaAsset1::setDampingFactor(float value)
{
    dampingFactor = value;
}

float RwaAsset1::getOffset() const
{
    return offset;
}

void RwaAsset1::setOffset(float value)
{
    offset = value;
}
float RwaAsset1::getFixedElevation() const
{
    return fixedElevation;
}

void RwaAsset1::setFixedElevation(float value)
{
    if(value < -90)
        return;
    if(value > 90)
        return;

    fixedElevation = value;
}

float RwaAsset1::getFixedDistance() const
{
    return fixedDistance;
}

void RwaAsset1::setFixedDistance(float value)
{
    if(value <= 0)
        return;

    fixedDistance = value;
}

float RwaAsset1::getFixedAzimuth() const
{
    return fixedAzimuth;
}

void RwaAsset1::setFixedAzimuth(float value)
{
    if(value < 0)
        return;
    if(value >= 360)
        return;

    fixedAzimuth = value;
}

bool RwaAsset1::getMute() const
{
    return mute;
}

void RwaAsset1::setMute(bool value)
{
    mute = value;
}

bool RwaAsset1::getHeadtrackerRelative2Source() const
{
    return headtrackerRelative2Source;
}

void RwaAsset1::setHeadtrackerRelative2Source(bool value)
{
    headtrackerRelative2Source = value;
}

bool RwaAsset1::getHasCoordinates() const
{
    return hasCoordinates;
}

int32_t RwaAsset1::getType() const
{
    return type;
}

void RwaAsset1::setType(const int32_t &value)
{
    if(value < 0)
        return;

    type = value;
}

bool RwaAsset1::getAlwaysPlayFromBeginning() const
{
    return alwaysPlayFromBeginning;
}

void RwaAsset1::setAlwaysPlayFromBeginning(bool value)
{
    alwaysPlayFromBeginning = value;
}

bool RwaAsset1::customChannelPositionsEnabled() const
{
    return allowIndividuellChannelPositions;
}

void RwaAsset1::enableCustomChannelPositions(bool value)
{
    allowIndividuellChannelPositions = value;
}

bool RwaAsset1::getLockPosition() const
{
    return lockPosition;
}

void RwaAsset1::setLockPosition(bool value)
{
    lockPosition = value;
}

int32_t RwaAsset1::getRotateOffset() const
{
    return rotateOffset;
}

void RwaAsset1::setRotateOffset(int32_t value)
{
    if(value < 0)
        return;
    if(value >= 360)
        return;

    rotateOffset = value;
    calculateChannelPositions();
}

int32_t RwaAsset1::getDampingFunction() const
{
    return dampingFunction;
}

void RwaAsset1::setDampingFunction(const int32_t &value)
{
    dampingFunction = value;
}

float RwaAsset1::getChannelRadius() const
{
    return channelRadius;
}

bool RwaAsset1::getReachedEndPosition() const
{
    return reachedEndPosition;
}

void RwaAsset1::setReachedEndPosition(bool value)
{
    reachedEndPosition = value;
}

bool RwaAsset1::getFollowEntity() const
{
    return followEntity;
}

void RwaAsset1::setFollowEntity(bool value)
{
    followEntity = value;
}

bool RwaAsset1::getLoopUntilEndPosition() const
{
    return loopUntilEndPosition;
}

void RwaAsset1::setLoopUntilEndPosition(bool value)
{
    loopUntilEndPosition = value;
}

bool RwaAsset1::getSurviveAfterLeavingState() const
{
    return surviveAfterLeavingState;
}

void RwaAsset1::setSurviveAfterLeavingState(bool value)
{
    surviveAfterLeavingState = value;
}

float RwaAsset1::getRotateOffsetPerTick() const
{
    return rotateOffsetPerTick;
}

void RwaAsset1::setRotateOffsetPerTick(float value)
{
    rotateOffsetPerTick = value;
}

float RwaAsset1::getMovingDistancePerTick() const
{
    return movingDistancePerTick;
}

void RwaAsset1::setMovingDistancePerTick(float value)
{
    movingDistancePerTick = value;
}

std::vector<double> RwaAsset1::getCurrentPosition() const
{
    return currentPosition;
}

void RwaAsset1::setCurrentPosition(const std::vector<double> &value)
{
    currentPosition = value;
}

std::vector<double> RwaAsset1::getStartPosition() const
{
    return startPosition;
}

void RwaAsset1::setStartPosition(const std::vector<double> &value)
{
    startPosition = value;
    if(value[0] == 0 || value[1] == 0)
    {
        std::vector<double> tmp(2, 0.0);
        tmp[0] = gpsLocation[0];
        tmp[1] = gpsLocation[1];

        setStartPosition(tmp);
    }

    distanceForMovement = RwaUtilities::calculateDistance1(startPosition, getCoordinates()) * 1000;
    bearingForMovement = -RwaUtilities::calculateBearing1(startPosition, getCoordinates());
}

bool RwaAsset1::getMoveFromStartPosition() const
{
    return moveFromStartPosition;
}

void RwaAsset1::setMoveFromStartPosition(bool value)
{
    moveFromStartPosition = value;
}

float RwaAsset1::getWaitTimeBeforeMovement() const
{
    return waitTimeBeforeMovement;
}

void RwaAsset1::setWaitTimeBeforeMovement(float value)
{
    waitTimeBeforeMovement = value;
}

float RwaAsset1::getMovementSpeed() const
{
    return movementSpeed;
}

void RwaAsset1::setMovementSpeed(float value)
{
    movementSpeed = value;
}

float RwaAsset1::getCurrentRotateAngleOffset() const
{
    return currentRotateAngleOffset;
}

void RwaAsset1::setCurrentRotateAngleOffset(const float &value)
{
    currentRotateAngleOffset = value;
}

float RwaAsset1::getRotateFrequency() const
{
    return rotateFrequency;
}

void RwaAsset1::setRotateFrequency(float value)
{
    rotateFrequency = value;
}

bool RwaAsset1::getAutoRotate() const
{
    return autoRotate;
}

void RwaAsset1::setAutoRotate(bool value)
{
    autoRotate = value;
}

bool RwaAsset1::getAllowTouches() const
{
    return allowTouches;
}

void RwaAsset1::setAllowTouches(bool value)
{
    allowTouches = value;
}

int64_t RwaAsset1::getNumberOfChannels() const
{
    return numberOfChannels;
}

void RwaAsset1::setNumberOfChannels(const int64_t &value)
{
    numberOfChannels = value;
}

void RwaAsset1::setChannelRadius(float value)
{
    channelRadius = value;
    calculateChannelPositions();
}

int64_t RwaAsset1::getFadeOutAfter() const
{
    return fadeOutAfter;
}

void RwaAsset1::setFadeOutAfter(const int64_t &value)
{
    fadeOutAfter = value;
}

int64_t RwaAsset1::getDuration() const
{
    return duration;
}

void RwaAsset1::setDuration(const int64_t &value)
{
    duration = value;
    setFadeOutAfter(duration-getCrossfadeTime());
}


bool RwaAsset1::getPlayOnlyOnce() const
{
    return playOnlyOnce;
}

void RwaAsset1::setPlayOnlyOnce(bool value)
{
    playOnlyOnce = value;
}

float RwaAsset1::getGain() const
{
    return gain;
}

void RwaAsset1::setGain(float value)
{
    gain = value;
}

int32_t RwaAsset1::getCrossfadeTime() const
{
    return crossfadeTime;
}

void RwaAsset1::setCrossfadeTime(const int32_t &value)
{
    crossfadeTime = value;
    setFadeOutAfter(getDuration()-crossfadeTime);
}

int32_t RwaAsset1::getFadeInTime() const
{
    return fadeInTime;
}

void RwaAsset1::setFadeInTime(const int32_t &value)
{
    fadeInTime = value;
}

int32_t RwaAsset1::getFadeOutTime() const
{
    return fadeOutTime;
}

void RwaAsset1::setFadeOutTime(const qint32 &value)
{
    fadeOutTime = value;
}

int32_t RwaAsset1::getPlaybackType() const
{
    return playbackType;
}

void RwaAsset1::setPlaybackType(const int32_t &value)
{
    playbackType = value;

    if(playbackType == RWAPLAYBACKTYPE_BINAURALMONO_FABIAN)
        channelRadius = 0;
    else
    {
        if(channelRadius <= 0)
            channelRadius = 20;
    }

    calculateChannelPositions();
    calculateReflectionPositions();
}

bool RwaAsset1::getIsExclusive() const
{
    return isExclusive;
}

void RwaAsset1::setIsExclusive(bool value)
{
    isExclusive = value;
}

bool RwaAsset1::getIsActive() const
{
    return isActive;
}

void RwaAsset1::setIsActive(bool value)
{
    isActive = value;
}

bool RwaAsset1::getIsAlive() const
{
    return isAlive;
}

void RwaAsset1::setIsAlive(bool value)
{
    isAlive = value;
}

bool RwaAsset1::getLoop() const
{
    return loop;
}

void RwaAsset1::setLoop(bool value)
{
    loop = value;
}

bool RwaAsset1::getBlocked() const
{
    return blocked;
}

void RwaAsset1::setBlocked(bool value)
{
    blocked = value;
}

bool RwaAsset1::getRawSensors2pd() const
{
    return rawSensors2pd;
}

void RwaAsset1::setRawSensors2pd(bool value)
{
    rawSensors2pd = value;
}

bool RwaAsset1::getGps2pd() const
{
    return gps2pd;
}

void RwaAsset1::setGps2pd(bool value)
{
    gps2pd = value;
}

/*bool RwaAssetItem::getElevation2pd() const
{
    return elevation2pd;
}

void RwaAssetItem::setElevation2pd(bool value)
{
    elevation2pd = value;
}

bool RwaAssetItem::getOrientation2pd() const
{
    return orientation2pd;
}

void RwaAssetItem::setOrientation2pd(bool value)
{
    orientation2pd = value;
}*/


