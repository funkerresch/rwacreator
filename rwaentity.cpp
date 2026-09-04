#include "rwaentity.h"
#include "rwautilities.h"
#include <cmath>

RwaEntity::RwaEntity(string entityName, int32_t type)
{
    setObjectName(entityName);
    this->type = type;
    timeInCurrentScene = 0;
    timeInCurrentState = 0;
    currentScene = nullptr;
    currentState = nullptr;
    headOrientation = std::vector<double>(3, 0.0); // so far only azimuth and elevation
    setCoordinates(std::vector<double>(2,0));
}

double RwaEntity::azimuth() const
{
    return headOrientation[0];
}

double RwaEntity::elevation() const
{
    return headOrientation[1];
}

void RwaEntity::setAzimuth(double azimuth)
{
    if(!std::isfinite(azimuth))
        return;
    headOrientation[0] = RwaUtilities::wrap360(azimuth);
}

void RwaEntity::setElevation(double elevation)
{
    if(!std::isfinite(elevation))
        return;
    // No clamp: pitch past vertical is a valid pose. Custom Pd patches in raw-head mode get the
    // full range; the source-relative elevation comes out of calculateRelativeDirection in [-90, 90].
    headOrientation[1] = RwaUtilities::wrap180(elevation);
}

void RwaEntity::moveMyChildren(double dx, double dy)
{

}

void RwaEntity::reset()
{
    timeInCurrentScene = 0;
    timeInCurrentState = 0;
    visitedStates.clear();
    activeAssets.clear();
    assets2unblock.clear();
}

RwaEntity::AssetMapItem::AssetMapItem(RwaAsset1 *item, qint32 patcherTag)
{
    this->asset = item;
    this->patcherTag = patcherTag;
}

RwaAsset1 *RwaEntity::AssetMapItem::getAssetItem()
{
    return this->asset;
}

int32_t RwaEntity::AssetMapItem::getPatcherTag()
{
    return this->patcherTag;
}

void RwaEntity::addAttribute(string attributeName, double floatValue)
{
    RwaAttribute *newAttribute = new RwaAttribute(attributeName, floatValue);
    attributes.push_back(newAttribute);
}

void RwaEntity::appendAsset2unblock(RwaAsset1 *item)
{
    assets2unblock.push_back(item);
}

void RwaEntity::clearAssets2unblock()
{
    assets2unblock.clear();
}

void RwaEntity::addActiveAsset(string assetName, RwaAsset1 *item, int32_t patcherTag)
{
    AssetMapItem newMapItem(item, patcherTag);
    this->activeAssets.insert( std::make_pair(assetName, newMapItem));
}

void RwaEntity::addBackgroundAsset(string assetName, RwaAsset1 *item, int32_t patcherTag)
{
    AssetMapItem newMapItem(item, patcherTag);
    this->backgroundAssets.insert(std::make_pair(assetName, newMapItem));
}

// when called by RwaRuntime::processAssets, argument assetName is set to asset->uniqueId
bool RwaEntity::isActiveAsset(string assetName)
{
    if(this->activeAssets.find(assetName) != this->activeAssets.end())
        return true;
    else
        return false;
}

float RwaEntity::getTimeInCurrentScene() const
{
    return timeInCurrentScene;
}

void RwaEntity::setTimeInCurrentScene(float value)
{
    timeInCurrentScene = value;
}

float RwaEntity::getTimeInCurrentState() const
{
    return timeInCurrentState;
}

void RwaEntity::setTimeInCurrentState(float value)
{
    timeInCurrentState = value;
}

void RwaEntity::addTimeInCurrentState(float value)
{
    timeInCurrentState += value;
}

void RwaEntity::addTimeInCurrentScene(float value)
{
    timeInCurrentScene += value;
}

RwaScene *RwaEntity::getScene(std::string sceneName)
{
    RwaScene *scene = nullptr;
    foreach(scene, scenes)
    {
        if(scene->objectName() == sceneName)
            return scene;
    }
    return scene;
}

RwaScene *RwaEntity::getCurrentScene()
{
    return currentScene;
}

RwaState *RwaEntity::getCurrentState()
{
    return currentState;
}

void RwaEntity::setCurrentScene(RwaScene *currentScene)
{
    if(this->currentScene == currentScene)
            return;
    this->currentScene = currentScene;
    this->currentState = nullptr;
}
void RwaEntity::setCurrentState(RwaState *currentState)
{
    this->currentState = currentState;
}

void RwaEntity::addAttribute(string attributeName, string stringValue)
{
    RwaAttribute *newAttribute = new RwaAttribute(attributeName, stringValue);
    attributes.push_back(newAttribute);
}

void RwaEntity::removeAttribute(string attributeName)
{
    RwaAttribute *ptr;
    foreach( ptr, attributes )
    {
        if(ptr->name == attributeName)
            attributes.remove (ptr);
    }
}

string RwaEntity::getStringAttribute(string attributeName)
{
    RwaAttribute *ptr;
    foreach( ptr, attributes )
    {
        if(ptr->name == attributeName)
            return ptr->stringValue;
    }
    return nullptr;
}

double RwaEntity::getFloatAttribute(string attributeName)
{
    RwaAttribute *ptr;
    foreach( ptr, attributes )
    {
        if(ptr->name == attributeName)
            return ptr->floatValue;
    }
    return 0;
}
