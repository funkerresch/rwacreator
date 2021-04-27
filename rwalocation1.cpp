#include "rwalocation1.h"
#define RWA_EARTHRADIUS 6378137

RwaLocation1::RwaLocation1()
{
    gpsLocation = std::vector<double>(2, 0.0);
    name = std::string();
    locationType = 0;
}
RwaLocation1::~RwaLocation1()
{

}

std::string RwaLocation1::objectName() const
{
    return name;
}

void RwaLocation1::setObjectName(const std::string &name)
{
    this->name = name;
}

std::vector<double> RwaLocation1::getCoordinates() const
{
    return gpsLocation;
}

void RwaLocation1::setCoordinates(const std::vector<double> &value)
{
    gpsLocation = value;
}

int32_t RwaLocation1::getLocationType() const
{
    return locationType;
}

void RwaLocation1::setLocationType(const int32_t value)
{
    locationType = value;
}

double degrees2radians(double degrees)
{
    return degrees * (M_PI/180);
}

double radians2degrees(double radians)
{
    return radians * (180/M_PI);
}


