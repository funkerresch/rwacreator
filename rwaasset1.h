#ifndef RWAASSETITEM_H
#define RWAASSETITEM_H

#include "rwautilities.h"
#include "rwalocation1.h"
#include <string.h>
#include <stdint.h>

#define RWA_UNDETERMINED 0

#define RWAASSETTYPE_WAV 1
#define RWAASSETTYPE_AIF 2
#define RWAASSETTYPE_PD 3
#define RWAASSETTYPE_ITEM 4
#define RWAASSETTYPE_ENTITY 5

#define RWAPOSITIONTYPE_ASSET 1
#define RWAPOSITIONTYPE_ASSETCHANNEL 2
#define RWAPOSITIONTYPE_ASSETSTARTPOINT 3
#define RWAPOSITIONTYPE_CURRENTASSETPOSITION 4
#define RWAPOSITIONTYPE_REFLECTIONPOSITION 5

#define RWAPLAYBACKTYPE_MONO 1
#define RWAPLAYBACKTYPE_STEREO 2
#define RWAPLAYBACKTYPE_NATIVE 3

#define RWAPLAYBACKTYPE_BINAURAL 4
#define RWAPLAYBACKTYPE_BINAURALMONO 4
#define RWAPLAYBACKTYPE_BINAURALSTEREO 5
#define RWAPLAYBACKTYPE_BINAURALAUTO 6
#define RWAPLAYBACKTYPE_BINAURAL5CHANNEL 7

#define RWAPLAYBACKTYPE_BINAURAL_FABIAN 8
#define RWAPLAYBACKTYPE_BINAURALMONO_FABIAN 8
#define RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN 9
#define RWAPLAYBACKTYPE_BINAURALAUTO_FABIAN 10
#define RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN 11
#define RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN 12

#define RWAPLAYBACKTYPE_BINAURALSPACE 13

#define RWAPLAYBACKTYPE_CUSTOM1 14
#define RWAPLAYBACKTYPE_CUSTOM2 15
#define RWAPLAYBACKTYPE_CUSTOM3 16

#define RWAASSETATTRIBUTE_ISEXCLUSIVE 1
#define RWAASSETATTRIBUTE_PLAYONCE 2
#define RWAASSETATTRIBUTE_ISACTIVE 3
#define RWAASSETATTRIBUTE_ISALIVE 4
#define RWAASSETATTRIBUTE_ISBLOCKED 5
#define RWAASSETATTRIBUTE_LOOP 6
#define RWAASSETATTRIBUTE_ELEVATION2PD 7
#define RWAASSETATTRIBUTE_ORIENTATION2PD 8
#define RWAASSETATTRIBUTE_RAWSENSORS2PD 9
#define RWAASSETATTRIBUTE_GPS2PD 10
#define RWAASSETATTRIBUTE_AUTOROTATE 11
#define RWAASSETATTRIBUTE_MOVEFROMSTARTPOSITION 12
#define RWAASSETATTRIBUTE_AUTOMOVE 12
#define RWAASSETATTRIBUTE_LOOPUNTILENDPOSITION 13
#define RWAASSETATTRIBUTE_DISTANCE2VOLUME 14
#define RWAASSETATTRIBUTE_MUTE 15
#define RWAASSETATTRIBUTE_HEADTRACKERRELATIVE2SOURCE 16
#define RWAASSETATTRIBUTE_LOCKPOSITION 17
#define RWAASSETATTRIBUTE_ALLOWINDIVIDUELLCHANNELPOSITIONS 18
#define RWAASSETATTRIBUTE_ALWAYSPLAYFROMBEGINNING 19

using namespace std;

class RwaScene;
class RwaState;

class RwaAsset1: public RwaLocation1
{

public:
    explicit RwaAsset1(const std::string &data, std::vector<double> gps = std::vector<double>(2,0.0), int32_t type = 0, const string uid = "");
    ~RwaAsset1();

