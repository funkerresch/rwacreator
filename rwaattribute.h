#ifndef RWAATTRIBUTE_H
#define RWAATTRIBUTE_H

#include "rwalocation1.h"

#define RWAATTRIBUTETYPE_FLOAT 1
#define RWAATTRIBUTETYPE_STRING 2
#define RWAATTRIBUTETYPE_COORDINATE2D 3
#define RWAATTRIBUTETYPE_COORDINATE3D 4

#define RWAATTRIBUTE_INVALIDFLOAT -1000

class RwaAttribute
{
public:
    explicit RwaAttribute(std::string attributeName = std::string(), double floatValue = -1);
    explicit RwaAttribute(std::string attributeName = std::string(), std::string stringValue = std::string());

    std::string name;
    std::string stringValue;
    int32_t type;
    double floatValue;
};

#endif // RWAATTRIBUTE_H
