#ifndef LIGHT_H
#define LIGHT_H
#include "core/math/Color.h"
#include "core/math/Point3.h"

// Base Class for all light types 
class Light {
    public:
    virtual ~Light() = default; 

    // Pure virtual functions that every light must provide
    virtual clrt::math::Color getIntensity() const = 0;
    virtual clrt::math::Point3 getPosition() const = 0;

};













#endif
