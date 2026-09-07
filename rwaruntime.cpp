#include "rwaruntime.h"
#include <algorithm>
#include <cmath>

std::list <RwaEntity *> RwaRuntime::entities;
bool RwaRuntime::debug;

pdPatcher RwaRuntime::binauralStereoPatchers_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::binauralStereoPatchersOgg_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::binauralMonoPatchers_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::binauralMonoPatchersOgg_fabian[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::binaural5channelPatchers_fabian[RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS];
pdPatcher RwaRuntime::binaural7channelPatchers_fabian[RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS];
pdPatcher RwaRuntime::stereoPatchers[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::stereoPatchersOgg[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::monoPatchers[RWARUNTIME_MAXNUMBEROFPATCHERS];
pdPatcher RwaRuntime::monoPatchersOgg[RWARUNTIME_MAXNUMBEROFPATCHERS];

std::list<pdPatcher *> RwaRuntime::dynamicPatchers1;
std::list<RwaEntity::AssetMapItem> RwaRuntime::assetsPendingRelease;

RwaBackend *RwaRuntime::backend;

bool RwaRuntime::logSim = false;
bool RwaRuntime::logPd = false;
float RwaRuntime::pdSampleRate = 48000;

#ifdef QT_VERSION
std::function<uint32_t()> RwaRuntime::seedSource = []{ return QRandomGenerator::global()->generate(); };
#else
std::function<uint32_t()> RwaRuntime::seedSource = []{ static std::random_device rd; return rd(); };
#endif

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

    // Init first: since libpd 0.14 the hook storage (queued ring buffers, the
    // print concatenator's buffer) lives in per-instance state that init
    // allocates, so setting a hook before it writes through a null pointer.
    //
    // Pd emits a print in pieces ("print", ": ", "1", " ", "2", "\n"); route them
    // through libpd's concatenator so printpd sees one complete line instead of
    // one log entry per symbol plus a trailing empty one.
#ifdef INIT_LIBPD_QUEUED
    libpd_queued_init();
    libpd_set_concatenated_printhook (static_cast<t_libpd_printhook>(RwaRuntime::printpd));
    libpd_set_queued_printhook (libpd_print_concatenator);
    libpd_set_queued_floathook (static_cast<t_libpd_floathook>(RwaRuntime::floatpd));
    libpd_set_queued_banghook (static_cast<t_libpd_banghook>(RwaRuntime::bangpd));
#else
    libpd_init();
    libpd_set_concatenated_printhook (static_cast<t_libpd_printhook>(RwaRuntime::printpd));
    libpd_set_printhook (libpd_print_concatenator);
    libpd_set_floathook (static_cast<t_libpd_floathook>(RwaRuntime::floatpd));
    libpd_set_banghook (static_cast<t_libpd_banghook>(RwaRuntime::bangpd));
#endif
    // Put the bundled Pd resources on Pd's search path. The FABIAN HRTF set
    // (fabian_dir256.txt, 38 MB) lives there next to the playback patches, so the
    // bundled patches find it by sitting in the same directory - a creator's own
    // patcher, opened from the game's assets folder, does not. With the resources
    // on the search path, [rwa_binauralsimple~ 256 fabian_dir256.txt] resolves from
    // any patch and the file no longer has to be copied into every project.
    libpd_add_to_search_path(pdpath);

    rwa_binauralsimple_tilde_setup();
    vas_reverb_tilde_setup();
    freeverb_tilde_setup();
    oggread_tilde_setup();

    libpd_openfile("stereoout.pd", pdpath);

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaloopplayermono.pd", pdpath);
        monoPatchers[i].patcherTag = d;
        monoPatchers[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&monoPatchers[i]);
    }

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaloopplayermonoogg.pd", pdpath);
        monoPatchersOgg[i].patcherTag = d;
        monoPatchersOgg[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&monoPatchersOgg[i]);
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
        void *d = libpd_openfile("rwaloopplayerstereoogg.pd", pdpath);
        stereoPatchersOgg[i].patcherTag = d;
        stereoPatchersOgg[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&stereoPatchersOgg[i]);
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

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaplayermonobinauralogg_fabian.pd", pdpath);
        binauralMonoPatchersOgg_fabian[i].patcherTag = d;
        binauralMonoPatchersOgg_fabian[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&binauralMonoPatchersOgg_fabian[i]);
    }

    for(uint32_t i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        void *d = libpd_openfile("rwaplayerstereobinauralogg_fabian.pd", pdpath);
        binauralStereoPatchersOgg_fabian[i].patcherTag = d;
        binauralStereoPatchersOgg_fabian[i].isBusy = false;
        createAndBindPlayFinishedReceiver(&binauralStereoPatchersOgg_fabian[i]);
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
}

extern "C" void *RwaRuntime_new(QObject *parent, const char *pdpath, const char *assetPath, float sampleRate, float schedulerRate, mutex *pdMutex, RwaBackend *_backend) // wrapper function
{
    RwaRuntime *runtime = new RwaRuntime(parent, pdpath, assetPath, sampleRate, schedulerRate, pdMutex, _backend);
    return runtime;
}

RwaRuntime::~RwaRuntime()
{

}

void RwaRuntime::createAndBindPlayFinishedReceiver(pdPatcher *patcher)
{
    std::stringstream receiverStream;
    receiverStream << libpd_getdollarzero(patcher->patcherTag) << "-playfinished";
    patcher->playFinishedReceiver = libpd_bind(receiverStream.str().c_str());
}

void RwaRuntime::printpd(const char *s)
{
    if(!logPd)
        return;

    QString line = QString::fromUtf8(s).trimmed();
    if(line.isEmpty())
        return;

    // the log view drops messages longer than 512 chars, so shorten here
    // instead of losing the whole line
    if(line.length() > 500)
        line = line.left(500) + QStringLiteral("...");

    // noquote(): keep the line as pd wrote it, no surrounding quotes and no
    // backslash escaping of the contained symbols
    qInfo().noquote() << "[pd]" << line;
}

void RwaRuntime::floatpd(const char *source, float value)
{
     if(logPd)
         qInfo().noquote() << "[pd]" << source << value;
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

    else if(asset->type == RWAASSETTYPE_OGG)
    {
        switch(playbackType)
        {
            case RWAPLAYBACKTYPE_MONO:
                monoPatchersOgg[getMonoPatcherOggIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_STEREO:
                stereoPatchersOgg[getStereoPatcherOggIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_BINAURALMONO_FABIAN:
                binauralMonoPatchersOgg_fabian[getBinauralMonoFabianOggPatcherIndex(patcherTag)].isBusy = false;
                break;

            case RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN:
                binauralStereoPatchersOgg_fabian[getBinauralStereoFabianOggPatcherIndex(patcherTag)].isBusy = false;
                break;

            default: break;
        }
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
                monoPatchers[getMonoPatcherIndex(patcherTag)].isBusy = false;
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
            if(logSim)
                qInfo() <<  "Released Asset" << ": " << QString::fromStdString(assetItem->fileName);

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
        if(logSim)
            qDebug() << "playfinished";
        RwaEntity *entity;
        RwaEntity::AssetMapItem item;

        foreach(entity, entities)
        {
            bangpdHelp(intPatcherTag, entity->activeAssets);
            bangpdHelp(intPatcherTag, entity->backgroundAssets);
        }

        // superseded background instance finished its fade-out
        std::list<RwaEntity::AssetMapItem>::iterator p = assetsPendingRelease.begin();
        while(p != assetsPendingRelease.end())
        {
            if(p->getPatcherTag() == intPatcherTag)
            {
                releasePatcherFromItem(*p);
                p = assetsPendingRelease.erase(p);
                if(logSim)
                    qInfo() << "Released superseded background patcher: " << intPatcherTag;
            }
            else
                ++p;
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

int32_t RwaRuntime::getBinauralMonoFabianOggPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(binauralMonoPatchersOgg_fabian[i].patcherTag) == patcherTag)
            return i;
    }
    return RWARUNTIME_INVALIDPATCHERINDEX;
}

int32_t RwaRuntime::getBinauralStereoFabianOggPatcherIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(binauralStereoPatchersOgg_fabian[i].patcherTag) == patcherTag)
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

int32_t RwaRuntime::getStereoPatcherOggIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(stereoPatchersOgg[i].patcherTag) == patcherTag)
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

int32_t RwaRuntime::getMonoPatcherOggIndex(int32_t patcherTag)
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(libpd_getdollarzero(monoPatchersOgg[i].patcherTag) == patcherTag)
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

             if(logSim)
                 qInfo() <<  "Found Dynamic PD Asset" << ": " << QString::fromStdString(patcher->name);

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

    if(logSim)
        qDebug() << "Freed all dynamic Patchers";
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
                        if(logSim)
                            qInfo() <<  "Initialize Pd Asset" << ": " << QString::fromStdString(newPatcher->name);
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

            qInfo() <<  "End background asset: "  << QString::fromStdString(assetItem->fileName);

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

            if(logSim)
                qInfo() <<  "End active asset: "  << QString::fromStdString(assetItem->fileName);

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
                asset->blockedForever = false;
                asset->blocked = false;
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

void *RwaRuntime::findFreeBinauralMonoFabianOggPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!binauralMonoPatchersOgg_fabian[i].isBusy)
        {
            binauralMonoPatchersOgg_fabian[i].isBusy = true;
            return binauralMonoPatchersOgg_fabian[i].patcherTag;
        }
    }
    return nullptr;
}

