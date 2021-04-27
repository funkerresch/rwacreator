#ifndef RWAAREA_H
#define RWAAREA_H

#include "rwalocation1.h"
#include "rwautilities.h"

#define RWAAREATYPE_CIRCLE 1
#define RWAAREATYPE_RECTANGLE 2
#define RWAAREATYPE_SQUARE 3
#define RWAAREATYPE_POLYGON 4

#define RWAAREAOFFSETTYPE_ENTER 1
#define RWAAREAOFFSETTYPE_EXIT 2

using namespace std;

class RwaArea : public RwaLocation1 // TODO: getter & setter for corners and exitOffsetCorners
{
public:
    explicit RwaArea();
     ~RwaArea();

    vector <vector<double>> corners;
    vector <vector<double>> exitOffsetCorners;

    int32_t getZoom() const;
    void setZoom(int32_t value);

    int32_t getAreaType() const;
    void setAreaType(int32_t value);

    int32_t getRadius() const;
    void setRadius(int32_t value);

    int32_t getWidth() const;
    void setWidth(int32_t value);

    int32_t getHeight() const;
    void setHeight(int32_t value);

    float getEnterOffset() const;
    void setEnterOffset(float value);

    float getExitOffset() const;
    void setExitOffset(float value);

    float getTimeOut() const;
    void setTimeOut(float value);

    bool positionIsLocked() const;
    void lockPosition(bool value);

    bool childrenDoFollowMe() const;
    void letChildrenFollowMe(bool value);

    float getMinimumStayTime() const;
    void setMinimumStayTime(float value);

    void calculateEnterOffsetCorners();
    void calculateExitOffsetCorners();
    void moveCorners(double dx, double dy);

protected:
    int32_t zoom = 13;
    int32_t areaType = RWAAREATYPE_CIRCLE;
    int32_t radius = 10;
    int32_t width = -1;
    int32_t height = -1;

    bool positionLocked = false;
    bool childrenFollowMe = false;

    float enterOffset = 0;
    float exitOffset = 0;
    float timeOut = -1;
    float minimumStayTime = 4;
};

#endif // RWAAREA_H
