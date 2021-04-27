#include "rwaassetitem.h"
#include <qdebug.h>
#include "taglib.h"
#include "fileref.h"

RwaAssetItem::RwaAssetItem(const QString &data, QPointF gps, qint32 type, qint32 id, QObject *parent)
    :RwaLocation(parent)
{
    this->fullPath = data;
    this->type = type;
    this->fileName = RwaUtilities::getFileName(data);
    this->receiverPatcher = QString();
    this->gpsLocation = gps;
    this->hasCoordinates = true;
    this->blocked = false;
    this->isExclusive = false; // no other asset at the same time
    this->isActive = false;    // currently active
    this->isAlive = true;     // can be activated (in principle)
    this->loop = false;        // start again automatically while within state radius
    this->rawSensors2pd = false;
    this->gps2pd = false;
    this->autoRotate = false;
    this->lockPosition = false;
    this->rotateOffset = 0;
    this->moveFromStartPosition = false;
    this->playbackType = 0;
    this->gain = 1.0;
    this->duration = 0;
    this->offset = 0;
    this->minDistance = 6;
    this->maxDistance = 50;
    this->dampingFunction = 1;
    this->dampingFactor = 30;
    this->dampingTrim = 2;
    this->dampingMin = 0;
    this->dampingMax = 1;
    this->fadeOutAfter = 0;
    this->channelRadius = 20;
    this->playOnlyOnce = 0;
    this->uniqueId = QUuid::createUuid();
    this->loopUntilEndPosition = false;
    this->surviveAfterLeavingState = false;
    this->reachedEndPosition = true;
    this->fixedAzimuth = -1;
    this->fixedDistance = -1;
    this->fixedElevation = -1;
    this->minDistance = -1;
    this->mute = false;
    this->headtrackerRelative2Source = true;

    for(int i = 0; i < 64; i++)
    {
        this->channelcoordinates[i].setX(0);
        this->channelcoordinates[i].setY(0);
        this->lastChannelBearing[i] = 0;
        this->channelBearing[i] = 0;
        this->channelDistance[i] = 0;
        this->channelGain[i] = 1.;
        this->channelRotate[i] = false;
        this->channelRotateFreq[i] = 0;
        this->individuellChannelPosition[i] = false;
    }

     TagLib::FileRef file(fullPath.toLatin1());
     if(!file.isNull())
     {
        this->setDuration(file.audioProperties()->lengthInMilliseconds());
         //qDebug() << "FILE LENGTH: "<<file.audioProperties()->lengthInMilliseconds();
        this->setNumberOfChannels(file.audioProperties()->channels());
         //qDebug() << "NUMBER OF CHANNElS: "<<file.audioProperties()->channels();
     }

     this->currentRotateAngleOffset = 0;
     this->fadeInTime = 50;
     this->fadeOutTime = 50;
     this->setCrossfadeTime(100);
     this->rotateFrequency = 0;
     this->movementSpeed = 20;
     this->waitTimeBeforeMovement = 0;
     this->moveFromStartPosition = false;
     this->startPosition = gps;
     this->currentPosition = gps;
     this->distanceForMovement = 0;
     this->movingDistancePerTick = 0;
     this->rotateOffsetPerTick = 0;
}

void RwaAssetItem::copyAttributes(RwaAssetItem *dest)
{
    dest->fullPath = this->fullPath;
    dest->type = this->type;
    dest->fileName = this->fileName;
    dest->gpsLocation.setX(this->gpsLocation.x());
    dest->gpsLocation.setY(this->gpsLocation.y());
    dest->hasCoordinates = this->hasCoordinates;
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

    for(int i = 0; i < 64; i++)
    {
        dest->channelcoordinates[i].setX(this->channelcoordinates[i].x());
        dest->channelcoordinates[i].setY(this->channelcoordinates[i].y());
        dest->lastChannelBearing[i] = this->lastChannelBearing[i];
        dest->channelBearing[i] = this->channelBearing[i];
        dest->channelDistance[i] = this->channelDistance[i];
        dest->channelGain[i] = this->channelGain[i];
        dest->channelRotate[i] = this->channelRotate[i];
        dest->channelRotateFreq[i] = this->channelRotateFreq[i];
        dest->individuellChannelPosition[i] = this->individuellChannelPosition[i];
    }

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
}

