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

#ifndef AFXSTATE
#define AFXSTATE

#include "rwaasset1.h"

#define RWASTATETYPE_FALLBACK 1
#define RWASTATETYPE_BACKGROUND 2
#define RWASTATETYPE_GPS 3
#define RWASTATETYPE_BLUETOOTH 4
#define RWASTATETYPE_COMBINED 5
#define RWASTATETYPE_RANDOMGPS 6
#define RWASTATETYPE_HINT 7
#define RWASTATETYPE_OTHER 8

#define RWARANDOMGPS_CHURCH 1
#define RWARANDOMGPS_MONUMENT 2

#define RWASTATEATTRIBUTE_FOLLOWINGASSETS 1
#define RWASTATEATTRIBUTE_ENTERONLYONCE 2
#define RWASTATEATTRIBUTE_EXCLUSIVE 3
#define RWASTATEATTRIBUTE_LEAVEAFTERASSETSFINISH 4
#define RWASTATEATTRIBUTE_LEAVEONLYAFTERASSETSFINISH 5
#define RWASTATEATTRIBUTE_ENTERONLYAFTERASSETSFINISH 6
#define RWASTATEATTRIBUTE_LOCKPOSITION 7
#define RWASTATEATTRIBUTE_STATEWITHINSTATE 8

class RwaScene;

class RwaState : public RwaArea
{

public:
    explicit RwaState(string stateName = "");
    ~RwaState();

    std::list <RwaAsset1 *> assets;
    std::list<std::string> requiredStates;
    std::string nextScene; // the scene to enter
    std::string nextState;
    std::string hintState; // could not enter

    RwaScene *myScene; // the scene I belong to
    RwaAsset1 *lastTouchedAsset;

    int64_t stateNumber;
    int32_t selectedAssetIndex;
    int32_t type;
    int32_t defaultPlaybackType;

    bool isExclusive;
    bool enterOnlyOnce;
    bool leaveAfterAssetsFinish;
    bool leaveOnlyAfterAssetsFinish;
    bool enterOnlyAfterAssetsFinish;
    bool isGpsState;
    bool isSelected;
    bool isImmortal;
    bool blockUntilRadiusHasBeenLeft;
    bool sensorData2Pd;
    bool stateWithinState = false;

    void addAsset(RwaAsset1 *item);
    void deleteAsset(const std::string &path);
    void resetAssets();
    void select(bool selected);
    void setScene(RwaScene *scene);

    void moveMyChildren(double dx, double dy);

    RwaScene *getScene();
    RwaAsset1 *getAsset(const std::string &path);

    bool getIsExclusive() const;
    void setIsExclusive(bool value);

    bool getEnterOnlyOnce() const;
    void setEnterOnlyOnce(bool value);

    bool getBlockUntilRadiusHasBeenLeft() const;
    void setBlockUntilRadiusHasBeenLeft(bool value);

    bool getLeaveAfterAssetsFinish() const;
    void setLeaveAfterAssetsFinish(bool value);

    bool getLeaveOnlyAfterAssetsFinish() const;
    void setLeaveOnlyAfterAssetsFinish(bool value);

    bool getEnterOnlyAfterAssetsFinish() const;
    void setEnterOnlyAfterAssetsFinish(bool value);

    int32_t getType() const;
    void setType(const int32_t &value);

    int32_t getDefaultPlaybackType() const;
    void setDefaultPlaybackType(const int32_t &value);

    std::string getNextScene() const;
    void setNextScene(const std::string &value);

    std::string getNextState() const;
    void setNextState(const std::string &value);

    std::string getHintState() const;
    void setHintState(const std::string &value);

    std::list<std::string> getRequiredStates() const;
    void setRequiredStates(const std::list<string> &value);

    std::list<RwaAsset1 *> getAssets() const;
    void setAssets(const std::list<RwaAsset1 *> &value);

    RwaAsset1 *getLastTouchedAsset() const;
    void setLastTouchedAsset(RwaAsset1 *value);

    void copyAttributes(RwaState *dest);
    void setAttribute(uint32_t attribute, bool value);
};
#endif


