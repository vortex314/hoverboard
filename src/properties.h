#ifndef __PROPERTIES_H__
#include <defines.h>

typedef struct
{
    int32_t speedTarget;
    int32_t steerTarget;
    int32_t angleTarget;
    int32_t angleMeasured;
    float speedLeft;  //  km/h
    float speedRight; //  km/h
    uint32_t hallSkippedLeft;
    uint32_t hallSkippedRight;
    float temperature; // °C
    float voltage;     // V
    float currentLeft; // A
    float currentRight;
    const char* version;
} Properties;

#endif