    vector<double> channelcoordinates[64]; // limited to 64 channels, maximum for soundfiles
    vector<double> reflectioncoordinates[64]; //
    int32_t currentReflection;
    float channelDistance[64];
    float channelBearing[64];
    float lastChannelBearing[64];
    float channelRotateFreq[64];
    float channelGain[64];
    bool individuellChannelPosition[64];    
    bool reflectionCoordinateIsSet[64];
    bool channelRotate[64];

    RwaState *myState;
    RwaScene *myScene;

    void moveMyChildren(double dx, double dy);
    void copyAttributes(RwaAsset1 *dest);
    void calculateChannelPositions();

    void setChannelCoordinate(int32_t channelNumber, std::vector<double> coordinate);
    void setReflectionCoordinate(int32_t channelNumber, std::vector<double> coordinate);
    void setIndividuellChannelPosition(int32_t channel, std::vector<double> position);
    void setChannelGain(int32_t channel, float gain);
    void setChannelRotateFrequency(int channel, float frequency);
    void setIndividualChannelRotateFrequency(int32_t channel, float frequency);

    string getFileName() const;
    void setFileName(const string &value);

    string getFullPath() const;
    void setFullPath(const string &value);

    string getUniqueId() const;
    void setUniqueId(const string &value);

    string getReceiverPatcher() const;
    void setReceiverPatcher(const string &value);

    vector<double> getStartPosition() const;
    void setStartPosition(const vector<double> &value);

    vector<double> getCurrentPosition() const;
    void setCurrentPosition(const vector<double> &value);

    float getChannelRadius() const;
    void setChannelRadius(float value);

    float getCurrentRotateAngleOffset() const;
    void setCurrentRotateAngleOffset(const float &value);

    float getWaitTimeBeforeMovement() const;
    void setWaitTimeBeforeMovement(float value);

    float getMovementSpeed() const;
    void setMovementSpeed(float value);

    float getMovingDistancePerTick() const;
    void setMovingDistancePerTick(float value);

    float getRotateOffsetPerTick() const;
    void setRotateOffsetPerTick(float value);

    float getExitStateAfter() const;
    void setExitStateAfter(float value);

    float getRotateFrequency() const;
    void setRotateFrequency(float value);

    float getGain() const;
    void setGain(float value);

    float getFixedAzimuth() const;
    void setFixedAzimuth(float value);

    float getFixedDistance() const;
    void setFixedDistance(float value);

    float getFixedElevation() const;
    void setFixedElevation(float value);

    float getOffset() const;
    void setOffset(float value);

    float getDampingFactor() const;
    void setDampingFactor(float value);

    float getDampingTrim() const;
    void setDampingTrim(float value);

    float getDampingMin() const;
    void setDampingMin(float value);

    float getDampingMax() const;
    void setDampingMax(float value);

    float getMinDistance() const;
    void setMinDistance(float value);

    int64_t getNumberOfChannels() const;
    void setNumberOfChannels(const int64_t &value);

    int64_t getDuration() const;
    void setDuration(const int64_t &value);

    int64_t getFadeOutAfter() const;
    void setFadeOutAfter(const int64_t &value);

    int32_t getPlaybackType() const;
    void setPlaybackType(const int32_t &value);

    int32_t getFadeOutTime() const;
    void setFadeOutTime(const qint32 &value);

    int32_t getFadeInTime() const;
    void setFadeInTime(const int32_t &value);

    int32_t getCrossfadeTime() const;
    void setCrossfadeTime(const int32_t &value);

    int32_t getDampingFunction() const;
    void setDampingFunction(const int32_t &value);

    int32_t getRotateOffset() const;
    void setRotateOffset(int32_t value);

    int32_t getType() const;
    void setType(const int32_t &value);

    bool getSurviveAfterLeavingState() const;
    void setSurviveAfterLeavingState(bool value);

    bool getLoopUntilEndPosition() const;
    void setLoopUntilEndPosition(bool value);

    bool getFollowEntity() const;
    void setFollowEntity(bool value);

    bool getReachedEndPosition() const;
    void setReachedEndPosition(bool value);

    bool getLockPosition() const;
    void setLockPosition(bool value);