void *RwaRuntime::findFreeBinauralStereoFabianOggPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!binauralStereoPatchersOgg_fabian[i].isBusy)
        {
            binauralStereoPatchersOgg_fabian[i].isBusy = true;
            return binauralStereoPatchersOgg_fabian[i].patcherTag;
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

void *RwaRuntime::findFreeStereoOggPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!stereoPatchersOgg[i].isBusy)
        {
            stereoPatchersOgg[i].isBusy = true;
            return stereoPatchersOgg[i].patcherTag;
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

void *RwaRuntime::findFreeMonoOggPatcher()
{
    for(int i=0; i<RWARUNTIME_MAXNUMBEROFPATCHERS; i++)
    {
        if(!monoPatchersOgg[i].isBusy)
        {
            monoPatchersOgg[i].isBusy = true;
            return monoPatchersOgg[i].patcherTag;
        }
    }
    return nullptr;
}

int32_t RwaRuntime::findFreePatcher(RwaAsset1 *asset)
{
    if(asset->type == RWAASSETTYPE_PD)
        return libpd_getdollarzero(findFreeDynamicPatcher(asset));

    else if (asset->type == RWAASSETTYPE_OGG)
    {
        switch(asset->playbackType)
        {
            case RWAPLAYBACKTYPE_MONO: return libpd_getdollarzero(findFreeMonoOggPatcher());

            case RWAPLAYBACKTYPE_STEREO: return libpd_getdollarzero(findFreeStereoOggPatcher());

            case RWAPLAYBACKTYPE_BINAURALMONO_FABIAN: return libpd_getdollarzero(findFreeBinauralMonoFabianOggPatcher());

            case RWAPLAYBACKTYPE_BINAURALSTEREO_FABIAN: return libpd_getdollarzero(findFreeBinauralStereoFabianOggPatcher());

            default: return RWARUNTIME_INVALIDPATCHERINDEX;
        }
    }

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

// The gain that actually reaches Pd: asset gain scaled by the gain of the state
// and the scene the asset belongs to.
// Owners resolved through asset (not through the entity's current state):
// background assets belong to the scene's background state, which is not the
// entity's current state. RwaAsset1::myScene is not reliably set, so the scene
// is reached via the state.
// Mirrored in RwaGameLoop.effectiveGain (rwa-player).
float RwaRuntime::effectiveGain(RwaAsset1 *asset)
{
    float gain = asset->gain;
    RwaState *state = asset->myState;
    if(state)
    {
        gain *= state->gain;
        if(state->myScene)
            gain *= state->myScene->gain;
    }
    return gain;
}

/**
 * Send to Pd: Where the asset's channels are, as heard from the entity:
 * one "-distanceN"/"-azimuthN"/ "-elevationN" triple per channel.
 *
 * called by sendInitValues2pd and sendData2Asset
 */
void RwaRuntime::sendSpatialData2pd(RwaEntity *entity, RwaAsset1 *asset, int patcherTag)
{
    if(asset->type == RWAASSETTYPE_PD && !asset->headtrackerRelative2Source)
    {
        // The patch spatialises on its own from the raw head orientation:
        // one set of data, azimuth/elevation are the head values, not source-relative.
        calculateChannelBearingAndDistance(entity, asset, 0);
        double totalDistance = RwaUtilities::calculateDistanceWithAltitude(asset->channelDistance[0], asset->getElevation());
        sendDistance(0, patcherTag, totalDistance);
        sendBearing(0, patcherTag, entity->azimuth());
        sendElevation(0, patcherTag, entity->elevation());
    }
    else
    {
        int numChannels = asset->playbackChannelCount();

        for(int i = 0; i < numChannels; i++)
        {
            calculateChannelBearingAndDistance(entity, asset, i);
            const double horizontalDistance = std::max(static_cast<double>(asset->channelDistance[i]), 0.0);
            const double altitude = asset->getElevation();
            // atan2 is finite at horizontalDistance == 0 (0 or +-90), unlike atan(altitude / d).
            const double worldElevation = RwaUtilities::radians2degrees(atan2(altitude, horizontalDistance));
            const double totalDistance = RwaUtilities::calculateDistanceWithAltitude(horizontalDistance, altitude);

            double azimuth, elevation;
            if(asset->getFixedAzimuth() < 0)
            {
                RwaUtilities::RelativeDirection rel = RwaUtilities::calculateRelativeDirection(asset->channelBearing[i], worldElevation, entity->azimuth(), entity->elevation());
                azimuth = rel.azimuth;
                elevation = rel.elevation;
            }
            else
            {
                // fixed orientation: the source keeps its place relative to the head,
                // so neither yaw nor pitch is applied.
                azimuth = asset->channelBearing[i];
                elevation = worldElevation;
            }

            sendDistance(i, patcherTag, totalDistance);
            sendBearing(i, patcherTag, azimuth);
            sendElevation(i, patcherTag, elevation);
        }
    }
}

void RwaRuntime::sendInitValues2pd(RwaEntity *entity, RwaAsset1 *asset, int patcherTag)
{
    if(logSim)
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
        firstCrossfadeAfter-= (asset->playheadPosition/(pdSampleRate/1000.));

        if(firstCrossfadeAfter <0)
        {
            firstCrossfadeAfter = asset->getFadeOutAfter();
            asset->playheadPosition = 0;
        }
    }

    // ctor sets both startPosition and gpsLocation to "gps" vector argument
    // creator allows to set new start position, at which point they differ
    if(asset->getMoveFromStartPosition())
    {
        // RwaAsset1::currentPostion <- RwaAsset1::startPosition
        asset->setCurrentPosition(asset->getStartPosition());
        asset->setReachedEndPosition(false);
    }
    else
    {
        // RwaAsset1::currentPostion <- RwaLocation1::gpsLocation
        asset->setCurrentPosition(asset->getCoordinates());
        asset->setReachedEndPosition(true);
    }

     asset->distanceForMovement = RwaUtilities::calculateDistance1(asset->startPosition, asset->getCoordinates()) * 1000;
     asset->movingDistancePerTick = asset->getMovementSpeed() * ((float)schedulerRate/1000.);
     asset->rotateOffsetPerTick = asset->rotateFrequency * 360 * schedulerRate/1000;
     asset->setCurrentRotateAngleOffset(0);

     sprintf(pdReceiver, "%d-assetlon", patcherTag);
     pdMutex->lock();
     // shouldn't this be asset->getCurrentPosition()? so that moving assets start  in the right place?
     libpd_float(pdReceiver, asset->getCoordinates()[0]);
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-assetlat", patcherTag);
     pdMutex->lock();
     // dito, see -assetlon above
     libpd_float(pdReceiver, asset->getCoordinates()[1]);
     pdMutex->unlock();

     sprintf (pdReceiver, "%d-samplerate", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, pdSampleRate);
     pdMutex->unlock();

     // how many azimuthN/distanceN/elevationN channels this asset will be
     // streamed, so patches can adapt (derived from the playback mode; the
     // channelcount XML attribute is unreliable and not used)
     sprintf(pdReceiver, "%d-numchannels", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->playbackChannelCount());
     pdMutex->unlock();

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

     sprintf(pdReceiver, "%d-smoothdist", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getSmoothDist());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-offset", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getOffset());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-loop", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getLoop());
     pdMutex->unlock();

     // new, sync to RWA Player
     sprintf(pdReceiver, "%d-gain", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, effectiveGain(asset));
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-fadeintime", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getFadeInTime());
     pdMutex->unlock();

     sprintf(pdReceiver, "%d-fadeouttime", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, asset->getFadeOutTime());
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

     // fresh seed per activation for [random] etc. in Pd asset patches
     // Kept within 24 bits and non-zero so it
     // survives the float32 conversion exactly.
     sprintf(pdReceiver, "%d-seed", patcherTag);
     pdMutex->lock();
     libpd_float(pdReceiver, (float)(1 + (seedSource() & 0xFFFFFE)));
     pdMutex->unlock();

     // Last before "-play":
     // "-dampingfunction" and "-smoothdist" have already setup the damping chain,
     // the patch should override any spatial values from preceding patch activations
     // and initialise objects / ramp-starts with the new values
     sendSpatialData2pd(entity, asset, patcherTag);

     fullAssetPath << assetPath << asset->fileName;
     sprintf(pdReceiver,"%d-play", patcherTag);
     pdMutex->lock();
     libpd_symbol(pdReceiver, fullAssetPath.str().c_str());
     pdMutex->unlock();

     if(logSim)
         qDebug() << "Initialised asset: " << asset->fileName << " sent to patcher id " << patcherTag << ", gain: " << asset->gain;
}

