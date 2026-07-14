#include "scene/LightShadeVector.h"
#include "geometry/Shape.h"
#include "math/LegacyMathAdapters.h"


// Negate the rays direction vector, turning it around to point back at its origin. 
void LightShadeVector::CalculateEyeVector(const clrt::math::Vec3& rayDirection) {
    E = (-rayDirection).normalized();
}

 // Subtract p from the position of the light source, giving you the vector poiting toward the light. 
void LightShadeVector::CalculateLightVector(const clrt::math::Point3& lightPosition, const clrt::math::Point3& point) {
    L = (lightPosition - point).normalized();
} 

void LightShadeVector::CalculateNormalVector(const clrt::math::Point3& point, const Shape& shape) {
    N = shape.normalAt(point);
}

void LightShadeVector::CalculateReflectionVector() {
    R = clrt::math::reflect(-L, N).normalized();
}

void LightShadeVector::CalculateEyeVector(const vector<double>& rayDirection) {
    CalculateEyeVector(clrt::compat::vectorFromLegacyTuple(rayDirection));
}

void LightShadeVector::CalculateLightVector(const vector<double>& lightPosition, const vector<double>& point) {
    CalculateLightVector(
        clrt::compat::pointFromLegacyTuple(lightPosition),
        clrt::compat::pointFromLegacyTuple(point)
    );
}

void LightShadeVector::CalculateNormalVector(const vector<double>& point, const Shape& shape) {
    CalculateNormalVector(clrt::compat::pointFromLegacyTuple(point), shape);
}