    bool individuellChannelPositionsAllowed() const;
    void setAllowIndividuellChannelPositions(bool value);

    bool getAlwaysPlayFromBeginning() const;
    void setAlwaysPlayFromBeginning(bool value);

    bool getMute() const;
    void setMute(bool value);

    bool getHeadtrackerRelative2Source() const;
    void setHeadtrackerRelative2Source(bool value);

    bool getGps2pd() const;
    void setGps2pd(bool value);

    bool getRawSensors2pd() const;
    void setRawSensors2pd(bool value);

    bool getBlocked() const;
    void setBlocked(bool value);

    bool getLoop() const;
    void setLoop(bool value);

    bool getIsAlive() const;
    void setIsAlive(bool value);

    bool getIsActive() const;
    void setIsActive(bool value);

    bool getIsExclusive() const;
    void setIsExclusive(bool value);

    bool getAllowTouches() const;
    void setAllowTouches(bool value);

    bool getAutoRotate() const;
    void setAutoRotate(bool value);

    bool getPlayOnlyOnce() const;
    void setPlayOnlyOnce(bool value);

    bool getMoveFromStartPosition() const;
    void setMoveFromStartPosition(bool value);

    void calculateReflectionPositions();

    int32_t getReflectionCount() const;
    void setReflectionCount(const int32_t &value);

    bool getHasCoordinates() const;
private:
    friend class RwaRuntime;

    string uniqueId;
    string fullPath;
    string fileName;

    vector<double> currentPosition; // variable postion if asset is moving
    vector<double> startPosition; // start postion, end position/map position is inherited from rwalocation

    float elevation = 0;
    float dampingFactor = 30;
    float dampingTrim = 2;
    float dampingMin = 0;
    float dampingMax = 1;
    float playheadPosition = 0;
    float playheadPositionWithoutOffset = 0;
    float followEntityThreshhold = 4;  // at what distance to entiy start following
    float rotateFrequency = 0;
    float gain = 1;
    float channelRadius = 20;
    float distanceForMovement = 0;
    float bearingForMovement = 0;
    float movingDistancePerTick = 0;
    float rotateOffsetPerTick = 0;
    float fixedAzimuth = -1;
    float fixedElevation = -1;
    float fixedDistance = -1;
    float offset = 0;
    float minDistance = -1;
    float maxDistance = 50;
    float currentRotateAngleOffset = 0;
    float movementSpeed = 20;
    float waitTimeBeforeMovement = 0;

    int32_t type;
    int32_t playbackType = -1; // binaural, stereo, mono, etc..
    int32_t dampingFunction = 1;
    int32_t fadeOutTime = 50;
    int32_t fadeInTime = 50;
    int64_t crossfadeTime = 2000;
    int32_t duration = 0;
    int64_t fadeOutAfter = 0;
    int32_t numberOfChannels = 0;
    int32_t rotateOffset = 0;
    int32_t timeOut = 0;
    int32_t reflectionCount = 0;

    bool mute = false;
    bool isExclusive = false; // no other asset at the same time
    bool isActive = false;    // currently active
    bool isAlive = true;     // can be activated (in principle)
    bool loop = false;        // start again automatically while within state radius
    bool blocked = false;     // blocked, can't be activated; for example: blocked by another client..
    bool headtrackerRelative2Source = true;
    bool lockPosition = false;
    bool allowIndividuellChannelPositions = false;
    bool rawSensors2pd = false;
    bool gps2pd = false;
    bool hasCoordinates = false; // if, else use state coordinates
    bool followEntity = false;   // always stays at the same distance to entity
    bool playOnlyOnce = false;
    bool allowTouches = false;
    bool autoRotate = false;
    bool moveFromStartPosition = false;
    bool reachedEndPosition = true;
    bool loopUntilEndPosition = false;
    bool surviveAfterLeavingState = false; // for following assets
    bool alwaysPlayFromBeginning = true;
    bool updatePlayheadPosition = true;
};

#endif // RWAASSETITEM_H
