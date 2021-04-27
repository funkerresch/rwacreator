#include "rwascene.h"

RwaState::RwaState(std::string stateName) :
    RwaArea()
{
    locationType = RWALOCATIONTYPE_STATE;
    areaType = RWAAREATYPE_CIRCLE;
    myScene = nullptr;
    lastTouchedAsset = nullptr;
    selectedAssetIndex = -1;
    isGpsState = false;
    blockUntilRadiusHasBeenLeft = false;
    timeOut = 0;
    nextState = std::string();
    nextScene = std::string();
    hintState = std::string();
    requiredStates = std::list<std::string>();
    assets = std::list<RwaAsset1 *>();
    setObjectName(stateName);
    defaultPlaybackType = RWAPLAYBACKTYPE_BINAURAL;
    zoom = 18;
    setType(0);
    childrenFollowMe = 1;
    radius = 10;
    width = -1;
    height = -1;

    setLeaveAfterAssetsFinish(false);
    setLeaveOnlyAfterAssetsFinish(false);
    setEnterOnlyAfterAssetsFinish(false);
    setEnterOnlyOnce(false);
    isExclusive = false;
    enterOffset = -6;
    exitOffset = 0;
    positionLocked = false;
    childrenFollowMe = true;
}

RwaState::~RwaState() // Delete Audio Files from Disk??
{
    foreach(RwaAsset1 *asset, assets)
        delete asset;
}

void RwaState::copyAttributes(RwaState *dest)
{
    RwaAsset1 *asset;
    foreach(asset, this->getAssets())
    {
        RwaAsset1 *assetCopy = new RwaAsset1("", std::vector<double>(2, 0.0), RWA_UNDETERMINED, "");
        asset->copyAttributes(assetCopy);
        assetCopy->myState = dest;
        dest->assets.push_back(assetCopy);
    }

    for(uint32_t i = 0;i < this->corners.size(); i++)
        dest->corners.push_back(this->corners[i]);

    for(uint32_t i = 0;i < this->exitOffsetCorners.size(); i++)
        dest->exitOffsetCorners.push_back(this->exitOffsetCorners[i]);

    dest->locationType = this->locationType;
    dest->areaType = this->areaType;
    dest->gpsLocation = this->gpsLocation;
    dest->myScene = this->myScene;
    dest->lastTouchedAsset = this->lastTouchedAsset;
    dest->selectedAssetIndex = this->selectedAssetIndex;
    dest->isGpsState = this->isGpsState;
    dest->blockUntilRadiusHasBeenLeft = this->blockUntilRadiusHasBeenLeft;
    dest->timeOut = this->timeOut;
    dest->nextState = this->nextState;
    dest->nextScene = this->nextScene;
    dest->hintState = this->hintState;
    dest->defaultPlaybackType = this->defaultPlaybackType;
    dest->zoom = this->zoom;
    dest->setType(this->getType());
    dest->letChildrenFollowMe(this->childrenDoFollowMe());
    dest->radius = this->radius;
    dest->width = this->width;
    dest->height = this->height;
    dest->minimumStayTime = this->minimumStayTime;
    dest->leaveAfterAssetsFinish = this->getLeaveAfterAssetsFinish();
    dest->leaveOnlyAfterAssetsFinish = this->getLeaveOnlyAfterAssetsFinish();
    dest->enterOnlyAfterAssetsFinish = this->getEnterOnlyAfterAssetsFinish();
    dest->enterOnlyOnce =  this->getEnterOnlyOnce();
    dest->isExclusive = this->isExclusive;
    dest->enterOffset = this->enterOffset;
    dest->exitOffset = this->exitOffset;
    dest->positionLocked = this->positionLocked;
}

void RwaState::select(bool selected)
{
    isSelected = selected;
}

void RwaState::setScene(RwaScene *scene)
{
    myScene = scene;
}

RwaScene * RwaState::getScene()
{
    return myScene;
}

void RwaState::resetAssets()
{
    RwaAsset1 *item;
    foreach(item, assets)
        item->setBlocked(false);
}

void RwaState::addAsset(RwaAsset1 *item)
{
    if(defaultPlaybackType && !item->getPlaybackType())
    {
        item->setPlaybackType(defaultPlaybackType);
    }
    item->calculateChannelPositions();
    item->calculateReflectionPositions();
    assets.push_back(item);
    item->myState = this;
}

