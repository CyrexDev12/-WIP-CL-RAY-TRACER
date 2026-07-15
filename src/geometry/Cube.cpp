#include "Cube.h"
#include "math/LegacyMathAdapters.h"
#include <algorithm>
#include <cmath>

// Helper function: find intersection range along one axis
std::pair<double, double> Cube::check_axis(double origin, double direction) const {
    double tminNumerator = (-1.0 - origin);
    double tmaxNumerator = (1.0 - origin);

    double tmin, tmax;

    if (std::abs(direction) >= 1e-6) {
        tmin = tminNumerator / direction;
        tmax = tmaxNumerator / direction;
    } else {
        // Ray is parallel to planes → treat as infinite
        tmin = tminNumerator * INFINITY;
        tmax = tmaxNumerator * INFINITY;
    }

    if (tmin > tmax) std::swap(tmin, tmax);

    return { tmin, tmax };
}

// Ray–cube intersection
void Cube::intersect(const Ray& ray, Intersections& intersectionsList) {
    const Ray localRay = ray.transform(getInverseTransform());

    auto xt = check_axis(localRay.origin[0], localRay.direction[0]);
    auto yt = check_axis(localRay.origin[1], localRay.direction[1]);
    auto zt = check_axis(localRay.origin[2], localRay.direction[2]);

    double tmin = std::max({ xt.first, yt.first, zt.first });
    double tmax = std::min({ xt.second, yt.second, zt.second });

    if (tmin > tmax) return;

    intersectionsList.addIntersection(Intersection(tmin, getObjectId()));
    intersectionsList.addIntersection(Intersection(tmax, getObjectId()));
}


// Surface normal
clrt::math::Vec3 Cube::normalAt(const clrt::math::Point3& worldPoint) const {
    // convert to object space (point → w = 1 stays)
    const clrt::math::Point3 point = getInverseTransform() * worldPoint;

    double maxc = std::max({
        std::abs(point.x),
        std::abs(point.y),
        std::abs(point.z)
    });

    clrt::math::Vec3 objectNormal;

    if (maxc == std::abs(point.x)) {
        objectNormal = {point.x, 0.0, 0.0};
    } 
    else if (maxc == std::abs(point.y)) {
        objectNormal = {0.0, point.y, 0.0};
    } 
    else {
        objectNormal = {0.0, 0.0, point.z};
    }

    // Transform normal back (IMPORTANT: w must stay 0)
    const clrt::math::Vec3 worldNormal =
        getInverseTranspose() * objectNormal;

    // Force w = 0 (safety in case matrix math pollutes it)
    return worldNormal.normalized();
}


bound Cube::local_bounds() const {
    return bound(
        -1.0, -1.0, -1.0,
         1.0,  1.0,  1.0
    );
}
