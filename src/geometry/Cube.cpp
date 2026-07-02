#include "Cube.h"
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
void Cube::intersect(Ray ray, Intersections& intersectionsList) {
    Ray localRay = ray.transform(this->getTransform().inverse());

    auto xt = check_axis(localRay.origin[0], localRay.direction[0]);
    auto yt = check_axis(localRay.origin[1], localRay.direction[1]);
    auto zt = check_axis(localRay.origin[2], localRay.direction[2]);

    double tmin = std::max({ xt.first, yt.first, zt.first });
    double tmax = std::min({ xt.second, yt.second, zt.second });

    if (tmin > tmax) return;

    intersectionsList.addIntersection(Intersection(tmin, this));
    intersectionsList.addIntersection(Intersection(tmax, this));
}


// Surface normal
std::vector<double> Cube::normal_at(const std::vector<double>& worldPoint) const {
    Matrix inv = this->getTransform().inverse();

    // convert to object space (point → w = 1 stays)
    std::vector<double> point = inv.multiplyTuple(worldPoint);

    double maxc = std::max({
        std::abs(point[0]),
        std::abs(point[1]),
        std::abs(point[2])
    });

    std::vector<double> objectNormal;

    if (maxc == std::abs(point[0])) {
        objectNormal = { point[0], 0.0, 0.0, 0.0 };
    } 
    else if (maxc == std::abs(point[1])) {
        objectNormal = { 0.0, point[1], 0.0, 0.0 };
    } 
    else {
        objectNormal = { 0.0, 0.0, point[2], 0.0 };
    }

    // Transform normal back (IMPORTANT: w must stay 0)
    Matrix transInv = inv.transpose();
    std::vector<double> worldNormal = transInv.multiplyTuple(objectNormal);

    // Force w = 0 (safety in case matrix math pollutes it)
    worldNormal[3] = 0.0;

    // Normalize (ignore w)
    double mag = std::sqrt(
        worldNormal[0]*worldNormal[0] +
        worldNormal[1]*worldNormal[1] +
        worldNormal[2]*worldNormal[2]
    );

    worldNormal[0] /= mag;
    worldNormal[1] /= mag;
    worldNormal[2] /= mag;

    return worldNormal;
}


bound Cube::local_bounds() const {
    return bound(
        -1.0, -1.0, -1.0,
         1.0,  1.0,  1.0
    );
}