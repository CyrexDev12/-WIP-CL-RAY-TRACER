// Sphere.cpp
#include "geometry/Sphere.h"
#include "geometry/Intersection.h" // Needed to construct Intersection objects
#include "math/LegacyMathAdapters.h"
#include <cmath>

// Default constructor
Sphere::Sphere() {
    radius = 1.0;
    diameter = 2.0;             // Kept matching your header variable spelling
    position = {0.0, 0.0, 0.0}; // Center of the sphere at the origin
}

// Populates the intersections collection with concrete Intersection objects
void Sphere::intersect(const Ray& ray, Intersections& intersectionsList) {

    // 1. Apply the inverse of the sphere's transformation to the ray
    // transformMatrix is cleanly accessible here because it is protected in Shape
    const Ray transformedRay = ray.transform(inverseTransform);

    // 2. Compute the coefficients of the quadratic equation
    // (Using Object space: sphere is centered at origin with radius 1)
    const clrt::math::Vec3 objectOrigin{
        transformedRay.origin.x,
        transformedRay.origin.y,
        transformedRay.origin.z
    };
    double a = clrt::math::dot(transformedRay.direction, transformedRay.direction);
    double b = 2.0 * clrt::math::dot(transformedRay.direction, objectOrigin);
    double c = clrt::math::dot(objectOrigin, objectOrigin) - 1.0;
    
    // 3. Compute the discriminant
    double discriminant = b * b - 4.0 * a * c;
    
    if (discriminant < 0) {
        // No intersection
        return;
    } else {
        // Compute the two intersection points
        double t1 = (-b - sqrt(discriminant)) / (2.0 * a);
        double t2 = (-b + sqrt(discriminant)) / (2.0 * a);
        
        // 4. Create modular Intersection items containing the 't' value and 'this' sphere pointer
        // We pass 'this' because it is a pointer to the current Shape object instance
        intersectionsList.addIntersection(Intersection(t1, this));
        intersectionsList.addIntersection(Intersection(t2, this));
    }
}
/* CODE REVIEW */
/*this is where our bug is because intersect() correctly transforms the incoming ray by the inverse transform before doing sphere math,
but normal_at() currently transforms the normal with only the inverse matrix, not inverse-transpose.*/


clrt::math::Vec3 Sphere::normalAt(const clrt::math::Point3& worldPoint) const {
    const clrt::math::Point3 objectPoint = inverseTransform * worldPoint;
    const clrt::math::Vec3 objectNormal =
        objectPoint - clrt::math::Point3{0.0, 0.0, 0.0};
    return (inverseTranspose * objectNormal).normalized();
}

bound Sphere::local_bounds() const {
    return bound(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
}
