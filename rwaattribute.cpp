#include "rwaattribute.h"

RwaAttribute::RwaAttribute(std::string attributeName, double floatValue)
{
    this->name = attributeName;
    this->floatValue = floatValue;
    this->stringValue = std::string();
    this->type = RWAATTRIBUTETYPE_FLOAT;
}

RwaAttribute::RwaAttribute(std::string attributeName, std::string stringValue)
{
    this->name = attributeName;
    this->stringValue = stringValue;
    this->floatValue = RWAATTRIBUTE_INVALIDFLOAT;
    this->type = RWAATTRIBUTETYPE_STRING;
}



