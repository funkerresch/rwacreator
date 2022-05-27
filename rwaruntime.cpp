#include "rwaruntime.h"
#ifdef QT_VERSION
#include "rwabackend.h"
#endif

std::list <RwaEntity *> RwaRuntime::entities;
bool RwaRuntime::debug;

float RwaRuntime::assetDuration;
float RwaRuntime::assetChannelCount;
float RwaRuntime::assetSampleRate;

pdPatcher RwaRuntime::binauralStereoPatchers_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::binauralMonoPatchers_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::binaural5channelPatchers_fabian[RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS];
pdPatcher RwaRuntime::binaural7channelPatchers_fabian[RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS];
pdPatcher RwaRuntime::stereoPatchers[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::monoPatchers[RWARUNTIME_MAXNUMBEROFPATCHERS];

std::list<pdPatcher *> RwaRuntime::dynamicPatchers1;
RwaBackend *RwaRuntime::backend;

#ifdef QT_VERSION
#define INIT_LIBPD_QUEUED
RwaRuntime::RwaRuntime(QObject *parent, const char *pdpath, const char *assetPath, float sampleRate, float schedulerRate, mutex *pdMutex, RwaBackend *_backend) :
    QObject(parent)
#else
RwaRuntime::RwaRuntime(const char *pdpath, const char *assetPath, float sampleRate, float schedulerRate, mutex *pdMutex)
#endif
{
    debug = true;
    this->pdPath = std::string(pdpath);
    this->assetPath = std::string(assetPath);
    this->sampleRate = sampleRate;
    this->schedulerRate = schedulerRate;
    this->pdMutex = pdMutex;
#ifdef QT_VERSION
    backend = _backend;
#endif

#ifdef INIT_LIBPD_QUEUED
    libpd_set_queued_printhook (static_cast<t_libpd_printhook>(RwaRuntime::printpd));
    libpd_set_queued_floathook (static_cast<t_libpd_floathook>(RwaRuntime::floatpd));
    libpd_set_queued_banghook (static_cast<t_libpd_banghook>(RwaRuntime::bangpd));
    libpd_queued_init();
#else
    libpd_set_printhook (static_cast<t_libpd_printhook>(RwaRuntime::printpd));
    libpd_set_floathook (static_cast<t_libpd_floathook>(RwaRuntime::floatpd));
    libpd_set_banghook (static_cast<t_libpd_banghook>(RwaRuntime::bangpd));
    libpd_init();
#endif
    rwa_binauralsimple_tilde_setup();
    freeverb_tilde_setup();
    oggread_tilde_setup();

    libpd_openfile("stereoout.pd", pdpath);
    libpd_openfile("rwagetmetadata.pd", pdpath);
    libpd_bind("duration4metadata");
    libpd_bind("channels4metadata");
    libpd_bind("samplerate4metadata");

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaloopplayermono.pd", pdpath);
        monoPatchers[i].patcherTag = d;
        monoPatchers[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&monoPatchers[i]);
    }

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaloopplayerstereo.pd", pdpath);
        stereoPatchers[i].patcherTag = d;
        stereoPatchers[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&stereoPatchers[i]);
    }

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaplayerstereobinaural_fabian.pd", pdpath);
        binauralStereoPatchers_fabian[i].patcherTag = d;
        binauralStereoPatchers_fabian[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&binauralStereoPatchers_fabian[i]);
    }

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaplayermonobinaural_fabian.pd", pdpath);
        binauralMonoPatchers_fabian[i].patcherTag = d;
        binauralMonoPatchers_fabian[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&binauralMonoPatchers_fabian[i]);
    }

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaplayer5_1channelbinaural_fabian.pd", pdpath);
        binaural5channelPatchers_fabian[i].patcherTag = d;
        binaural5channelPatchers_fabian[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&binaural5channelPatchers_fabian[i]);
    }

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaplayer7channelbinaural_fabian.pd", pdpath);
        binaural7channelPatchers_fabian[i].patcherTag = d;
        binaural7channelPatchers_fabian[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&binaural7channelPatchers_fabian[i]);
    }

    openFile4MetaData("unitclick.wav"); // first message gets lost somehow, sending an init dummy message
}

RwaRuntime::~RwaRuntime()
{

}

void RwaRuntime::openFile4MetaData(const char *fileName)
{
    pdMutex->lock();
    libpd_symbol("filename4metadata", fileName);
#ifdef INIT_LIBPD_QUEUED
    libpd_queued_receive_pd_messages();
#endif
    pdMutex->unlock();
}

void RwaRuntime::createAndBindPlayFinishedReceiver(pdPatcher *patcher)
{
    std::stringstream receiverStream;
    receiverStream << libpd_getdollarzero(patcher->patcherTag) << "-playfinished";
    patcher->playFinishedReceiver = libpd_bind(receiverStream.str().c_str());
}

void RwaRuntime::printpd(const char *s)
{
    if(!backend)
        return;

    if(backend->logPd)
        qDebug() << s;
}

void RwaRuntime::floatpd(const char *source, float value)
{
    if(!backend)
        return;

     if(!strcmp(source, "duration4metadata"))
        assetDuration = value;

     if(!strcmp(source, "channels4metadata"))
        assetChannelCount = value;

     if(!strcmp(source, "samplerate4metadata"))
        assetSampleRate = value;

     if(backend->logPd)
         qDebug() << source << " " << value;
}

pdPatcher *RwaRuntime::findDynamicPatcher(int32_t patcherTag)
{
    pdPatcher *patcher;
    foreach(patcher, dynamicPatchers1)
    {
        if(libpd_getdollarzero(patcher->patcherTag) == patcherTag)
            return patcher;
    }
    return nullptr;
}