void RwaRuntime::processAssets(RwaEntity *entity)
{
    RwaAsset1 *asset;
    int patcherTag;

    if(entity->getCurrentState() == nullptr)
        return;

    foreach(asset, entity->getCurrentState()->getAssets())
    {
        if(!entity->isActiveAsset(asset->uniqueId) && !asset->getBlocked() && !asset->mute && !asset->getBlockedForever())
        {
            patcherTag = findFreePatcher(asset);
            sendInitValues2pd(entity, asset, patcherTag);

            if(asset->playOnlyOnce)
                asset->setBlockedForever(true);

            entity->addActiveAsset(asset->uniqueId, asset, patcherTag);

            if(logSim) {
                qInfo() << "Add Active Asset: " << QString::fromStdString(asset->fileName);
                qDebug() << asset->type;
            }

            break;
        }
    }
}

void RwaRuntime::calculateChannelBearingAndDistance(RwaEntity *entity, RwaAsset1 *asset, int channel)
{
    int offset = RwaAsset1::channelOffsetForPlaybackType(asset->getPlaybackType(), channel);
    offset += (360-asset->getRotateOffset()) % 360;

    if(!asset->hasCustomChannelPosition[channel])
    {
        float channelRadius = asset->getChannelRadius();
        if(asset->playbackType == RWAPLAYBACKTYPE_MONO || asset->playbackType == RWAPLAYBACKTYPE_STEREO)
            channelRadius = 0;

        std::vector<double> destination = RwaUtilities::calculateDestination1(asset->getCurrentPosition(), channelRadius, RwaUtilities::wrap360(offset + asset->currentRotateAngleOffset));
        asset->channelcoordinates[channel] = destination;
    }

    if(asset->getFixedDistance() < 0) // -1 = default value = use actual distance
    {
        double distance = RwaUtilities::calculateDistanceInMeters(entity->getCoordinates(), asset->channelcoordinates[channel]);
        if(asset->minDistance >= 0 && distance < asset->minDistance)
            distance = asset->minDistance; // keeps the listener from walking "through" the source
        asset->channelDistance[channel] = distance;
    }
    else
        asset->channelDistance[channel] = asset->getFixedDistance();

    // channelBearing holds the world bearing of the channel as seen from the listener; the head
    // rotation is applied in sendData2Asset. A fixed azimuth is already listener-relative and is
    // stored as the final value (no head rotation is applied to it, see sendData2Asset).
    if(asset->getFixedAzimuth() < 0) // -1 = default value = use actual azimuth
        asset->channelBearing[channel] = RwaUtilities::calculateWorldBearing(entity->getCoordinates(), asset->channelcoordinates[channel]);
    else
        asset->channelBearing[channel] = RwaUtilities::wrap360(asset->getFixedAzimuth() + offset);
}

