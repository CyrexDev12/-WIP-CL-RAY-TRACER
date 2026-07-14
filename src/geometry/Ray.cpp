#include "geometry/Ray.h"

#include <iostream>

#include "math/LegacyMathAdapters.h"

Ray::Ray(
    const std::vector<double>& originTuple,
    const std::vector<double>& directionTuple
) : clrt::math::Ray(
        clrt::compat::pointFromLegacyTuple(originTuple),
        clrt::compat::vectorFromLegacyTuple(directionTuple)
    ) {}

clrt::math::Point3 Ray::position(double distance) const noexcept {
    return clrt::math::Ray::position(distance);
}

Ray Ray::transform(const Matrix& matrix) const {
    return transform(clrt::compat::matrixFromLegacy(matrix));
}

Ray Ray::transform(const clrt::math::Mat4& matrix) const {
    return Ray{matrix * origin, matrix * direction};
}

void Ray::printRay() const {
    std::cout << "Ray Origin: "
              << origin.x << ' ' << origin.y << ' ' << origin.z
              << " 1\nRay Direction: "
              << direction.x << ' ' << direction.y << ' ' << direction.z
              << " 0\n";
}