void RwaRuntime::releasePatcherFromItem(RwaEntity::AssetMapItem item)
{
    RwaAsset1 *asset = item.getAssetItem();
    int32_t patcherTag = item.getPatcherTag();
    int32_t playbackType = asset->getPlaybackType();

    if(asset->type == RWAASSETTYPE_PD)
    {
        pdPatcher *patcher = findDynamicPatcher(patcherTag);
        if(patcher != nullptr)
            patcher->isBusy = false;
    }

    else
    {
        switch(playbackType)
        {
            case RWAPLAYBACKTYPE_NATIVE:
                if(asset->getNumberOfChannels() == 1)
                    monoPatchers[getMonoPatcherIndex(patcherTag)].isBusy = false;

                if(asset->getNumberOfChannels() == 2)
                    stereoPatchers[getStereoPatcherIndex(patcherTag)].isBusy = false;

                break;

            case RWAPLAYBACKTYPE_MONO:
                monoPatchers[getStereoPatcherIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_STEREO:
                stereoPatchers[getStereoPatcherIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_BINAURALMONO_FABIAN:
                binauralMonoPatchers_fabian[getBinauralMonoFabianPatcherIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN:
                binauralStereoPatchers_fabian[getBinauralStereoFabianPatcherIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN:
                binaural5channelPatchers_fabian[getBinaural5channelFabianPatcherIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN:
                binaural7channelPatchers_fabian[getBinaural7channelFabianPatcherIndex(patcherTag)].isBusy = false;
                break;

            default: break;
        }
    }
}

void RwaRuntime::bangpdHelp(int32_t patcherTag, std::map<string, RwaEntity::AssetMapItem> &assetItemMap)
{
    RwaAsset1 *assetItem;
    RwaEntity::AssetMapItem item;
    string key;

    std::map<string, RwaEntity::AssetMapItem>::iterator i = assetItemMap.begin();
    while(i != assetItemMap.end())
    {
        key = i->first;
        item = i->second;

        assetItem = item.getAssetItem();

        if(item.getPatcherTag() == patcherTag)
        {
            i = assetItemMap.erase(i);
            releasePatcherFromItem(item);
            if(backend->logSim)
                qDebug() <<  "Released Asset" << ": " << QString::fromStdString(assetItem->fileName);

            return;
        }
        else
            ++i;
    }
}

void RwaRuntime::bangpd(const char *source)
{
    int intPatcherTag;
    char receiveFromPd[100];
    char receiver[100];
    char patcherTag[100];
    char delimiter[] = "-";
    char *ptr;

    strcpy(receiveFromPd, source);
    ptr = strtok(receiveFromPd, delimiter);
    strcpy(patcherTag, ptr);
    ptr = strtok(nullptr, delimiter);
    strcpy(receiver, ptr);
    intPatcherTag = atoi(patcherTag);

    if(!strcmp(receiver, "playfinished"))
    {
        RwaEntity *entity;
        RwaEntity::AssetMapItem item;

        foreach(entity, entities)
        {
            bangpdHelp(intPatcherTag, entity->activeAssets);
            bangpdHelp(intPatcherTag, entity->backgroundAssets);
        }
    }
}

int32_t RwaRuntime::getBinauralStereoFabianPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(binauralStereoPatchers_fabian[i].patcherTag) == patcherTag)
            return i;
    }
    return RWARUNTIME_INVALIDPATCHERINDEX;
}

int32_t RwaRuntime::getBinaural5channelFabianPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS; i++)
    {
        if(libpd_getdollarzero(binaural5channelPatchers_fabian[i].patcherTag) == patcherTag)
            return i;
    }
    return RWARUNTIME_INVALIDPATCHERINDEX;
}

int32_t RwaRuntime::getBinaural7channelFabianPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS; i++)
    {
        if(libpd_getdollarzero(binaural7channelPatchers_fabian[i].patcherTag) == patcherTag)
            return i;
    }
    return RWARUNTIME_INVALIDPATCHERINDEX;
}

int32_t RwaRuntime::getBinauralMonoFabianPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(binauralMonoPatchers_fabian[i].patcherTag) == patcherTag)
            return i;
    }
    return RWARUNTIME_INVALIDPATCHERINDEX;
}

int32_t RwaRuntime::getStereoPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(stereoPatchers[i].patcherTag) == patcherTag)
            return i;
    }
    return RWARUNTIME_INVALIDPATCHERINDEX;
}

int32_t RwaRuntime::getMonoPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(monoPatchers[i].patcherTag) == patcherTag)
            return i;
    }
    return RWARUNTIME_INVALIDPATCHERINDEX;
}

void *RwaRuntime::findFreeDynamicPatcher(RwaAsset1 *asset)
{
    pdPatcher *patcher;
    foreach(patcher, dynamicPatchers1)
    {
        if( (asset->fileName == patcher->name) && !patcher->isBusy)
        {
             patcher->isBusy = true;

             if(backend->logSim)
                 qDebug() <<  "Found Dynamic PD Asset" << ": " << QString::fromStdString(patcher->name);

             return patcher->patcherTag;
        }
    }
    return nullptr;
}

void RwaRuntime::freeDynamicPdPatchers1()
{
    pdPatcher *patcher;

    std::list<pdPatcher *>::iterator i = dynamicPatchers1.begin();
    while(i != dynamicPatchers1.end())
    {
        libpd_unbind((*i)->playFinishedReceiver);
        libpd_closefile((*i)->patcherTag);
        patcher = *i;
        i = dynamicPatchers1.erase(i);
        delete patcher;
    }

    dynamicPatchers1.clear();

    if(backend->logSim)
        qDebug() <<  "Freed all dynamic Patchers";
}

void RwaRuntime::initDynamicPdPatchers(RwaEntity *entitiy)
{
    RwaScene *scene;
    RwaState *state;
    RwaAsset1 *asset;

    foreach(scene, entitiy->scenes)
    {
        foreach(state, scene->getStates())
        {
            foreach(asset, state->getAssets())
            {
                if(asset->type == RWAASSETTYPE_PD && !asset->mute)
                {   
                    void *d = libpd_openfile(asset->fileName.c_str(), assetPath.c_str());
                    if(d != nullptr)
                    {
                        pdPatcher *newPatcher = new pdPatcher();
                        newPatcher->patcherTag = d;
                        newPatcher->isBusy = false;
                        newPatcher->name = asset->fileName;
                        createAndBindPlayFinishedReceiver(newPatcher);
                        dynamicPatchers1.push_back(newPatcher);
                        if(backend->logSim)
                            qDebug() <<  "Initialize Pd Asset" << ": " << QString::fromStdString(newPatcher->name);
                    }
                }
            }
        }
    }
}

