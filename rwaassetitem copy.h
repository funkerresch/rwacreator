#ifndef RWAASSETITEM_H
#define RWAASSETITEM_H

#include <QObject>
#include <QPointF>
#include <QUuid>
#include "rwautilities.h"
#include "rwalocation.h"

#define RWA_UNDETERMINED 0

#define RWAASSETTYPE_WAV 1
#define RWAASSETTYPE_AIF 2
#define RWAASSETTYPE_PD 3

#define RWAPOSITIONTYPE_ASSET 1
#define RWAPOSITIONTYPE_ASSETCHANNEL 2
#define RWAPOSITIONTYPE_ASSETSTARTPOINT 3
#define RWAPOSITIONTYPE_CURRENTASSETPOSITION 4

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


class RwaScene;
class RwaState;

class RwaAssetItem: public RwaLocation
{
    Q_OBJECT

public:
    explicit RwaAssetItem(const QString &data, QPointF coordinates = QPointF(), qint32 type = 0, qint32 id = 0, QObject *parent = 0);

    void unblock();

    qint32 type;
    qint32 playbackType; // binaural, stereo, mono, etc..
    qint32 dampingFunction;
    float dampingFactor;
    float dampingTrim;
    float dampingMin;
    float dampingMax;
    QUuid uniqueId;
    qint32 fadeOutTime;
    qint32 fadeInTime;
    qint32 crossfadeTime;
    qint64 duration;
    qint64 fadeOutAfter;
    qint64 numberOfChannels;

    bool mute;
    bool isExclusive; // no other asset at the same time
    bool isActive;    // currently active
    bool isAlive;     // can be activated (in principle)
    bool loop;        // start again automatically while within state radius
    bool blocked;     // blocked, can't be activated; for example: blocked by another client..
    bool headtrackerRelative2Source;
    bool lockPosition;

    bool rawSensors2pd;
    bool gps2pd;
    bool hasCoordinates; // if, else use state coordinates
    bool followEntity;   // always stays at the same distance to entity
    bool playOnlyOnce;
    bool allowTouches;
    bool autoRotate;
    bool moveFromStartPosition;
    bool reachedEndPosition;
    bool loopUntilEndPosition;
    bool surviveAfterLeavingState; // for following assets

    float followEntityThreshhold;  // at what distance to entiy start following
    float rotateFrequency;
    float gain;
    float channelRadius;
    int rotateOffset;
    float distanceForMovement;
    float bearingForMovement;
    float movingDistancePerTick;
    float rotateOffsetPerTick;
    float exitStateAfter;
    float fixedAzimuth;
    float fixedElevation;
    float fixedDistance;
    float offset;
    float minDistance;
    float maxDistance;

    qint32 timeOut;   // timeOut in ms
    float currentRotateAngleOffset;
    QString fullPath;
    QString fileName;
    QString receiverPatcher;
    QPointF currentPosition;
    QPointF startPosition;
    float movementSpeed;
    float waitTimeBeforeMovement;

    QPointF channelcoordinates[64]; // limited to 64 channels for now...
    float channelDistance[64];
    float channelBearing[64];
    float lastChannelBearing[64];
    bool individuellChannelPosition[64];
    float channelRotateFreq[64];
    float channelGain[64];
    bool channelRotate[64];

    float elevation;

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

    qint32 getPlaybackType() const;
    void setPlaybackType(const qint32 &value);

    qint32 getFadeOutTime() const;
    void setFadeOutTime(const qint32 &value);

    qint32 getFadeInTime() const;
    void setFadeInTime(const qint32 &value);

    qint32 getCrossfadeTime() const;
    void setCrossfadeTime(const qint32 &value);

    float getGain() const;
    void setGain(float value);

    bool getPlayOnlyOnce() const;
    void setPlayOnlyOnce(bool value);

    qint64 getDuration() const;
    void setDuration(const qint64 &value);

    qint64 getFadeOutAfter() const;
    void setFadeOutAfter(const qint64 &value);

    float getMultichannelsourceradius() const;
    void setChannelRadius(float value);

    void calculateAssetChannelParameters(int channel, int bearing);
    void calculateChannelPositions();

    qint64 getNumberOfChannels() const;
    void setNumberOfChannels(const qint64 &value);

    QUuid getUniqueId() const;

    bool getAllowTouches() const;
    void setAllowTouches(bool value);

    bool getAutoRotate() const;
    void setAutoRotate(bool value);

    float getRotateFrequency() const;
    void setRotateFrequency(float value);

    void setChannelCoordinate(int channelNumber, QPointF coordinate);
    float getCurrentRotateAngleOffset() const;
    void setCurrentRotateAngleOffset(const float &value);

    void setIndividuellChannelPosition(int channel, QPointF position);
    void setChannelGain(int channel, float gain);
    void setChannelRotateFrequency(int channel, float frequency);
    float getMovementSpeed() const;
    void setMovementSpeed(float value);

    float getWaitTimeBeforeMovement() const;
    void setWaitTimeBeforeMovement(float value);

    bool getMoveFromStartPosition() const;
    void setMoveFromStartPosition(bool value);

    QPointF getStartPosition() const;
    void setStartPosition(const QPointF &value);

    void setIndividualChannelRotateFrequency(int channel, float frequency);
    QPointF getCurrentPosition() const;
    void setCurrentPosition(const QPointF &value);

    float getMovingDistancePerTick() const;
    void setMovingDistancePerTick(float value);

    float getRotateOffsetPerTick() const;
    void setRotateOffsetPerTick(float value);

    float getExitStateAfter() const;
    void setExitStateAfter(float value);

    bool getSurviveAfterLeavingState() const;
    void setSurviveAfterLeavingState(bool value);

    bool getLoopUntilEndPosition() const;
    void setLoopUntilEndPosition(bool value);

    bool getFollowEntity() const;
    void setFollowEntity(bool value);

    bool getReachedEndPosition() const;
    void setReachedEndPosition(bool value);

    float getChannelRadius() const;

    float getFixedAzimuth() const;
    void setFixedAzimuth(float value);

    float getFixedDistance() const;
    void setFixedDistance(float value);

    //bool getDistance2volume() const;
    //void setDistance2volume(bool value);

    float getFixedElevation() const;
    void setFixedElevation(float value);

    float getOffset() const;
    void setOffset(float value);

    qint32 getDampingFunction() const;
    void setDampingFunction(const qint32 &value);

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

    int getRotateOffset() const;
    void setRotateOffset(int value);

    bool getLockPosition() const;
    void setLockPosition(bool value);


    RwaState *myState;
    RwaScene *myScene;

    QString getReceiverPatcher() const;
    void setReceiverPatcher(const QString &value);

signals:

public slots:

    void copyAttributes(RwaAssetItem *dest);
};

#endif // RWAASSETITEM_H
