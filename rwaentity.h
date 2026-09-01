/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwaentity.h
 * by Thomas Resch
 */

#ifndef RWAENTITY_H
#define RWAENTITY_H

#include "rwascene.h"
#include "rwaattribute.h"
#include "rwaasset1.h"

#define RWAENTITYTYPE_HERO 1
#define RWAENTITYTYPE_NPC 2
#define RWAENTITYTYPE_OBJECT 3

// in order to imitate real behavior on the smartphone, every entity needs a copy of the whole game
// including assets etc...

class RwaEntity : public RwaLocation1
{

public:
    class AssetMapItem
    {
    public:
        explicit AssetMapItem(RwaAsset1 *item = nullptr, qint32 patcherTag = -1);
        int32_t getPatcherTag();
        RwaAsset1 *getAssetItem();

    private:
        RwaAsset1 *asset;
        int32_t patcherTag;
    };

    explicit RwaEntity(string entityName = string(), int32_t type = RWAENTITYTYPE_HERO);

    std::list <RwaScene *> scenes;
    std::list <RwaAttribute *> attributes;
    std::list<RwaAsset1 *> assets2unblock;
    std::map<string, AssetMapItem> activeAssets;
    std::map<string, AssetMapItem> backgroundAssets;
    std::list<string> visitedStates;

    int32_t type;

    float timeInCurrentState;
    float timeInCurrentScene;

    void setElevation(int32_t elevation);
    void setAzimuth(int32_t azimuth);

    void loadGameScript();
    void addAttribute(string attributeName, double floatValue);
    void addAttribute(string attributeName, string stringValue);
    void removeAttribute(string attributeName);
    void setCurrentScene(RwaScene *currentScene);
    void setCurrentState(RwaState *currentState);
    void addActiveAsset(string assetName, RwaAsset1 *item, int32_t patcherTag);
    void addBackgroundAsset(string assetName, RwaAsset1 *item, int32_t patcherTag);
    void addTimeInCurrentState(float value);
    void addTimeInCurrentScene(float value);
    void appendAsset2unblock(RwaAsset1 *item);
    void clearAssets2unblock();
    bool isActiveAsset(string name);

    string getStringAttribute(string name);
    double getFloatAttribute(string name);

    float getTimeInCurrentState() const;
    void setTimeInCurrentState(float value);

    float getTimeInCurrentScene() const;
    void setTimeInCurrentScene(float value);

    float getTime() const;
    void setTime(float value);

    void reset();
    void moveMyChildren(double dx, double dy);

    RwaScene *getCurrentScene();
    RwaState *getCurrentState();
    RwaScene *getScene(std::string sceneName);
    int32_t azimuth();
    int32_t elevation();

private:
    RwaScene *currentScene;
    RwaState *currentState;
    std::vector<float> headOrientation;
};

#endif // RWAENTITY_H