void RwaRuntime::sendEnd2backgroundAssets(RwaEntity *entity)
{
    char end2pd[40];
    int intPatcherTag;
    RwaEntity::AssetMapItem item;
    RwaAsset1 *assetItem;

    if(!entity->backgroundAssets.empty())
    {
        std::map<string, RwaEntity::AssetMapItem>::iterator i = entity->backgroundAssets.begin();
        while(i != entity->backgroundAssets.end())
        {
            item = i->second;
            assetItem = item.getAssetItem();
            intPatcherTag = item.getPatcherTag();
            sprintf(end2pd,"%d-end", intPatcherTag);

            if(pdMutex != nullptr)
                pdMutex->lock();
            libpd_bang(end2pd);
            if(pdMutex != nullptr)
                pdMutex->unlock();

            if(backend->logSim)
                qDebug() <<  "End background asset: "  << QString::fromStdString(assetItem->fileName);

            ++i;
        }
    }
}

void RwaRuntime::sendEnd2activeAssets(RwaEntity *entity)
{
    char end2pd[40];
    int intPatcherTag;
    RwaEntity::AssetMapItem item;
    RwaAsset1 *assetItem;

    if(!entity->activeAssets.empty())
    {
        std::map<string, RwaEntity::AssetMapItem>::iterator i = entity->activeAssets.begin();
        while(i != entity->activeAssets.end())
        {
            item = i->second;
            assetItem = item.getAssetItem();
            intPatcherTag = item.getPatcherTag();
            sprintf(end2pd,"%d-end", intPatcherTag);

            if(pdMutex != nullptr)
                pdMutex->lock();
            libpd_bang(end2pd);
            if(pdMutex != nullptr)
                pdMutex->unlock();

            if(backend->logSim)
                qDebug() <<  "End active asset: "  << QString::fromStdString(assetItem->fileName);

            ++i;
        }
    }

    if(!entity->assets2unblock.empty()) // assets2unblock contains assets, that are finished but are not supposed to start again within the same state
    {
        foreach(assetItem, entity->assets2unblock)
        {
            assetItem->setBlocked(false);
            assetItem->setReachedEndPosition(false);
        }

        entity->clearAssets2unblock();
    }
}

void RwaRuntime::unblockAssets(RwaState *state)
{
    RwaAsset1 *item;
    foreach(item, state->getAssets())
        item->setBlocked(false);
}

void RwaRuntime::unblockStates(RwaEntity *entity)
{
    RwaState *state;
    RwaScene *scene;
    RwaAsset1 *asset;

    foreach(scene, entity->scenes)
    {
        foreach(state, scene->getStates())
        {
            state->setBlockUntilRadiusHasBeenLeft(false);
            foreach(asset, state->getAssets())
            {
                asset->playheadPosition = 0;
                asset->playheadPositionWithoutOffset = 0;
                asset->updatePlayheadPosition = true;
            }
        }
    }
}

void *RwaRuntime::findFreeBinauralMonoFabianPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!binauralMonoPatchers_fabian[i].isBusy)
        {
            binauralMonoPatchers_fabian[i].isBusy = true;

            return binauralMonoPatchers_fabian[i].patcherTag;

        }
    }
    return nullptr;
}

void *RwaRuntime::findFreeBinauralStereoFabianPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!binauralStereoPatchers_fabian[i].isBusy)
        {
            binauralStereoPatchers_fabian[i].isBusy = true;
            return binauralStereoPatchers_fabian[i].patcherTag;
        }
    }
    return nullptr;
}

void *RwaRuntime::findFreeBinaural5channelFabianPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS; i++)
    {
        if(!binaural5channelPatchers_fabian[i].isBusy)
        {
            binaural5channelPatchers_fabian[i].isBusy = true;
            return binaural5channelPatchers_fabian[i].patcherTag;
        }
    }
    return nullptr;
}

void *RwaRuntime::findFreeBinaural7channelFabianPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS; i++)
    {
        if(!binaural7channelPatchers_fabian[i].isBusy)
        {
            binaural7channelPatchers_fabian[i].isBusy = true;
            return binaural7channelPatchers_fabian[i].patcherTag;
        }
    }
    return nullptr;
}

void *RwaRuntime::findFreeStereoPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!stereoPatchers[i].isBusy)
        {
            stereoPatchers[i].isBusy = true;
            return stereoPatchers[i].patcherTag;
        }
    }
    return nullptr;
}

void *RwaRuntime::findFreeMonoPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!monoPatchers[i].isBusy)
        {
            monoPatchers[i].isBusy = true;
            return monoPatchers[i].patcherTag;
        }
    }
    return nullptr;
}

