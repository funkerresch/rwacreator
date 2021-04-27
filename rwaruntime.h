#ifndef RWARUNTIME_H
#define RWARUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mutex>
#include <iostream>
#include <sstream>
#include <string>
#include "rwaentity.h"
#include "z_libpd.h"
#include "m_pd.h"
#include "util/z_queued.h"
#include "rwa_binauralsimple~.h"
#include "freeverb~.h"


#ifdef QT_VERSION
#include <QDebug>
#endif

#define RWARUNTIME_MAXNUMBEROFPATCHERS 20
#define RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS 2
#define RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS 2

#define RWARUNTIME_INVALIDPATCHERINDEX -1

struct pdPatcher {
    void *patcherTag;
    void *playFinishedReceiver;
    std::string name;
    bool isBusy;
};

#ifdef QT_VERSION
class RwaRuntime : public QObject
#else
class RwaRuntime
#endif
{
#ifdef QT_VERSION
    Q_OBJECT
#endif

public:

#ifdef QT_VERSION
RwaRuntime(QObject *parent, const char *pdpath, const char *assetPath, float sampleRate, float schedulerRate, std::mutex *pdMutex = nullptr);
#else
    RwaRuntime(const char *pdpath, const char *assetPath, float sampleRate, float schedulerRate, std::mutex *pdMutex = nullptr);
#endif
    ~RwaRuntime();

    std::string pdPath = "";
    std::string assetPath = "";
    std::mutex *pdMutex = nullptr;
    float sampleRate = 44.1f;
    float schedulerRate = 0;

    static bool debug;
    static std::list<RwaEntity *> entities;

    bool step = false;

    static pdPatcher binauralStereoPatchers_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
    static pdPatcher binauralMonoPatchers_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
    static pdPatcher binaural5channelPatchers_fabian[RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS];
    static pdPatcher binaural7channelPatchers_fabian[RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS];
    static pdPatcher stereoPatchers[RWARUNTIME_MAXNUMBEROFPATCHERS];
    static pdPatcher monoPatchers[RWARUNTIME_MAXNUMBEROFPATCHERS];

    static std::list<pdPatcher *> dynamicPatchers1;

    static float assetChannelCount;
    static float assetDuration;
    static float assetSampleRate;

    static void printpd(const char *s);
    static void bangpdHelp(int32_t patcherTag, std::map<string, RwaEntity::AssetMapItem> &assetItemMap);
    static void bangpd(const char *source);
    static void floatpd(const char *source, float value);
    static void releasePatcherFromItem(RwaEntity::AssetMapItem item);

    static int32_t getBinauralMonoFabianPatcherIndex(int32_t patcherTag);
    static int32_t getBinauralStereoFabianPatcherIndex(int32_t patcherTag);
    static int32_t getBinaural5channelFabianPatcherIndex(int32_t patcherTag);
    static int32_t getBinaural7channelFabianPatcherIndex(int32_t patcherTag);

    static int32_t getStereoPatcherIndex(int32_t patcherTag);
    static int32_t getMonoPatcherIndex(int32_t patcherTag);
    static int32_t getDynamicPatcherIndex(int32_t patcherTag);

    void initDynamicPdPatchers(RwaEntity *entitiy);
    void freeDynamicPdPatchers1();
    void *findFreeDynamicPatcher(RwaAsset1 *asset);
    void *findFreeBinauralMonoFabianPatcher();
    void *findFreeBinauralStereoFabianPatcher();
    void *findFreeBinaural5channelFabianPatcher();
    void *findFreeBinaural7channelFabianPatcher();
    void *findFreeStereoPatcher();
    void *findFreeMonoPatcher();
    void freeAllPatchers();
    void endBackgroundState();

    void sendEnd2activeAssets(RwaEntity *entity);
    void unblockAssets(RwaState *state);
    void unblockStates(RwaEntity *entitiy);
    void processAssets(RwaEntity *entity);
    int32_t findFreePatcher(RwaAsset1 *asset);
    static pdPatcher *findDynamicPatcher(int32_t patcherTag);

    void sendInitValues2pd(RwaAsset1 *asset, int patcherTag);
    void update(RwaEntity *entity);
    void sendData2activeAssets(RwaEntity *entity);
    void sendData2Asset(RwaEntity *entity, RwaEntity::AssetMapItem item);
    void calculateChannelBearingAndDistance(RwaEntity *entity, RwaAsset1 *asset, int channel);
    int getOffsetForChannel(int channel, int playbackType);
    void sendDistance(int channel, int patcherTag, float distance);
    void sendBearing(int channel, int patcherTag, float bearing);
    void sendElevation(int channel, int patcherTag, float elevation);
    void setEntityState(RwaEntity *entity);
    bool entityIsWithinArea(RwaEntity *entity, RwaArea *area, int offsetType);
    void setEntityScene(RwaEntity *entity);
    void sendEnd2backgroundAssets(RwaEntity *entity);
    void setEntityStartCoordinates(RwaEntity *entity);
    void startBackgroundState(RwaEntity *entity);

    void createAndBindPlayFinishedReceiver(pdPatcher *patcher);
    void openFile4MetaData(const char *fileName);

signals:
    void sendRedrawAssets();
    void sendSelectedScene(RwaScene *scene);
    void sendSelectedState(RwaState *state);
};

#endif // RWARUNTIME_H
