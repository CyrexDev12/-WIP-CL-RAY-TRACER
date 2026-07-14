#include "Triangle.h"
#include "math/Operations.h"
#include "math/LegacyMathAdapters.h"

#include <cmath>
#include <algorithm>

#ifndef EPSILON
#define EPSILON 0.00001
#endif

Triangle::Triangle(
    const std::vector<double>& point1,
    const std::vector<double>& point2,
    const std::vector<double>& point3
) : Shape(),
    p1(clrt::compat::pointFromLegacyTuple(point1)),
    p2(clrt::compat::pointFromLegacyTuple(point2)),
    p3(clrt::compat::pointFromLegacyTuple(point3)),
    e1(p2 - p1),
    e2(p3 - p1),
    normal(clrt::math::cross(e2, e1).normalized()) {}

Triangle::Triangle(
    const clrt::math::Point3& point1,
    const clrt::math::Point3& point2,
    const clrt::math::Point3& point3
) : Shape(),
    p1(point1),
    p2(point2),
    p3(point3),
    e1(p2 - p1),
    e2(p3 - p1),
    normal(clrt::math::cross(e2, e1).normalized()) {}

void Triangle::intersect(const Ray& ray, Intersections& intersectionsList) {
    // Convert world ray into triangle local space
    const Ray localRay = ray.transform(getInverseTransform());

    // Moller-Trumbore triangle intersection
    const clrt::math::Vec3 dir_cross_e2 =
        clrt::math::cross(localRay.direction, e2);

    double det = clrt::math::dot(e1, dir_cross_e2);

    if (std::fabs(det) < EPSILON) {
        return;
    }

    double f = 1.0 / det;

    const clrt::math::Vec3 p1_to_origin = localRay.origin - p1;

    double u = f * clrt::math::dot(p1_to_origin, dir_cross_e2);

    if (u < 0.0 || u > 1.0) {
        return;
    }

    const clrt::math::Vec3 origin_cross_e1 =
        clrt::math::cross(p1_to_origin, e1);

    double v = f * clrt::math::dot(localRay.direction, origin_cross_e1);

    if (v < 0.0 || (u + v) > 1.0) {
        return;
    }

    double t = f * clrt::math::dot(e2, origin_cross_e1);

    // Adjust this line to match how your Intersections class stores intersections.
    intersectionsList.addIntersection(Intersection(t, this));
}

clrt::math::Vec3 Triangle::normalAt(const clrt::math::Point3&) const {
    return normalToWorld(normal);
}

clrt::math::Vec3 Triangle::localNormalAt(const clrt::math::Point3&) const {
    return normal;
}

bound Triangle::local_bounds() const {
    double minX = std::min({p1.x, p2.x, p3.x});
    double minY = std::min({p1.y, p2.y, p3.y});
    double minZ = std::min({p1.z, p2.z, p3.z});

    double maxX = std::max({p1.x, p2.x, p3.x});
    double maxY = std::max({p1.y, p2.y, p3.y});
    double maxZ = std::max({p1.z, p2.z, p3.z});

    return bound(minX, minY, minZ, maxX, maxY, maxZ);
}

std::vector<double> Triangle::getP1() const {
    return clrt::compat::toLegacyTuple(p1);
}

std::vector<double> Triangle::getP2() const {
    return clrt::compat::toLegacyTuple(p2);
}

std::vector<double> Triangle::getP3() const {
    return clrt::compat::toLegacyTuple(p3);
}

std::vector<double> Triangle::getE1() const {
    return clrt::compat::toLegacyTuple(e1);
}

std::vector<double> Triangle::getE2() const {
    return clrt::compat::toLegacyTuple(e2);
}

std::vector<double> Triangle::getNormal() const {
    return clrt::compat::toLegacyTuple(normal);
}
