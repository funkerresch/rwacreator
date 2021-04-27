#ifndef RWALOCATION_H
#define RWALOCATION_H

#include <vector>
#include <string>
#include <math.h>

#ifdef QT_VERSION
#include <QDebug>
#endif

#define RWALOCATIONTYPE_ASSET 1
#define RWALOCATIONTYPE_ASSETSTARTPOSITION 2
#define RWALOCATIONTYPE_ASSETCHANNELPOSITION 3
#define RWALOCATIONTYPE_STATE 6
#define RWALOCATIONTYPE_SCENE 7
#define RWALOCATIONTYPE_AREA 8


class RwaLocation1
{

public:
    explicit RwaLocation1();
    virtual ~RwaLocation1();

    std::vector<double> getCoordinates() const;
    virtual void setCoordinates(const std::vector<double> &value);

    std::string objectName() const;
    void setObjectName(const std::string &name);

    int32_t getLocationType() const;
    void setLocationType(const int32_t value);

    virtual void moveMyChildren(double dx, double dy) = 0;


protected:
    int locationType;
    std::vector<double> gpsLocation;
    std::string name;
};

#endif // RWALOCATION_H
