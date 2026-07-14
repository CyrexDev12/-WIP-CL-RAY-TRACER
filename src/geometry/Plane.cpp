#include "geometry/Plane.h"
#include "math/LegacyMathAdapters.h"
#include <cmath>
#include <limits>


// - Local Intersect: If the ray is parallel to the plane (ray.direction.y is close to 0), it misses completeley. Otherwise, t = -rayorigin.y  ray.direction.y 
void Plane::intersect(const Ray& ray, Intersections& intersectionsList) {
    const Ray localRay = ray.transform(getInverseTransform());

    const double EPSILON = 1e-5; // Small epsilon product to protect against floating pt issues 
    double local_ray_originY = localRay.origin.y;
    double local_ray_directionY = localRay.direction.y;

    // If the ray is moving parallel to the XZ plane, it misses
    if (std::abs(localRay.direction.y) < EPSILON) {
        return; // We return nothing no intersection exists 
    }

    // calculate the distance t to the intersection pt
    double t = -(local_ray_originY) / local_ray_directionY; 

    // Append to list

    intersectionsList.addIntersection(Intersection(t, this)); 

    return; 
}

clrt::math::Vec3 Plane::normalAt(const clrt::math::Point3&) const {
    return (getInverseTranspose() * clrt::math::Vec3{0.0, 1.0, 0.0})
        .normalized();
}


bound Plane::local_bounds() const {
    double inf = std::numeric_limits<double>::infinity();

    return bound(
        -inf, 0.0, -inf,
         inf, 0.0,  inf
    );
}