void RwaAssetItem::setIndividuellChannelPosition(int channel, QPointF position)
{
    channelcoordinates[channel].setX(position.x());
    channelcoordinates[channel].setY(position.y());
    individuellChannelPosition[channel] = true;
}

void RwaAssetItem::setIndividualChannelRotateFrequency(int channel, float frequency)
{
    channelRotateFreq[channel] = frequency;
    channelRotate[channel] = true;
}

void RwaAssetItem::setChannelRotateFrequency(int channel, float frequency)
{
    channelRotateFreq[channel] = frequency;
}

void RwaAssetItem::setChannelGain(int channel, float gain)
{
    channelGain[channel] = gain;
}

void RwaAssetItem::unblock()
{
    this->blocked = false;
}

QString RwaAssetItem::getReceiverPatcher() const
{
    return receiverPatcher;
}

void RwaAssetItem::setReceiverPatcher(const QString &value)
{
    receiverPatcher = value;
}

bool RwaAssetItem::getLockPosition() const
{
    return lockPosition;
}

void RwaAssetItem::setLockPosition(bool value)
{
    lockPosition = value;
}

int RwaAssetItem::getRotateOffset() const
{
    return rotateOffset;
}

void RwaAssetItem::setRotateOffset(int value)
{
    rotateOffset = value;
    calculateChannelPositions();
}

float RwaAssetItem::getMinDistance() const
{
    return minDistance;
}

void RwaAssetItem::setMinDistance(float value)
{
    minDistance = value;
}

float RwaAssetItem::getDampingMax() const
{
    return dampingMax;
}

void RwaAssetItem::setDampingMax(float value)
{
    dampingMax = value;
}

float RwaAssetItem::getDampingMin() const
{
    return dampingMin;
}

void RwaAssetItem::setDampingMin(float value)
{
    dampingMin = value;
}

float RwaAssetItem::getDampingTrim() const
{
    return dampingTrim;
}

void RwaAssetItem::setDampingTrim(float value)
{
    dampingTrim = value;
}

float RwaAssetItem::getDampingFactor() const
{
    return dampingFactor;
}

void RwaAssetItem::setDampingFactor(float value)
{
    dampingFactor = value;
}

qint32 RwaAssetItem::getDampingFunction() const
{
    return dampingFunction;
}

void RwaAssetItem::setDampingFunction(const qint32 &value)
{
    dampingFunction = value;
}

float RwaAssetItem::getOffset() const
{
    return offset;
}

void RwaAssetItem::setOffset(float value)
{
    offset = value;
}
float RwaAssetItem::getFixedElevation() const
{
    return fixedElevation;
}

void RwaAssetItem::setFixedElevation(float value)
{
    fixedElevation = value;
}

/*bool RwaAssetItem::getDistance2volume() const
{
    return distance2volume;
}

void RwaAssetItem::setDistance2volume(bool value)
{
    distance2volume = value;
}*/

float RwaAssetItem::getFixedDistance() const
{
    return fixedDistance;
}

void RwaAssetItem::setFixedDistance(float value)
{
    fixedDistance = value;
}

float RwaAssetItem::getFixedAzimuth() const
{
    return fixedAzimuth;
}

void RwaAssetItem::setFixedAzimuth(float value)
{
    fixedAzimuth = value;
}

float RwaAssetItem::getChannelRadius() const
{
    return channelRadius;
}

bool RwaAssetItem::getReachedEndPosition() const
{
    return reachedEndPosition;
}

void RwaAssetItem::setReachedEndPosition(bool value)
{
    reachedEndPosition = value;
}

bool RwaAssetItem::getFollowEntity() const
{
    return followEntity;
}

void RwaAssetItem::setFollowEntity(bool value)
{
    followEntity = value;
}

bool RwaAssetItem::getLoopUntilEndPosition() const
{
    return loopUntilEndPosition;
}

void RwaAssetItem::setLoopUntilEndPosition(bool value)
{
    loopUntilEndPosition = value;
}

bool RwaAssetItem::getSurviveAfterLeavingState() const
{
    return surviveAfterLeavingState;
}

void RwaAssetItem::setSurviveAfterLeavingState(bool value)
{
    surviveAfterLeavingState = value;
}

