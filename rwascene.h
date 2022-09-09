/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwascene.h
 * by Thomas Resch
 *
 */

#ifndef AFXSCENE_H
#define AFXSCENE_H

#include "rwaarea.h"
#include "rwastate.h"

#define RWASCENE_GPS 1
#define RWASCENE_BLUETOOTH 2
#define RWASCENE_WLAN 4
#define RWASCENE_2D 8
#define RWASCENE_3D 16

#define RWASCENEATTRIBUTE_FOLLOWINGASSETS 1
#define RWASCENEATTRIBUTE_ENTERONLYONCE 2
#define RWASCENEATTRIBUTE_EXCLUSIVE 3
#define RWASCENEATTRIBUTE_LOCKPOSITION 7
#define RWASCENEATTRIBUTE_DISABLEFALLBACK 8

#ifdef QT_VERSION
#include <QDebug>
#endif

class RwaScene : public RwaArea
{
public:
    explicit RwaScene(std::vector<double> gps = std::vector<double>());
    explicit RwaScene(std::string sceneName = std::string(), std::vector<double> gps = std::vector<double>(), int32_t zoom = 0);
    ~RwaScene();

    std::list<std::string> requiredScenes;
    std::list<std::string> requiredStates;
    std::string nextScene; // the scene to enter
    std::list <RwaState *> states;
    std::vector<double> currentViewCoordinates;

    RwaState *backgroundState;
    RwaState *lastTouchedState;
    int32_t level;
    bool disableFallback = false;

    void moveMyChildren(double dx, double dy);
    void clear();
    void addState();
    void addState(RwaState *state);
    void removeState(RwaState *state);
    void resetAssets();
    void setName(std::string name);
    void deselectAllStates();
    void findStateSurroundingArea();
    void moveScene2NewLocation(std::vector<double> newLocation);

    std::list<RwaState *> &getStates();
    RwaState *getState(string stateName);
    void setCurrentState(string stateName);
    RwaState *addState(string stateName, std::vector<double> gpsCoordinates);

    void setBackgroundState(RwaState *backgroundState);
    RwaState *getBackgroundState();

    std::list<string> getRequiredStates() const;
    void setRequiredStates(const std::list<string> &value);

    std::list<string> getRequiredScenes() const;
    void setRequiredScenes(const std::list<string> &value);

    std::string getNextScene() const;
    void setNextScene(const string &value);

    int32_t getLevel() const;
    void setLevel(int value);

    bool fallbackDisabled() const;
    void setDisableFallback(bool value);
    void InsertDefaultFallbackState();
    void InsertDefaultBackgroundState();
    void InsertStateAtIndex(RwaState *state, int32_t index);
    RwaState * createFallbackState();
    RwaState *createBackgroundState();
    void copyAttributes(RwaScene *dest);
};

#endif // AFXSCENE_H
