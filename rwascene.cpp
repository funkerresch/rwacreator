#include "rwascene.h"

RwaScene::RwaScene(std::vector<double> gps) :
   RwaArea()
{
    locationType = RWALOCATIONTYPE_SCENE;
    gpsLocation[0] = gps[0];
    gpsLocation[1] = gps[1];

    RwaState *fallbackState = createFallbackState();
    addState(fallbackState);
    RwaState *backgroundState = createBackgroundState();
    addState(backgroundState);

    this->backgroundState = backgroundState;
    this->currentState = nullptr;
    this->zoom = 13;

    level = -1;
    areaType = RWAAREATYPE_CIRCLE;
    radius = 200;
    width = 200;
    height = 200;
    enterOffset = 0;
    exitOffset = 0;
    timeOut = 0;
    positionLocked = true;
    childrenFollowMe = true;
}

RwaScene::RwaScene(std::string sceneName, std::vector<double> gps, int32_t zoom) :
    RwaArea()
{
    locationType = RWALOCATIONTYPE_SCENE;
    setObjectName(sceneName);
    backgroundState = nullptr;
    currentState = nullptr;
    gpsLocation = gps;
    this->zoom = zoom;
    areaType = RWAAREATYPE_CIRCLE;
    radius = 200;
    width = -1;
    height = -1;
    level = -1;
    enterOffset = 0;
    exitOffset = 0;
    timeOut = 0;
    positionLocked = true;
    childrenFollowMe = true;
}

RwaScene::~RwaScene()
{
    foreach(RwaState *state, states)
        delete state;
}

void RwaScene::addState(RwaState *state)
{
    state->setScene(this);
    states.push_back(state);
}

RwaState *RwaScene::addState(std::string stateName,  std::vector<double> gpsCoordinates)
{
    qDebug() << QString::fromStdString(stateName);
    RwaState *newState = new RwaState(stateName);
    newState->setCoordinates(gpsCoordinates);
    newState->setScene(this);
    newState->setRadius(50);
    newState->setWidth(100);
    newState->setHeight(100);
    newState->isGpsState = true;
    newState->setType(RWASTATETYPE_GPS);
    newState->setDefaultPlaybackType(RWAPLAYBACKTYPE_BINAURAL);
    newState->isImmortal = false;
    states.push_back(newState);
    return newState;
}

RwaState * RwaScene::createFallbackState()
{
    RwaState *fallbackState = new RwaState("FALLBACK"); // this should be called after creating a new scene -> addFallbackState()
    fallbackState->setScene(this);
    fallbackState->setCoordinates(gpsLocation);
    fallbackState->setDefaultPlaybackType(RWAPLAYBACKTYPE_NATIVE);
    fallbackState->isImmortal = true;
    fallbackState->isGpsState = false;
    fallbackState->setType(RWASTATETYPE_FALLBACK);
    return fallbackState;
}

RwaState * RwaScene::createBackgroundState()
{
    RwaState *backgroundState = new RwaState("BACKGROUND"); // this should be called after creating a new scene -> addBackgroundState()
    backgroundState->setScene(this);
    backgroundState->setCoordinates(gpsLocation);
    backgroundState->setDefaultPlaybackType(RWAPLAYBACKTYPE_NATIVE);
    backgroundState->isImmortal = true;
    backgroundState->isGpsState = false;
    backgroundState->setType(RWASTATETYPE_BACKGROUND);
    return backgroundState;
}

void RwaScene::InsertStateAtIndex(RwaState *state, int32_t index)
{
    int i = 0;
    for(auto it=states.begin(); it!=states.end(); it++)
    {
        if(i == index)
        {
            states.insert(it, state);
            return;
        }
        i++;
    }
}

void RwaScene::InsertDefaultFallbackState()
{
    states.push_front(createFallbackState());
}

void RwaScene::InsertDefaultBackgroundState()
{
    int i = 0;
    for(auto it=states.begin(); it!=states.end(); it++)
    {
        if(i == 1)
        {
            states.insert(it, createBackgroundState());
            return;
        }
        i++;
    }
}

std::string RwaScene::getNextScene() const
{
    return nextScene;
}

void RwaScene::setNextScene(const std::string &value)
{
    nextScene = value;
}