int32_t RwaRuntime::findFreePatcher(RwaAsset1 *asset)
{
    if(asset->type == RWAASSETTYPE_PD)
        return libpd_getdollarzero(findFreeDynamicPatcher(asset));

    else
    {
        switch(asset->playbackType)
        {
            case RWAPLAYBACKTYPE_MONO: return libpd_getdollarzero(findFreeMonoPatcher());

            case RWAPLAYBACKTYPE_STEREO: return libpd_getdollarzero(findFreeStereoPatcher());

            case RWAPLAYBACKTYPE_BINAURALMONO_FABIAN: return libpd_getdollarzero(findFreeBinauralMonoFabianPatcher());

            case RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN: return libpd_getdollarzero(findFreeBinauralStereoFabianPatcher());

            case RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN: return libpd_getdollarzero(findFreeBinaural5channelFabianPatcher());

            case RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN: return libpd_getdollarzero(findFreeBinaural7channelFabianPatcher());

            case RWAPLAYBACKTYPE_NATIVE:
            {
                if(asset->getNumberOfChannels() == 1)
                    return libpd_getdollarzero(findFreeMonoPatcher());

                if(asset->getNumberOfChannels() == 2)
                    return libpd_getdollarzero(findFreeStereoPatcher());

                break;
            }

            case RWAPLAYBACKTYPE_BINAURALAUTO:
            {
                if(asset->getNumberOfChannels() == 1)
                    return libpd_getdollarzero(findFreeBinauralMonoFabianPatcher());
                if(asset->getNumberOfChannels() == 2)
                    return libpd_getdollarzero(findFreeBinauralStereoFabianPatcher());
                if(asset->getNumberOfChannels() == 5)
                    return libpd_getdollarzero(findFreeBinaural5channelFabianPatcher());
                if(asset->getNumberOfChannels() == 7)
                    return libpd_getdollarzero(findFreeBinaural7channelFabianPatcher());

                break;
            }

            default: return RWARUNTIME_INVALIDPATCHERINDEX;
        }
        return RWARUNTIME_INVALIDPATCHERINDEX;
    }
}

void RwaRuntime::sendInitValues2pd(RwaAsset1 *asset, int patcherTag)
{
    if(backend->logSim)
        qDebug();

    char pdReceiver[50];
    std::ostringstream fullAssetPath;
    float firstCrossfadeAfter = asset->getFadeOutAfter();
    asset->playheadPositionWithoutOffset = 0;
    asset->updatePlayheadPosition = true;

    if(asset->alwaysPlayFromBeginning)
        asset->playheadPosition = 0;
    else
    {
        firstCrossfadeAfter-= (asset->playheadPosition/44.1);

        if(firstCrossfadeAfter <0)
        {
            firstCrossfadeAfter = asset->getFadeOutAfter();
            asset->playheadPosition = 0;
        }
    }

    if(asset->getMoveFromStartPosition())
    {
        asset->setCurrentPosition(asset->getStartPosition());
        asset->setReachedEndPosition(false);
    }
    else
    {
        asset->setCurrentPosition(asset->getCoordinates());
        asset->setReachedEndPosition(true);
    }

     asset->distanceForMovement = RwaUtilities::calculateDistance1(asset->startPosition, asset->getCoordinates()) * 1000;
     asset->movingDistancePerTick = asset->getMovementSpeed() * ((float)schedulerRate/1000.);
     asset->rotateOffsetPerTick = asset->rotateFrequency * 360 * schedulerRate/1000;
     asset->setCurrentRotateAngleOffset(0);

     sprintf (pdReceiver, "%d-dampingfunction", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getDampingFunction());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-dampingfactor", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getDampingFactor());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-dampingtrim", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getDampingTrim());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-dampingmin", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getDampingMin());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-dampingmax", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getDampingMax());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-offset", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getOffset());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-loop", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getLoop());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-fadeintime", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->fadeInTime);
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-fadeouttime", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->fadeOutTime);
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-crossfadetime", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getCrossfadeTime());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-crossfadeafter", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getFadeOutAfter());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-firstcrossfade", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, firstCrossfadeAfter);
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-playheadposition", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->playheadPosition);
     pdMutex->unlock();

     fullAssetPath << assetPath << asset->fileName;
     sprintf(pdReceiver,"%d-play", patcherTag);
     pdMutex->lock();
     libpd_symbol(pdReceiver, fullAssetPath.str().c_str());
     pdMutex->unlock();
}

void RwaRuntime::processAssets(RwaEntity *entity)
{
    RwaAsset1 *asset;
    int patcherTag;

    if(entity->getCurrentState() == nullptr)
        return;

    foreach(asset, entity->getCurrentState()->getAssets())
    {
        if(!entity->isActiveAsset(asset->uniqueId) && !asset->getBlocked() && !asset->mute)
        {
             patcherTag = findFreePatcher(asset);
             sendInitValues2pd(asset, patcherTag);
             entity->addActiveAsset(asset->uniqueId, asset, patcherTag);

             if(backend->logSim)
                qDebug() << "Add Active Asset: " << QString::fromStdString(asset->fileName);
             break;
        }
    }
}

int RwaRuntime::getOffsetForChannel(int channel, int playbackType)
{
    switch (playbackType) {
    case RWAPLAYBACKTYPE_BINAURALMONO:
        return 0;

    case RWAPLAYBACKTYPE_BINAURALMONO_FABIAN:
        return 0;

    case RWAPLAYBACKTYPE_BINAURALSTEREO:
        if(channel == 0)
            return -60;
        if(channel == 1)
            return 60;
        break;

    case RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN:
        if(channel == 0)
            return -60;
        if(channel == 1)
            return 60;
        break;

    case RWAPLAYBACKTYPE_BINAURAL5CHANNEL:
        if(channel == 0)
            return -60;
        if(channel == 1)
            return 0;
        if(channel == 2)
            return 60;
        if(channel == 3)
            return -120;
        if(channel == 4)
            return 120;
        break;

    case RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN:
        if(channel == 0)
            return -60;
        if(channel == 1)
            return 0;
        if(channel == 2)
            return 60;
        if(channel == 3)
            return -120;
        if(channel == 4)
            return 120;
        break;

    case RWAPLAYBACKTYPE_BINAURALAUTO:

        break;
    case RWAPLAYBACKTYPE_STEREO:

        break;
    default:
        return 0;//
    }
    return 0;
}

