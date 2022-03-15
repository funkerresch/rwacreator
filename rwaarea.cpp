#include "rwaarea.h"
#include <QDebug>

RwaArea::RwaArea() :
    RwaLocation1()
{

}

RwaArea::~RwaArea()
{
    //delete corners;
}

int32_t RwaArea::getZoom() const
{
    return zoom;
}

void RwaArea::setZoom(int32_t value)
{
    zoom = value;
}

int32_t RwaArea::getAreaType() const
{
    return areaType;
}

void RwaArea::setAreaType(int32_t value)
{
    areaType = value;

    if(areaType == RWAAREATYPE_RECTANGLE || areaType == RWAAREATYPE_SQUARE || areaType == RWAAREATYPE_POLYGON)
    {
        if(width <= 0 || height <= 0)
        {
            width = radius*2;
            height = radius*2;
        }
    }

    if(corners.empty())
    {
        std::vector<double> corner(2, 0.0);
        std::vector<double> tmpWest(2, 0.0);
        std::vector<double> tmpEast(2, 0.0);

        tmpWest = RwaUtilities::calculateDestination1(getCoordinates(), getWidth()/2, 270);
        tmpEast = RwaUtilities::calculateDestination1(getCoordinates(), getWidth()/2, 90);

        corner = RwaUtilities::calculateDestination1(tmpWest, getHeight()/2, 0);
        //qDebug() << corner.x() << " " << corner.y();
        corners.push_back(corner);

        corner = RwaUtilities::calculateDestination1(tmpEast, getHeight()/2, 0);
        corners.push_back(corner);

        corner = RwaUtilities::calculateDestination1(tmpEast, getHeight()/2, 180);
        corners.push_back(corner);

        corner = RwaUtilities::calculateDestination1(tmpWest, getHeight()/2, 180);
        corners.push_back(corner);

        //calculateEnterOffsetCorners();
        calculateExitOffsetCorners();
        //qDebug() << "INIT POLYGON";
    }
    else
    {
        //calculateEnterOffsetCorners();
        calculateExitOffsetCorners();
    }
}

int32_t RwaArea::getRadius() const
{
    return radius;
}

void RwaArea::setRadius(int32_t value)
{
    radius = value;
}

int32_t RwaArea::getWidth() const
{
    return width;
}

void RwaArea::setWidth(int32_t value)
{
    width = value;
}

int32_t RwaArea::getHeight() const
{
    return height;
}

void RwaArea::setHeight(int32_t value)
{
    height = value;
}

float RwaArea::getEnterOffset() const
{
    return enterOffset;
}

void RwaArea::setEnterOffset(float value)
{
    enterOffset = value;
    if(!corners.empty())
    {
        calculateEnterOffsetCorners();
    }
}

void RwaArea::calculateEnterOffsetCorners()
{
    //enterOffsetCorners->clear();
    //RwaUtilities::calculatePolygonOffset1(enterOffset, corners, enterOffsetCorners);
    //qDebug() << "calcEnterOffsetCorners: " << enterOffsetCorners->count();

}

void RwaArea::calculateExitOffsetCorners()
{
    exitOffsetCorners.clear();
    RwaUtilities::calculatePolygonOffset2(exitOffset, corners, exitOffsetCorners);
    //qDebug() << "calcExitOffsetCorners: " << enterOffsetCorners->count();
}

float RwaArea::getExitOffset() const
{
    return exitOffset;
}

void RwaArea::setExitOffset(float value)
{
    exitOffset = value;
    if(!corners.empty())
    {
        calculateExitOffsetCorners();
    }
}

float RwaArea::getTimeOut() const
{
    return timeOut;
}

void RwaArea::setTimeOut(float value)
{
    timeOut = value;
}

bool RwaArea::positionIsLocked() const
{
    return positionLocked;
}

void RwaArea::lockPosition(bool value)
{
    positionLocked = value;
}

bool RwaArea::childrenDoFollowMe() const
{
    return childrenFollowMe;
}

void RwaArea::letChildrenFollowMe(bool value)
{
    childrenFollowMe = value;
}

void RwaArea::moveCorners(double dx, double dy)
{
    if(!corners.empty())
    {
        for(uint_fast32_t i = 0; i< corners.size(); i++)
        {
           corners[i][0] = (corners[i][0] + dx);
           corners[i][1] = (corners[i][1] + dy);
        }
    }
}

float RwaArea::getMinimumStayTime() const
{
    return minimumStayTime;
}

void RwaArea::setMinimumStayTime(float value)
{
    minimumStayTime = value;
}
