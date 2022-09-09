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

#include "rwaarea.h"
#include <QDebug>

RwaArea::RwaArea() :
    RwaLocation1()
{

}

RwaArea::~RwaArea()
{
    foreach(std::vector<double> corner, corners)
        std::vector<double>().swap(corner);

    std::vector<std::vector<double>>().swap(corners);

    foreach(std::vector<double> corner, exitOffsetCorners)
        std::vector<double>().swap(corner);

    std::vector<std::vector<double>>().swap(exitOffsetCorners);

    qDebug() << "Deleted Corners";
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
        corners.push_back(corner);

        corner = RwaUtilities::calculateDestination1(tmpEast, getHeight()/2, 0);
        corners.push_back(corner);

        corner = RwaUtilities::calculateDestination1(tmpEast, getHeight()/2, 180);
        corners.push_back(corner);

        corner = RwaUtilities::calculateDestination1(tmpWest, getHeight()/2, 180);
        corners.push_back(corner);

        calculateExitOffsetCorners();
    }
    else
        calculateExitOffsetCorners();
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

void RwaArea::calculateExitOffsetCorners()
{
    exitOffsetCorners.clear();
    RwaUtilities::calculatePolygonOffset2(exitOffset, corners, exitOffsetCorners);
}

float RwaArea::getExitOffset() const
{
    return exitOffset;
}

void RwaArea::setExitOffset(float value)
{
    exitOffset = value;
    if(!corners.empty())
        calculateExitOffsetCorners();
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