void RwaRuntime::calculateChannelBearingAndDistance(RwaEntity *entity, RwaAsset1 *asset, int channel)
{
    int offset = getOffsetForChannel(channel, asset->getPlaybackType());
    offset += (360-asset->getRotateOffset()) % 360;
    QPointF tmp;
    QPointF tmp2;

    if(!asset->individuellChannelPosition[channel])
    {
        tmp2 = QPointF(asset->getCurrentPosition()[0], asset->getCurrentPosition()[1]);
        tmp = RwaUtilities::calculateDestination(tmp2, asset->getChannelRadius(), (qint32)(offset+asset->currentRotateAngleOffset)%360);
        asset->channelcoordinates[channel][0] = tmp.x();
        asset->channelcoordinates[channel][1] = tmp.y();
    }
    if(asset->getFixedDistance() < 0)
    {
        if(asset->minDistance == -1)
        {
            asset->channelDistance[channel] = RwaUtilities::calculateDistance1(entity->getCoordinates(), asset->channelcoordinates[channel])*1000 + asset->minDistance; // we do not want assets moving through us
        }
        else
        {
            asset->channelDistance[channel] = RwaUtilities::calculateDistance1(entity->getCoordinates(), asset->channelcoordinates[channel])*1000;
            if(asset->channelDistance[channel] < asset->minDistance)
                asset->channelDistance[channel] = asset->minDistance;
        }
    }
    else
        asset->channelDistance[channel] = asset->getFixedDistance();

    if(asset->getFixedAzimuth() < 0)
    {
        asset->lastChannelBearing[channel] = asset->channelBearing[channel];
        asset->channelBearing[channel] = RwaUtilities::calculateBearing1(entity->getCoordinates(), asset->channelcoordinates[channel], entity->azimuth());
       // qDebug() << asset->channelBearing[channel];
    }
    else
        asset->channelBearing[channel] = asset->getFixedAzimuth() + offset;
}

void RwaRuntime::sendDistance(int channel, int patcherTag, float distance)
{
    char distance2pd[20];
    sprintf(distance2pd,"%d-distance%d", patcherTag, channel+1);
    pdMutex->lock();
    libpd_float(distance2pd, distance);
    pdMutex->unlock();
}

void RwaRuntime::sendBearing(int channel, int patcherTag, float bearing)
{
    char azimuth2pd[20];
    sprintf(azimuth2pd,"%d-azimuth%d", patcherTag, channel+1);
    pdMutex->lock();
    libpd_float(azimuth2pd, bearing);
    pdMutex->unlock();
}

void RwaRuntime::sendElevation(int channel, int patcherTag, float elevation)
{
    char elevation2pd[20];
    sprintf(elevation2pd,"%d-elevation%d", patcherTag, channel+1);
    pdMutex->lock();
    libpd_float(elevation2pd, elevation);
    pdMutex->unlock();
}

void RwaRuntime::sendData2Asset(RwaEntity *entity, RwaEntity::AssetMapItem item)
{
    RwaAsset1 *asset;

    int intPatcherTag;
    char gain2pd[20];
    char lon2pd[20];
    char lat2pd[20];
    char step2pd[20];
    char end2pd[20];

    asset = item.getAssetItem();

    intPatcherTag = item.getPatcherTag();
    sprintf(gain2pd, "%d-gain", intPatcherTag);
    sprintf(lon2pd, "%d-lon", intPatcherTag);
    sprintf(lat2pd, "%d-lat", intPatcherTag);
    sprintf(step2pd, "%d-step", intPatcherTag);

    pdMutex->lock();
    libpd_float(gain2pd, asset->gain);
    libpd_float(lon2pd, entity->getCoordinates()[0]);
    libpd_float(lat2pd, entity->getCoordinates()[1]);
    pdMutex->unlock();

    asset->elevation = -entity->elevation();

    if(asset->type == RWAASSETTYPE_PD)
    {
        calculateChannelBearingAndDistance(entity, asset, 0);
        sendDistance(0, intPatcherTag, asset->channelDistance[0]);

        if(step)
        {
            pdMutex->lock();
            libpd_bang(step2pd);
            pdMutex->unlock();
        }

        if(asset->headtrackerRelative2Source)
        {
            sendBearing(0, intPatcherTag, asset->channelBearing[0]);
            sendElevation(0, intPatcherTag, asset->elevation);

        }
        else
        {
            sendBearing(0, intPatcherTag, entity->azimuth());
            sendElevation(0, intPatcherTag, entity->elevation());
        }
    }

    else
    {
        if(asset->playbackType == RWAPLAYBACKTYPE_BINAURALMONO
          || asset->playbackType == RWAPLAYBACKTYPE_BINAURALMONO_FABIAN
          || asset->playbackType == RWAPLAYBACKTYPE_MONO
          || asset->playbackType == RWAPLAYBACKTYPE_STEREO
          || asset->playbackType == RWAPLAYBACKTYPE_CUSTOM1
          || asset->playbackType == RWAPLAYBACKTYPE_CUSTOM2
          || asset->playbackType == RWAPLAYBACKTYPE_CUSTOM3 )
        {
            calculateChannelBearingAndDistance(entity, asset, 0);
            sendDistance(0, intPatcherTag, asset->channelDistance[0]);
            sendBearing(0, intPatcherTag, asset->channelBearing[0]);
            sendElevation(0, intPatcherTag, asset->elevation);
           // qDebug() << "Distance: " << asset->channelDistance[0];
            //qDebug() << "Bearing for libPd" << headtrackerX;
        }

        if(asset->playbackType == RWAPLAYBACKTYPE_BINAURALSTEREO
           || asset->playbackType == RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN)
        {
            for(int i = 0;i < 2; i++)
            {
                calculateChannelBearingAndDistance(entity, asset, i);
                sendDistance(i, intPatcherTag, asset->channelDistance[i]);
                sendBearing(i, intPatcherTag, asset->channelBearing[i]);
                sendElevation(i, intPatcherTag, asset->elevation);
               // qDebug() << asset->channelDistance[0];
            }
        }

        if(asset->playbackType == RWAPLAYBACKTYPE_BINAURAL5CHANNEL
           || asset->playbackType == RWAPLAYBACKTYPE_BINAURAL5CHANNEL_FABIAN)
        {
            for(int i = 0;i < 5; i++)
            {
                calculateChannelBearingAndDistance(entity, asset, i);
                sendDistance(i, intPatcherTag, asset->channelDistance[i]);
                sendBearing(i, intPatcherTag, asset->channelBearing[i]);
                sendElevation(i, intPatcherTag, asset->elevation);
            }
        }

        if(asset->playbackType == RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN)
        {
            for(int i = 0;i < 7; i++)
            {
                calculateChannelBearingAndDistance(entity, asset, i);
                sendDistance(i, intPatcherTag, asset->channelDistance[i]);
                sendBearing(i, intPatcherTag, asset->channelBearing[i]);
                sendElevation(i, intPatcherTag, asset->elevation);
            }
        }
    }

    if(!asset->getReachedEndPosition())
    {
        if(asset->distanceForMovement > 0)
        {
            asset->distanceForMovement -= asset->getMovingDistancePerTick();
            //qDebug() << asset->distanceForMovement;
            std::vector<double> tmp = RwaUtilities::calculateDestination1(asset->getCoordinates(), asset->distanceForMovement, asset->bearingForMovement);
            asset->setCurrentPosition(tmp );
        }
        else
        {
            asset->setReachedEndPosition(true);
            if(asset->getLoopUntilEndPosition())
            {
                asset->setBlocked(true);
                entity->appendAsset2unblock(asset);
                sprintf(end2pd, "%d-", intPatcherTag);
                strcat(end2pd, "end");
                pdMutex->lock();
                libpd_bang(end2pd);
                pdMutex->unlock();
            }
        }
    }

    if( ( (entity->getTimeInCurrentState() * 1000) > (asset->getDuration() + asset->getOffset()) ) && !asset->getLoop() && !asset->getBlocked() && !(asset->type == RWAASSETTYPE_PD))
    {
        asset->setBlocked(true);
        entity->appendAsset2unblock(asset);
        sprintf(end2pd, "%d-", intPatcherTag);
        strcat(end2pd, "end");
        pdMutex->lock();
        libpd_bang(end2pd);
        pdMutex->unlock();
    }

    if(asset->getAutoRotate())
    {
        asset->currentRotateAngleOffset += asset->getRotateOffsetPerTick();
        //qDebug() << "rotate: " << asset->getRotateOffsetPerTick();
    }

    if(asset->updatePlayheadPosition)
    {
        asset->playheadPositionWithoutOffset += schedulerRate;
        if(asset->playheadPositionWithoutOffset >= asset->offset)
            asset->playheadPosition += ((float)schedulerRate/1000. * 44100);

        if(asset->playheadPosition >= asset->fadeOutAfter/1000. * 44100)
        {
            asset->playheadPosition = 0;
            if(!asset->getLoop())
                asset->updatePlayheadPosition = false;

        }
        //qDebug() << asset->playheadPosition;
    }
}

