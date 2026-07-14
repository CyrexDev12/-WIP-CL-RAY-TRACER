#ifndef RAY_H
#define RAY_H
#include <vector>
#include "core/math/Point3.h"
#include "core/math/Ray.h"
#include "core/math/Vec3.h"
#include "math/Matrix.h"


// RAY CLASS 

// RAY CASTING: Creating a ray or line, and finding the intersections of that ray with the objects within the scene

// RAY Includes; A starting point Origin, and a vector called the direction which says where it points

// We think of the rays direction vector as its speed

struct Ray : clrt::math::Ray {
    using clrt::math::Ray::Ray;

    Ray() = default;
    Ray(const std::vector<double>& originTuple, const std::vector<double>& directionTuple);

    // Compute the point at the given distance t along the ray
    [[nodiscard]] clrt::math::Point3 position(double distance) const noexcept;
    [[nodiscard]] Ray transform(const clrt::math::Mat4& matrix) const;
    [[nodiscard]] Ray transform(const Matrix& matrix) const;

    // Debug function
    void printRay() const;
};

















#endif