void RwaRuntime::sendDistance(int channel, int patcherTag, float distance)
{
    if(!std::isfinite(distance))
        return;
    char distance2pd[20];
    sprintf(distance2pd,"%d-distance%d", patcherTag, channel+1);
    pdMutex->lock();
    libpd_float(distance2pd, distance);
    pdMutex->unlock();
}

void RwaRuntime::sendBearing(int channel, int patcherTag, float bearing)
{
    if(!std::isfinite(bearing))
        return;
    char azimuth2pd[20];
    sprintf(azimuth2pd,"%d-azimuth%d", patcherTag, channel+1);
    pdMutex->lock();
    libpd_float(azimuth2pd, bearing);
    pdMutex->unlock();
}

void RwaRuntime::sendElevation(int channel, int patcherTag, float elevation)
{
    if(!std::isfinite(elevation))
        return;
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
    libpd_float(gain2pd, effectiveGain(asset));
    libpd_float(lon2pd, entity->getCoordinates()[0]);
    libpd_float(lat2pd, entity->getCoordinates()[1]);
    pdMutex->unlock();

    if(step)
    {
        pdMutex->lock();
        libpd_bang(step2pd);
        pdMutex->unlock();
    }

    sendSpatialData2pd(entity, asset, intPatcherTag);

    if(!asset->getReachedEndPosition())
    {
        if(asset->distanceForMovement > 0)
        {
            asset->distanceForMovement -= asset->getMovingDistancePerTick();
            std::vector<double> tmp = RwaUtilities::calculateDestination1(asset->getCoordinates(), asset->distanceForMovement, asset->bearingForMovement);
            asset->setCurrentPosition(tmp);
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
            asset->playheadPosition += ((float)schedulerRate/1000. * pdSampleRate);

        if(asset->playheadPosition >= asset->fadeOutAfter/1000. * pdSampleRate)
        {
            asset->playheadPosition = 0;
            if(!asset->getLoop())
                asset->updatePlayheadPosition = false;

        }
        if(asset->playheadPosition - lastP > pdSampleRate)
            lastP = asset->playheadPosition;
    }
}

void RwaRuntime::sendData2activeAssets(RwaEntity *entity)
{
    RwaState *entityState;
    RwaEntity::AssetMapItem item;
    string key;

    if(entities.empty())
        return;

    foreach(entity, entities)
    {
        // having an active state,
        entityState = entity->getCurrentState();
        if(entityState != NULL)
        {
            // update active assets
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

            // update background assets
            if(!entity->backgroundAssets.empty())
            {
                std::map<string, RwaEntity::AssetMapItem>::iterator i;
                for (i = entity->backgroundAssets.begin(); i != entity->backgroundAssets.end(); ++i)
                {
                    key = i->first;
                    item = i->second;
                    sendData2Asset(entity, item);
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
        radiusInKm = static_cast<double>(area->getRadius())/1000.;
        if(distance <= radiusInKm + areaOffsetInKm)
        {
           // qDebug() << "Within Circle Area";
           // qDebug() << distance << " " << radiusInKm << " " << areaOffsetInKm;
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

/**
 * Transition entity into a new scene
 *
 * - end old bg assets
 * - unblock old state
 * - switch scene
 * - if fallback activates: end old active assets, enter fallback state
 * - else assets keep playing until new state is triggered
 * - start new bg state.
 */
void RwaRuntime::setScene(RwaEntity *entity, RwaScene *scene)
{
    if(logSim)
        qInfo() << "Enter new scene: " << QString::fromStdString(scene->objectName());

    // let bg assets of current scene fade out
    sendEnd2backgroundAssets(entity);

    // release the current state's re-entry latch
    if(entity->getCurrentState()) {
        entity->getCurrentState()->setBlockUntilRadiusHasBeenLeft(false);
        if(logSim)
            qDebug() << "Unblocking state " << entity->getCurrentState()->objectName();
    }

    // switch to the new/first scene
    entity->setCurrentScene(scene);
    entity->setTimeInCurrentScene(0);

    // activate fallback
    // if fallback of new scene can be activated, notify active assets of previous state/scene to end
    // (otherwhise, active assets stay active until a new state is triggered)
    if(!scene->fallbackDisabled())
    {
        if(scene->getStates().empty())
            // this should be caught in the XML parseer (InsertDefaultFallbackState)
            // enforcing presence of BG and FB states
            qWarning() << "Scene " << QString::fromStdString(scene->objectName()) << " has fallback enabled but no states.";
        else
        {
            RwaState *fallback = scene->getStates().front();
            if(!fallback->getAssets().empty())
                sendEnd2activeAssets(entity);
            entity->setCurrentState(fallback);
            entity->setTimeInCurrentState(0);
        }
    }

    startBackgroundState(entity);
    emit sendSelectedScene(scene);
}

void RwaRuntime::setEntityScene(RwaEntity *entity)
{
    // While the hero is still inside the current scene's area, stay. Without this,
    // overlapping scene areas switch back and forth on every tick, each switch
    // ending and restarting the background states.
    // Mirrored in RwaGameLoop.swift setEntityScene().
    if(entityIsWithinArea(entity, entity->getCurrentScene(), RWAAREAOFFSETTYPE_EXIT))
        return;

    foreach(RwaScene *scene, entity->scenes)
    {
        if(scene->getLevel() == entity->getCurrentScene()->getLevel())
        {
            if(scene != entity->getCurrentScene())
            {
                if(entityIsWithinArea(entity, scene, RWAAREAOFFSETTYPE_ENTER))
                {
                    setScene(entity, scene);
                    return; // parity
                }
            }
        }
    }
}

void RwaRuntime::startBackgroundState(RwaEntity *entity)
{
    RwaState *entityState;
    RwaAsset1 *asset;
    int patcherTag;

    entityState = entity->getCurrentScene()->getBackgroundState();

    if(logSim)
        qInfo() << "Starting Background State of " << QString::fromStdString((entity->getCurrentScene())->objectName());

    if(entityState == NULL)
        return;

    if(entityState->getAssets().empty())
        return;

    // contrary to RwaRuntime::processAssets, this loads all assets in one go (no scheduler-tick between loads)
    foreach(asset, entityState->getAssets())
    {
        if(!asset->mute)
        {
             // An instance from an earlier visit may still be fading out (scene re-entered
             // within the fade-out window). Its map slot must go to the new instance,
             // otherwise the new patch starts but is never tracked: it can't be ended or
             // released and loops forever. The fading patcher is parked for release on
             // its "-playfinished".
             std::map<string, RwaEntity::AssetMapItem>::iterator existing = entity->backgroundAssets.find(asset->uniqueId);
             if(existing != entity->backgroundAssets.end())
             {
                 assetsPendingRelease.push_back(existing->second);
                 entity->backgroundAssets.erase(existing);
             }

             patcherTag = findFreePatcher(asset);
             sendInitValues2pd(entity, asset, patcherTag); // sends "-gain" ans spatial params itself

             entity->addBackgroundAsset(asset->uniqueId, asset, patcherTag);
             if(logSim)
                 qDebug() << "Add Background Asset: " << QString::fromStdString(asset->fileName);
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
}

void RwaRuntime::setEntityState(RwaEntity *entity)
{
    RwaState *state;
    RwaState *hint;
    RwaState *background = nullptr;
    RwaScene *entityScene;
    QString newScene;
    bool exitState = false;
    bool enterconditionsFulfilled = true;
    std::list<std::string> requiredStates;

    entity->addTimeInCurrentState(schedulerRate/1000.0f);
    entity->addTimeInCurrentScene(schedulerRate/1000.0f);

    /** **************************************** Without a scene, do nothing *********************************************** */

    if(!(entityScene = entity->getCurrentScene()))
        return;

    /** ******************************** If minimum times are not reached, do nothing ************************************** */

    if(entity->getCurrentState())
    {
        if(entity->getTimeInCurrentState() < entity->getCurrentState()->getMinimumStayTime())
            return;

        if(entity->getTimeInCurrentScene() < entity->getCurrentScene()->getMinimumStayTime())
            return;

        if(entity->getCurrentState()->getLeaveOnlyAfterAssetsFinish() && !entity->activeAssets.empty())
            return;
    }

    enterconditionsFulfilled = true;
    exitState = false;
    newScene = QString();
    hint = nullptr;

    /** *************************************** Check conditions for a new scene ******************************************* */

    setEntityScene(entity);
    // setEntityScene may have switched the scene; the checks below must run against the
    // new one, not the scene cached above (the Swift engine re-reads it here as well).
    entityScene = entity->getCurrentScene();

    /** *************************************** Check conditions for a new state ******************************************* */

    foreach(state, entityScene->getStates())
    {
        enterconditionsFulfilled = true;
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

                            break; // We can break here, no need to search for other required states
                        }
                    }
                }

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
                        if(logSim)
                            qInfo() << "Append to visited states" << QString::fromStdString(state->objectName());
                        entity->visitedStates.push_back(state->objectName());
                    }

                    unblockAssets(state);
                    entity->setTimeInCurrentState(0);
                    emit sendSelectedState(state);

                    if(logSim)
                        qInfo() << "Enter new State: " << QString::fromStdString(state->objectName());

                    break; // we can break here for now
                }
            }
        }
    }

    foreach(state, entityScene->getStates())
    {
        if(state->getBlockUntilRadiusHasBeenLeft())
        {
           if(!entityIsWithinArea(entity, state, RWAAREAOFFSETTYPE_EXIT))
                state->setBlockUntilRadiusHasBeenLeft(false);
        }
    }

    background = entityScene->getBackgroundState();

    if(background)
    {
        if(entity->getTimeInCurrentScene() > background->getTimeOut() && background->getTimeOut() > 0)
        {
            newScene = QString::fromStdString(background->getNextScene());
            if(newScene.compare(""))
            {
                if(logSim)
                    qInfo() << "Enter new Scene after timeout.";

                sendEnd2activeAssets(entity);
                exitState = true;
            }
        }
    }

    if(entity->getTimeInCurrentScene() > entityScene->getTimeOut() && entityScene->getTimeOut() > 0)
    {
        newScene = QString::fromStdString(entityScene->getNextScene());
        if(newScene.compare(""))
        {
            sendEnd2activeAssets(entity);
            exitState = true;
        }
    }

    state = entity->getCurrentState();

    if(state)
    {
        if(entity->getTimeInCurrentState() > state->getTimeOut() && state->getTimeOut() > 0)
        {
            sendEnd2activeAssets(entity);
            if(logSim)
                qInfo() << "Will exit State after timeout.";

            exitState = true;
        }

        if(state->getLeaveAfterAssetsFinish() && entity->getTimeInCurrentState() > 0)
        {
            if(entity->activeAssets.empty() )
            {
                if(logSim)
                    qDebug() << "exit state after assets finish";
                exitState = true;
            }
        }

        if(entity->getCurrentState()->getType() == RWASTATETYPE_GPS)
        {
            if(!entityIsWithinArea(entity, state, RWAAREAOFFSETTYPE_EXIT))
            {
                if( (!state->getLeaveOnlyAfterAssetsFinish() && !entity->getCurrentScene()->fallbackDisabled())
                     || state->stateWithinState)
                {
                    if(logSim)
                        qDebug() << "exit state after leaving state area";
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
    }

    if(exitState)
    {
        if(logSim)
            qInfo() << "Exit state";
        if(hint)
        {
             qDebug() << "auto hint state";
             entity->getCurrentState()->setBlockUntilRadiusHasBeenLeft(true);
             RwaState *nextState = hint;
             if(hint != entity->getCurrentState())
             {
                 unblockAssets(nextState);
                 entity->setCurrentState(nextState);
                 entity->setTimeInCurrentState(0);
                 emit sendSelectedState(nextState);
                 qDebug() << QString::fromStdString(nextState->objectName());
             }
        }

        else if(newScene.compare(""))
        {
            RwaScene *nextScene = entity->getScene(newScene.toStdString());
            setScene(entity, nextScene);
        }

        else if(state->getNextScene().compare(""))
        {
            RwaScene *nextScene = entity->getScene(state->getNextScene());
            setScene(entity, nextScene);
        }

        else if(state->getNextState().compare("") )
        {
            qDebug() << "auto next state";
            RwaState *nextState = entity->getCurrentScene()->getState(state->getNextState());
            if(!nextState)
            {
                qWarning() << "Next state" << QString::fromStdString(state->getNextState()) << "not found in current scene.";
                return;
            }
            unblockAssets(nextState);
            entity->setCurrentState(nextState);
            entity->setTimeInCurrentState(0);
            emit sendSelectedState(nextState);
            qDebug() << "RwaSimulator::setEntityState" << QString::fromStdString(nextState->objectName());
        }
        else
        {
            if(!entity->getCurrentScene()->fallbackDisabled())
            {
                RwaState *nextState = entity->getCurrentScene()->getStates().front();
                entity->setCurrentState(nextState); // set to fallback state
                entity->setTimeInCurrentState(0);
                emit sendSelectedState(nextState);
                if(logSim)
                    qInfo() << "Enter Fallback State";
            }
        }
    }
}

void RwaRuntime::resetPatcher(int intPatcherTag)
{
    char pdReceiver[100];
    sprintf(pdReceiver,"%d-", intPatcherTag);
    strcat(pdReceiver, "free");
    pdMutex->lock();
    libpd_bang(pdReceiver);
    pdMutex->unlock();

    sprintf(pdReceiver, "%d-fadeouttime", intPatcherTag);
    pdMutex->lock();
    libpd_float(pdReceiver, 0);
    pdMutex->unlock();

    sprintf(pdReceiver,"%d-", intPatcherTag);
    strcat(pdReceiver, "end");
    pdMutex->lock();
    libpd_bang(pdReceiver);
    pdMutex->unlock();
}

void RwaRuntime::endBackgroundState()
{
    int intPatcherTag;
    RwaEntity *entity;
    RwaEntity::AssetMapItem item;
    string key;

    foreach(entity, entities)
    {
        std::map<string, RwaEntity::AssetMapItem>::iterator i = entity->backgroundAssets.begin();
        while(i != entity->backgroundAssets.end())
        {
            key = i->first;
            item = i->second;
            intPatcherTag = item.getPatcherTag();
            i = entity->backgroundAssets.erase(i);
            releasePatcherFromItem(item);
            resetPatcher(intPatcherTag);
            if(logSim)
                qInfo() << "Free Background Asset: " << intPatcherTag;
        }
    }
}

void RwaRuntime::freeAllPatchers()
{
    int intPatcherTag;
    RwaEntity *entity = nullptr;
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
            i = entity->activeAssets.erase(i);
            releasePatcherFromItem(item);
            resetPatcher(intPatcherTag);
        }
    }

    foreach(item, assetsPendingRelease)
    {
        releasePatcherFromItem(item);
        resetPatcher(item.getPatcherTag());
    }
    assetsPendingRelease.clear();

    endBackgroundState();
    if(entity)
        entity->reset();
}

/**
  Sends the release protocol ("<tag>-free", "<tag>-fadeouttime 0", "<tag>-end") to every
  pooled patcher, busy or not, and clears the busy flags.

  freeAllPatchers() resets only the patchers of *active* assets. A patcher whose asset
  already ended but is still fading out has left activeAssets, so nothing re-arms its
  [delay] on stop - its clock survives in Pd's clock queue (the pooled patchers are never
  closed) and fires the remaining fade time into the *next* simulation, switching the
  patch off and sending "<tag>-playfinished" for a tag a fresh asset may own by then.
  Addressing the whole pool forces every pending fade to zero length; the simulator then
  advances Pd's scheduler so all of them mature at stop (RwaSimulator::flushPdScheduler).

  Only messages from the existing patcher protocol are used, so every patch that follows
  it ends cleanly without patch-side changes. The dynamic patchers are not swept: they
  are closed on stop, and closing a canvas frees its objects' pending clocks with them.
*/
void RwaRuntime::resetAllPatchers()
{
    const struct { pdPatcher *pool; int count; } pools[] = {
        { monoPatchers, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { monoPatchersOgg, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { stereoPatchers, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { stereoPatchersOgg, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { binauralMonoPatchers_fabian, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { binauralMonoPatchersOgg_fabian, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { binauralStereoPatchers_fabian, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { binauralStereoPatchersOgg_fabian, RWARUNTIME_MAXNUMBEROFPATCHERS },
        { binaural5channelPatchers_fabian, RWARUNTIME_MAXNUMBEROF5CHANNELPATCHERS },
        { binaural7channelPatchers_fabian, RWARUNTIME_MAXNUMBEROF7CHANNELPATCHERS },
    };

    for(const auto &p : pools)
    {
        for(int i = 0; i < p.count; i++)
        {
            if(p.pool[i].patcherTag)
                resetPatcher(libpd_getdollarzero(p.pool[i].patcherTag));
            p.pool[i].isBusy = false;
        }
    }
}

void RwaRuntime::emptyPdMessageQueue()
{
#ifdef INIT_LIBPD_QUEUED
        pdMutex->lock();
        libpd_queued_receive_pd_messages();
        pdMutex->unlock();
#endif
}

/**
 * RwaSimulator::gameLoopTimer (T = 25ms) -> RwaSimulator::updateRwaGameState() -> RwaRuntime::update()
 */
void RwaRuntime::update(RwaEntity *entity)
{
    //if(!devicesRegistered)
    {
        emptyPdMessageQueue();
        sendData2activeAssets(entity);
        setEntityState(entity);
        processAssets(entity);
    }
    //sendData2Devices();
}