void RwaRuntime::sendData2activeAssets(RwaEntity *entity)
{
    RwaState *entityState;
    RwaEntity::AssetMapItem item;
    string key;

    QPointF assetCoordinates;

    if(entities.empty())
        return;

    foreach(entity, entities)
    {
        entityState = entity->getCurrentState();
        if(entityState != NULL)
        {
            if(!entity->activeAssets.empty())
            {
                std::map<string, RwaEntity::AssetMapItem>::iterator i;
                for (i = entity->activeAssets.begin(); i != entity->activeAssets.end(); ++i)
                {
                    key = i->first;
                    item = i->second;

                    sendData2Asset(entity, item);
                }
                step = 0;
            }

            if(!entity->backgroundAssets.empty())
            {
                std::map<string, RwaEntity::AssetMapItem>::iterator i = entity->backgroundAssets.begin();
                while(i != entity->backgroundAssets.end())
                {
                    key = i->first;
                    item = i->second;
                    sendData2Asset(entity, item);
                    ++i;
                }
                step = 0;
            }
        }
    }
    emit sendRedrawAssets();
}

bool RwaRuntime::entityIsWithinArea(RwaEntity *entity, RwaArea *area, int offsetType)
{
    double distance;
    double radiusInKm;
    double areaOffsetInKm;
    double areaOffsetInMeters;

    if(offsetType == RWAAREAOFFSETTYPE_EXIT)
    {
        areaOffsetInKm = area->getExitOffset()/1000;
        areaOffsetInMeters = area->getExitOffset();
    }
    else
    {
        areaOffsetInKm = 0;
        areaOffsetInMeters = 0;
    }

    if(area->getAreaType() == RWAAREATYPE_CIRCLE)
    {
        distance = RwaUtilities::calculateDistance1( entity->getCoordinates(), area->getCoordinates());
        radiusInKm = area->getRadius()/1000.;
        if(distance <= radiusInKm + areaOffsetInKm)
        {
            //qDebug() << "Within Circle Area";
            return true;
        }
    }

    if(area->getAreaType() == RWAAREATYPE_RECTANGLE || area->getAreaType() == RWAAREATYPE_SQUARE)
    {
        if(RwaUtilities::coordinateWithinRectangle1(entity->getCoordinates(), area->getCoordinates(), area->getWidth() + areaOffsetInMeters, area->getHeight() + areaOffsetInMeters))
        {
           // qDebug() << "Within Rect Area";
            return true;
        }
    }

    if(area->getAreaType() == RWAAREATYPE_POLYGON)
    {
        if(offsetType == RWAAREAOFFSETTYPE_ENTER || area->getExitOffset() == 0)
        {
            //qDebug() << "CHECK ENTER POLYGON";

            if(RwaUtilities::coordinateWithinPolygon3(entity->getCoordinates(), area->corners))
            {
               // qDebug() << "Within Rect Area";
                return true;
            }
        }

        else
        {
            if(RwaUtilities::coordinateWithinPolygon3(entity->getCoordinates(), area->exitOffsetCorners))
            {
               // qDebug() << "Within Rect Area";
                return true;
            }
         }
    }

    return false;
}

