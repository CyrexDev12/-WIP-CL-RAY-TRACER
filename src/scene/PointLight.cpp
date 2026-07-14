#include "scene/PointLight.h"
#include "math/LegacyMathAdapters.h"



PointLight::PointLight() : position{0, 0, 0}, intensity{1, 1, 1} {}


// Constructor for PointLight, initializes position and intensity
// White Color default 
PointLight::PointLight(const clrt::math::Point3& position, const clrt::math::Color& intensity)
    : position(position), intensity(intensity) {}

PointLight::PointLight(const std::vector<double>& position, const clrt::math::Color& intensity)
    : PointLight(clrt::compat::pointFromLegacyTuple(position), intensity) {}