void RwaState::deleteAsset(const std::__1::string &path)
{
    foreach (RwaAsset1 *item, assets)
    {
        if(item->getFileName() == path)
        {
            assets.remove (item);
            qDebug() << "remove Asset";
            break;
        }

    }
}

RwaAsset1 *RwaState::getAsset(const std::__1::string &path)
{
    foreach (RwaAsset1 *item, assets)
    {
        if(item->getFileName() == path)
            return item;
    }
    return nullptr;
}

void RwaState::moveMyChildren(double dx, double dy)
{
    std::vector<double> newCoordinates(2, 0.0);
    foreach (RwaAsset1 *asset, assets)
    {
        newCoordinates[0] = (asset->getCoordinates()[0]+dx);
        newCoordinates[1] = (asset->getCoordinates()[1]+dy);
        asset->setCoordinates(newCoordinates);
        newCoordinates[0] = (asset->getStartPosition()[0]+dx);
        newCoordinates[1] = (asset->getStartPosition()[1]+dy);
        asset->setStartPosition(newCoordinates);
        asset->moveMyChildren(dx, dy);
    }

    if(!corners.empty())
    {
        for(uint32_t i = 0; i< corners.size();i++)
        {
           corners[i][0] = (corners[i][0] + dx);
           corners[i][1] = (corners[i][1] + dy);
        }
    }
}

bool RwaState::getEnterOnlyAfterAssetsFinish() const
{
    return enterOnlyAfterAssetsFinish;
}

void RwaState::setEnterOnlyAfterAssetsFinish(bool value)
{
    enterOnlyAfterAssetsFinish = value;
}

bool RwaState::getLeaveOnlyAfterAssetsFinish() const
{
    return leaveOnlyAfterAssetsFinish;
}

void RwaState::setLeaveOnlyAfterAssetsFinish(bool value)
{
    leaveOnlyAfterAssetsFinish = value;
}

bool RwaState::getLeaveAfterAssetsFinish() const
{
    return leaveAfterAssetsFinish;
}

void RwaState::setLeaveAfterAssetsFinish(bool value)
{
    leaveAfterAssetsFinish = value;
}

bool RwaState::getBlockUntilRadiusHasBeenLeft() const
{
    return blockUntilRadiusHasBeenLeft;
}

void RwaState::setBlockUntilRadiusHasBeenLeft(bool value)
{
    blockUntilRadiusHasBeenLeft = value;
}

std::string RwaState::getHintState() const
{
    return hintState;
}

void RwaState::setHintState(const std::string &value)
{
    hintState = value;
}

std::string RwaState::getNextState() const
{
    return nextState;
}

void RwaState::setNextState(const std::string &value)
{
    nextState = value;
}

std::string RwaState::getNextScene() const
{
    return nextScene;
}

void RwaState::setNextScene(const std::string &value)
{
    nextScene = value;
}

std::list<RwaAsset1 *> RwaState::getAssets() const
{
    return assets;
}

void RwaState::setAssets(const std::list<RwaAsset1 *> &value)
{
    assets = value;
}

std::list<std::string> RwaState::getRequiredStates() const
{
    return requiredStates;
}

void RwaState::setRequiredStates(const std::list<std::string> &value)
{
    requiredStates = value;
}

RwaAsset1 *RwaState::getLastTouchedAsset() const
{
    return lastTouchedAsset;
}

void RwaState::setLastTouchedAsset(RwaAsset1 *value)
{
    lastTouchedAsset = value;
}

bool RwaState::getEnterOnlyOnce() const
{
    return enterOnlyOnce;
}

void RwaState::setEnterOnlyOnce(bool value)
{
    enterOnlyOnce = value;
}

bool RwaState::getIsExclusive() const
{
    return isExclusive;
}

void RwaState::setIsExclusive(bool value)
{
    isExclusive = value;
}

int32_t RwaState::getDefaultPlaybackType() const
{
    return defaultPlaybackType;
}

void RwaState::setDefaultPlaybackType(const int32_t &value)
{
    defaultPlaybackType = value;
}

int32_t RwaState::getType() const
{
    return type;
}

void RwaState::setType(const int32_t &value)
{
    type = value;
    if(type == RWASTATETYPE_BACKGROUND)
    {
        if(this->myScene)
            this->myScene->setBackgroundState(this);
    }
}