float RwaAssetItem::getExitStateAfter() const
{
    return exitStateAfter;
}

void RwaAssetItem::setExitStateAfter(float value)
{
    exitStateAfter = value;
}

float RwaAssetItem::getRotateOffsetPerTick() const
{
    return rotateOffsetPerTick;
}

void RwaAssetItem::setRotateOffsetPerTick(float value)
{
    rotateOffsetPerTick = value;
}

float RwaAssetItem::getMovingDistancePerTick() const
{
    return movingDistancePerTick;
}

void RwaAssetItem::setMovingDistancePerTick(float value)
{
    movingDistancePerTick = value;
}

QPointF RwaAssetItem::getCurrentPosition() const
{
    return currentPosition;
}

void RwaAssetItem::setCurrentPosition(const QPointF &value)
{
    currentPosition = value;
}

QPointF RwaAssetItem::getStartPosition() const
{
    return startPosition;
}

void RwaAssetItem::setStartPosition(const QPointF &value)
{
    startPosition = value;
    if(value.x() == 0 || value.y() == 0)
        setStartPosition(gpsLocation);

    distanceForMovement = RwaUtilities::calculateDistance(startPosition, getCoordinates()) * 1000;
    bearingForMovement = -RwaUtilities::calculateBearing(startPosition, getCoordinates());
}

bool RwaAssetItem::getMoveFromStartPosition() const
{
    return moveFromStartPosition;
}

void RwaAssetItem::setMoveFromStartPosition(bool value)
{
    moveFromStartPosition = value;
}

float RwaAssetItem::getWaitTimeBeforeMovement() const
{
    return waitTimeBeforeMovement;
}

void RwaAssetItem::setWaitTimeBeforeMovement(float value)
{
    waitTimeBeforeMovement = value;
}

float RwaAssetItem::getMovementSpeed() const
{
    return movementSpeed;
}

void RwaAssetItem::setMovementSpeed(float value)
{
    movementSpeed = value;
}

float RwaAssetItem::getCurrentRotateAngleOffset() const
{
    return currentRotateAngleOffset;
}

void RwaAssetItem::setCurrentRotateAngleOffset(const float &value)
{
    currentRotateAngleOffset = value;
}

float RwaAssetItem::getRotateFrequency() const
{
    return rotateFrequency;
}

void RwaAssetItem::setRotateFrequency(float value)
{
    rotateFrequency = value;
}

bool RwaAssetItem::getAutoRotate() const
{
    return autoRotate;
}

void RwaAssetItem::setAutoRotate(bool value)
{
    autoRotate = value;
}

bool RwaAssetItem::getAllowTouches() const
{
    return allowTouches;
}

void RwaAssetItem::setAllowTouches(bool value)
{
    allowTouches = value;
}

QUuid RwaAssetItem::getUniqueId() const
{
    return uniqueId;
}

qint64 RwaAssetItem::getNumberOfChannels() const
{
    return numberOfChannels;
}

void RwaAssetItem::setNumberOfChannels(const qint64 &value)
{
    numberOfChannels = value;
}

float RwaAssetItem::getMultichannelsourceradius() const
{
    return channelRadius;
}
void RwaAssetItem::calculateAssetChannelParameters(int channel, int bearing)
{
    float dx, dy;
    float mainLat, mainLon;
    float tmpLat, tmpLon;

    mainLat = this->getCoordinates().y();
    mainLon = this->getCoordinates().x();

    dy = cos(RwaUtilities::degrees2radians(bearing)) * this->getMultichannelsourceradius();
    dx = -sin(RwaUtilities::degrees2radians(bearing)) * this->getMultichannelsourceradius();
    tmpLat = mainLat + (180/M_PI) * (dy/6378137);
    tmpLon = mainLon + (180/M_PI) * (dx/6378137)/cos(mainLat);

    channelcoordinates[channel].setY(tmpLat);
    channelcoordinates[channel].setX(tmpLon);
}

void RwaAssetItem::setChannelCoordinate(int channelNumber, QPointF coordinate)
{
    channelcoordinates[channelNumber].setX(coordinate.x());
    channelcoordinates[channelNumber].setY(coordinate.y());
}