void RwaRuntime::setEntityScene(RwaEntity *entity)
{
    foreach(RwaScene *scene, entity->scenes)
    {
        if(scene->getLevel() == entity->getCurrentScene()->getLevel())
        {
            if(scene != entity->getCurrentScene())
            {

                if(entityIsWithinArea(entity, scene, RWAAREAOFFSETTYPE_ENTER))
                {
                    if(backend->logSim)
                        qDebug() << "Enter New Scene: " << QString::fromStdString(scene->objectName());

                    sendEnd2backgroundAssets(entity);
                    entity->getCurrentState()->setBlockUntilRadiusHasBeenLeft(false);
                    //currentScene = scene;
                    entity->setCurrentScene(scene);
                    entity->setTimeInCurrentScene(0);
                    setEntityStartCoordinates(entity);
                    emit sendSelectedScene(scene);
                    return;
                }
            }
        }
    }
}

void RwaRuntime::startBackgroundState(RwaEntity *entity)
{
    RwaState *entityState;
    RwaAsset1 *asset;
    char gain2pd[100];
    int patcherTag;

    entityState = entity->getCurrentScene()->getBackgroundState();

    if(entityState == NULL)
        return;

   // if(backend->getLogSim())
       // qDebug() << "Enter Background State";

    if(entityState->getAssets().empty())
        return;

    foreach(asset, entityState->getAssets())
    {
        if(!asset->mute)
        {
             patcherTag = findFreePatcher(asset);

             sprintf(gain2pd, "%d-", patcherTag);
             strcat(gain2pd, "gain");
             pdMutex->lock();
             libpd_float(gain2pd, asset->gain);
             pdMutex->unlock();

             sendInitValues2pd(asset, patcherTag);
             entity->addBackgroundAsset(asset->uniqueId, asset, patcherTag);
           //  if(backend->getLogSim())
               // qDebug() << "Add Background Asset: " << QString::fromStdString(asset->fileName);
        }
    }

}

void RwaRuntime::setEntityStartCoordinates(RwaEntity *entity)
{
    if(entity->getCurrentScene() == nullptr)
        return;

    entity->getCurrentScene()->resetAssets();

    if(entity->getCoordinates()[0] == 0)
        entity->setCoordinates(entity->getCurrentScene()->getCoordinates());

     startBackgroundState(entity);
     if(!entity->getCurrentScene()->getStates().empty() && !entity->getCurrentScene()->fallbackDisabled())
     {
         qDebug() << "Set coordinates and fallback";
         entity->setCurrentState(entity->getCurrentScene()->getStates().front());
     }
}

void RwaRuntime::setEntityState(RwaEntity *entity)
{
    RwaState *state;
    RwaState *hint;
    RwaState *background = nullptr;
    RwaScene *entityScene;
    QString newScene;

    double distance;
    bool exitState = false;
    bool enterconditionsFulfilled = true;
    std::list<std::string> requiredStates;

    entity->addTimeInCurrentState((float)schedulerRate/1000.);
    entity->addTimeInCurrentScene((float)schedulerRate/1000.);

    //qDebug() << entity->getTimeInCurrentState();

    if(entity->getTimeInCurrentState() < entity->getCurrentState()->getMinimumStayTime())
        return;

    if(entity->getTimeInCurrentScene() < entity->getCurrentScene()->getMinimumStayTime())
        return;

    enterconditionsFulfilled = true;
    exitState = false;
    setEntityScene(entity);
    entityScene = entity->getCurrentScene();
    newScene = QString();

    if(entityScene)
    {
        //qDebug() << QString::fromStdString(entityScene->objectName());
        hint = nullptr;
        foreach(state, entityScene->getStates())
        {
            if( (state->getType() == RWASTATETYPE_GPS) && (entity->getCurrentState() != state) && !state->getBlockUntilRadiusHasBeenLeft())
            {
                requiredStates = state->getRequiredStates();
                if(entityIsWithinArea(entity, state, RWAAREAOFFSETTYPE_ENTER))
                {
                    if(!requiredStates.empty())
                    {
                        foreach (std::string stateName, requiredStates)
                        {
                            bool found = (std::find(entity->visitedStates.begin(), entity->visitedStates.end(), stateName) != entity->visitedStates.end());
                            if(!found)
                            {
                                enterconditionsFulfilled = false;
                                state->setBlockUntilRadiusHasBeenLeft(true);
                                if(state->getHintState().compare(""))
                                {
                                    hint = entity->getCurrentScene()->getState(state->getHintState());
                                    state->setBlockUntilRadiusHasBeenLeft(true);
                                 }

                                 break;
                            }
                        }
                    }

                    if(state->getBlockUntilRadiusHasBeenLeft())
                        enterconditionsFulfilled = false;

                    if(state->getEnterOnlyOnce())
                    {
                        bool found = (std::find(entity->visitedStates.begin(), entity->visitedStates.end(), state->objectName()) != entity->visitedStates.end());

                        if(found)
                            enterconditionsFulfilled = false;
                    }

                    if(enterconditionsFulfilled)
                    {
                        sendEnd2activeAssets(entity);
                        state->setBlockUntilRadiusHasBeenLeft(true);
                        entity->setCurrentState(state);

                        bool found = (std::find(entity->visitedStates.begin(), entity->visitedStates.end(), state->objectName()) != entity->visitedStates.end());

                        if(!found)
                        {
                            qDebug() << "Append to visited states" << QString::fromStdString(state->objectName());
                            entity->visitedStates.push_back(state->objectName());
                        }

                        unblockAssets(state);
                        entity->setTimeInCurrentState(0);
                        emit sendSelectedState(state);
                        if(backend->logSim)
                            qDebug() << "Enter new State: " << QString::fromStdString(state->objectName());
                    }
                }
            }

            if(state->getBlockUntilRadiusHasBeenLeft())
            {
               if(!entityIsWithinArea(entity, state, RWAAREAOFFSETTYPE_EXIT))
                    state->setBlockUntilRadiusHasBeenLeft(false);
            }
        }
    }

    state = entity->getCurrentState();
    background = entity->getCurrentScene()->getBackgroundState();

    if(entity->getTimeInCurrentState() > state->getTimeOut() && state->getTimeOut() > 0)
    {
        sendEnd2activeAssets(entity);
        //if(backend->getLogSim())
            //qDebug() << "Will exit State after timeout.";

        exitState = true;
    }

    if(background)
    {
        if(entity->getTimeInCurrentScene() > background->getTimeOut() && background->getTimeOut() > 0)
        {
            newScene = QString::fromStdString(background->getNextScene());
            if(newScene.compare(""))
            {
                //if(backend->getLogSim())
                    //qDebug() << "Enter new Scene after timeout.";

                sendEnd2activeAssets(entity);
                exitState = true;
            }
        }
    }

    if(state->getLeaveAfterAssetsFinish() && entity->getTimeInCurrentState() > 0)
    {
        if(entity->activeAssets.empty() )
        {
           //qDebug() << "EXIT STATE AFTER ASSETS FINISH";
            exitState = true;
        }
    }

    if(entity->getCurrentState()->getType() == RWASTATETYPE_GPS)
    {
        distance = RwaUtilities::calculateDistance1( entity->getCoordinates(), state->getCoordinates());
        if(!entityIsWithinArea(entity, state, RWAAREAOFFSETTYPE_EXIT))
        {
            if( (!state->getLeaveOnlyAfterAssetsFinish() && !entity->getCurrentScene()->fallbackDisabled())
                 || state->stateWithinState)
            {
                //qDebug() << "EXIT STATE AFTER LEAVING STATE AREA";
                sendEnd2activeAssets(entity);
                exitState = true;
            }
            else
            {
                if(entity->activeAssets.empty())
                    exitState = true;
            }
        }
    }
    if(hint)
    {
        exitState = true;
        sendEnd2activeAssets(entity);
    }

    if(exitState)
    {
        qDebug() << "EXIT STATE";
        if(hint)
        {
             //qDebug() << "auto hint state";
             entity->getCurrentState()->setBlockUntilRadiusHasBeenLeft(true);
             RwaState *nextState = hint;
             if(hint != entity->getCurrentState())
             {
                 unblockAssets(nextState);
                 entity->setCurrentState(nextState);
                 entity->setTimeInCurrentState(0);
                 emit sendSelectedState(nextState);
                 //qDebug() << "RwaSimulator::setEntityState" << nextState->objectName().toLatin1();
             }
        }

        else if(state->getNextScene().compare(""))
        {
            //qDebug() << "auto next scene";
            RwaScene *nextScene = entity->getScene(state->getNextScene());

            sendEnd2backgroundAssets(entity);
            //currentScene = nextScene;
            entity->setCurrentScene(nextScene);
            entity->setTimeInCurrentScene(0);
            setEntityStartCoordinates(entity);
            emit sendSelectedScene(nextScene);
            emit sendSelectedState(entity->getCurrentState());
        }

        else if(newScene.compare(""))
        {
            //qDebug() << "auto next scene: " << newScene;
            RwaScene *nextScene = entity->getScene(newScene.toStdString());

            sendEnd2backgroundAssets(entity);
            //currentScene = nextScene;
            entity->setCurrentScene(nextScene);
            entity->setTimeInCurrentScene(0);
            setEntityStartCoordinates(entity);
            emit sendSelectedScene(nextScene);
        }

        else if(state->getNextState().compare("") )
        {
            //qDebug() << "auto next state";
            RwaState *nextState = entity->getCurrentScene()->getState(state->getNextState());
            unblockAssets(nextState);
            entity->setCurrentState(nextState);
            entity->setTimeInCurrentState(0);
            emit sendSelectedState(nextState);
            //qDebug() << "RwaSimulator::setEntityState" << nextState->objectName().toLatin1();
        }
        else
        {
            if(!entity->getCurrentScene()->fallbackDisabled())
            {
                RwaState *nextState = entity->getCurrentScene()->getStates().front();
                entity->setCurrentState(nextState); // set to fallback state
                entity->setTimeInCurrentState(0);
                emit sendSelectedState(nextState);
               // if(backend->getLogSim())
                    qDebug() << "Enter Fallback State";
            }
        }
    }
}

