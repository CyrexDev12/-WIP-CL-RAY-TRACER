#ifndef POINTLIGHT_H
#define POINTLIGHT_H
#include <vector>
#include "scene/Light.h"


// No size or shape, exists at a single point in space, defined by intensity (how bright it is), the intensity also describes the light source.



// Your updated PointLight class inheriting from Light
class PointLight : public Light {
private:
    clrt::math::Point3 position;
    clrt::math::Color intensity;

public:
    PointLight();

    PointLight(const clrt::math::Point3& position, const clrt::math::Color& intensity);
    PointLight(const std::vector<double>& position, const clrt::math::Color& intensity);

    clrt::math::Color getIntensity() const override { return intensity; }
    clrt::math::Point3 getPosition() const override { return position; }
};







#endif