void RwaScene::deselectAllStates()
{
    RwaState *state;
    foreach (state, states)
        state->select(false);
}

void RwaScene::findStateSurroundingArea()
{
    RwaState *state;
    std::vector<double> west(2,0.0), east(2,0.0), north(2,0.0), south(2,0.0);
    std::vector<double> middlePoint(2,0.0);

    if(states.empty())
    {
        return;
       // qDebug() << "EMPTY SCENE";
    }

    west = east = north = south = states.front()->getCoordinates();
    foreach (state, states)
    {
        if(state->getCoordinates()[0] < west[0])
            west = state->getCoordinates();
    }

    foreach (state, states)
    {
        if(state->getCoordinates()[0] > east[0])
            east = state->getCoordinates();
    }

    foreach (state, states)
    {
        if(state->getCoordinates()[1] > north[1])
            north = state->getCoordinates();
    }

    foreach (state, states)
    {
        if(state->getCoordinates()[1] < south[1])
            south = state->getCoordinates();
    }

    double difX = east[0] - west[0];
    middlePoint[0] = (west[0] + difX/2);

    double difY = north[1] - south[1];
    middlePoint[1] = (south[1] + difY/2);

    setCoordinates(middlePoint);
    double distX = RwaUtilities::calculateDistance1(west, east) * 1000 ;
    double distY = RwaUtilities::calculateDistance1(south, north) * 1000;
    if(distX <= 0)
        distX = 100;
    if(distY <= 0)
        distY = 100;
    setWidth(distX);
    setHeight(distY);
    setRadius(std::max(distX/2, distY/2));
    setAreaType(RWAAREATYPE_RECTANGLE);
}

void RwaScene::moveScene2NewLocation(std::vector<double> newLocation)
{
    double dx = newLocation[0] - getCoordinates()[0];
    double dy = newLocation[1] - getCoordinates()[1];
    setCoordinates(newLocation);
    moveMyChildren(dx, dy);
    moveCorners(dx, dy);
}

bool RwaScene::fallbackDisabled() const
{
    return disableFallback;
}

void RwaScene::setDisableFallback(bool value)
{
    disableFallback = value;
}

void RwaScene::moveMyChildren(double dx, double dy)
{
    std::vector<double> newCoordinates(2, 0.0);
    foreach (RwaState *state, this->getStates())
    {
        newCoordinates[0] = (state->getCoordinates()[0]+dx);
        newCoordinates[1] = (state->getCoordinates()[1]+dy);
        state->setCoordinates(newCoordinates);
        state->moveMyChildren(dx, dy);
    }
}

void RwaScene::clear()
{
    this->states.clear();
}

std::list <RwaState *> &RwaScene::getStates()
{
    return states;
}

RwaState * RwaScene::getState(string stateName)
{
    RwaState *state = nullptr;
    foreach(state, this->states)
    {
        if(state->objectName() == stateName)
            break;
    }
    return state;

}

void RwaScene::setCurrentState(string stateName)
{
    RwaState *state = nullptr;
    foreach(state, this->states)
    {
        if(state->objectName() == stateName)
            break;
    }

    currentState = state;
}

RwaState * RwaScene::getBackgroundState()
{
    return this->backgroundState;
}

void RwaScene::setBackgroundState(RwaState *backgroundState)
{
    if(backgroundState != NULL)
        this->backgroundState = backgroundState;
}



int32_t RwaScene::getLevel() const
{
    return level;
}

void RwaScene::setLevel(int value)
{
    level = value;
}

std::list<std::string> RwaScene::getRequiredScenes() const
{
    return requiredScenes;
}

void RwaScene::setRequiredScenes(const std::list<std::string> &value)
{
    requiredScenes = value;
}

std::list<std::string> RwaScene::getRequiredStates() const
{
    return requiredStates;
}

void RwaScene::setRequiredStates(const std::list<std::string> &value)
{
    requiredStates = value;
}

void RwaScene::addState()
{
    RwaState *newState = new RwaState(0);
    states.push_back(newState);
}

void RwaScene::resetAssets()
{
    RwaState *state;
    foreach(state, states)
        state->resetAssets();
}

void RwaScene::removeState(RwaState *state)
{
    states.remove(state);
}

void RwaScene::setName(string name)
{
    setObjectName(name);
}