void RwaAssetItem::calculateChannelPositions()
{
    float mainLat, mainLon;
    QPointF tmp;

    mainLat = this->getCoordinates().y();
    mainLon = this->getCoordinates().x();
    int turnedOffset = 360-rotateOffset;

    if(this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALMONO ||
       this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALMONO_FABIAN)
    {
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (0+turnedOffset)%360);
        setChannelCoordinate(0, tmp);
    }

    if(this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSTEREO ||
       this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN)
    {
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (-60+turnedOffset)%360);
        setChannelCoordinate(0, tmp);
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (60+turnedOffset)%360);
        setChannelCoordinate(1, tmp);
    }

    if(this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL5CHANNEL ||
       this->getPlaybackType() == RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN)
    {
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (-60 + turnedOffset)%360);
        setChannelCoordinate(0, tmp);
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (0 + turnedOffset)%360);
        setChannelCoordinate(1, tmp);
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (60 + turnedOffset)%360);
        setChannelCoordinate(2, tmp);
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (-120 + turnedOffset)%360);
        setChannelCoordinate(3, tmp);
        tmp = RwaUtilities::calculateDestination(this->getCoordinates(), this->getMultichannelsourceradius(), (120 + turnedOffset)%360);
        setChannelCoordinate(4, tmp);
    }
}

void RwaAssetItem::setChannelRadius(float value)
{
    channelRadius = value;
    calculateChannelPositions();
}

qint64 RwaAssetItem::getFadeOutAfter() const
{
    return fadeOutAfter;
}

void RwaAssetItem::setFadeOutAfter(const qint64 &value)
{
    fadeOutAfter = value;
}

qint64 RwaAssetItem::getDuration() const
{
    return duration;
}

void RwaAssetItem::setDuration(const qint64 &value)
{
    duration = value;
    setFadeOutAfter(duration-getCrossfadeTime());
}


bool RwaAssetItem::getPlayOnlyOnce() const
{
    return playOnlyOnce;
}

void RwaAssetItem::setPlayOnlyOnce(bool value)
{
    playOnlyOnce = value;
}

float RwaAssetItem::getGain() const
{
    return gain;
}

void RwaAssetItem::setGain(float value)
{
    gain = value;
}

qint32 RwaAssetItem::getCrossfadeTime() const
{
    return crossfadeTime;
}

void RwaAssetItem::setCrossfadeTime(const qint32 &value)
{
    crossfadeTime = value;
    setFadeOutAfter(getDuration()-crossfadeTime);
}

qint32 RwaAssetItem::getFadeInTime() const
{
    return fadeInTime;
}

void RwaAssetItem::setFadeInTime(const qint32 &value)
{
    fadeInTime = value;
}

qint32 RwaAssetItem::getFadeOutTime() const
{
    return fadeOutTime;
}

void RwaAssetItem::setFadeOutTime(const qint32 &value)
{
    fadeOutTime = value;
}

qint32 RwaAssetItem::getPlaybackType() const
{
    return playbackType;
}

void RwaAssetItem::setPlaybackType(const qint32 &value)
{
    playbackType = value;
}

bool RwaAssetItem::getIsExclusive() const
{
    return isExclusive;
}

void RwaAssetItem::setIsExclusive(bool value)
{
    isExclusive = value;
}

bool RwaAssetItem::getIsActive() const
{
    return isActive;
}

void RwaAssetItem::setIsActive(bool value)
{
    isActive = value;
}

bool RwaAssetItem::getIsAlive() const
{
    return isAlive;
}

void RwaAssetItem::setIsAlive(bool value)
{
    isAlive = value;
}

bool RwaAssetItem::getLoop() const
{
    return loop;
}

void RwaAssetItem::setLoop(bool value)
{
    loop = value;
}

bool RwaAssetItem::getBlocked() const
{
    return blocked;
}

void RwaAssetItem::setBlocked(bool value)
{
    blocked = value;
}

bool RwaAssetItem::getRawSensors2pd() const
{
    return rawSensors2pd;
}

void RwaAssetItem::setRawSensors2pd(bool value)
{
    rawSensors2pd = value;
}

bool RwaAssetItem::getGps2pd() const
{
    return gps2pd;
}

void RwaAssetItem::setGps2pd(bool value)
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