void RwaRuntime::endBackgroundState()
{
    int intPatcherTag;
    char end2pd[100];
    RwaEntity *entity;
    RwaEntity::AssetMapItem item;
    string key;

    foreach(entity, entities)
    {
        std::map<string, RwaEntity::AssetMapItem>::iterator i = entity->activeAssets.begin();
        while(i != entity->activeAssets.end())
        {
            key = i->first;
            item = i->second;
            intPatcherTag = item.getPatcherTag();
            i = entity->backgroundAssets.erase(i);
            releasePatcherFromItem(item);
            sprintf(end2pd,"%d-", intPatcherTag);
            strcat(end2pd, "free");
            pdMutex->lock();
            libpd_bang(end2pd);
            pdMutex->unlock();

            if(backend->logSim)
                qDebug() << "Free Background Asset: " << intPatcherTag;
        }
    }
}

void RwaRuntime::freeAllPatchers()
{
    int intPatcherTag;
    char end2pd[100];
    RwaEntity *entity = nullptr;
    RwaEntity::AssetMapItem item;
    string key;

    if(backend->logSim)
        qDebug() << "Free all Patchers";

    foreach(entity, entities)
    {
        std::map<string, RwaEntity::AssetMapItem>::iterator i = entity->activeAssets.begin();
        while(i != entity->activeAssets.end())
        {
            key = i->first;
            item = i->second;

            intPatcherTag = item.getPatcherTag();
            i = entity->activeAssets.erase(i);
            releasePatcherFromItem(item);
            sprintf(end2pd,"%d-", intPatcherTag);
            strcat(end2pd, "free");
            pdMutex->lock();
            libpd_bang(end2pd);
            pdMutex->unlock();
        }
    }

    endBackgroundState();
    if(entity)
        entity->reset();
}

void RwaRuntime::update(RwaEntity *entity)
{
    //if(!devicesRegistered)
    {
#ifdef INIT_LIBPD_QUEUED
        pdMutex->lock();
        libpd_queued_receive_pd_messages();
        pdMutex->unlock();
#endif
        sendData2activeAssets(entity);
        setEntityState(entity);
        processAssets(entity);

    }
    //sendData2Devices();
}
