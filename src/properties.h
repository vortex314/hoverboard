#ifndef __PROPERTIES_H__

typedef struct
{
    int32_t speedTarget;
    int32_t steerTarget;
    float speedLeft;  //  km/h
    float speedRight; //  km/h
    uint32_t hallSkippedLeft;
    uint32_t hallSkippedRight;
    float temperature; // °C
    float voltage;     // V
    float currentLeft; // A
    float currentRight;
} Properties;

#endif
