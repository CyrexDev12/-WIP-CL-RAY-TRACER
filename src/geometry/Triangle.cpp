#include "Triangle.h"
#include "Math/Operations.h"

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
    p1(point1),
    p2(point2),
    p3(point3)
{
    e1 = p2 - p1; 
    e2 = p3 - p1; 

    // The Ray Tracer Challenge uses cross(e2, e1)
    normal = NormalizeTuple(CrossProduct(e2, e1));
}

void Triangle::intersect(Ray ray, Intersections& intersectionsList) {
    // Convert world ray into triangle local space
    Ray localRay = ray.transform(this->getTransform().inverse());

    // Moller-Trumbore triangle intersection
    std::vector<double> dir_cross_e2 = CrossProduct(localRay.direction, e2);

    double det = CalculateDotProd(e1, dir_cross_e2);

    if (std::fabs(det) < EPSILON) {
        return;
    }

    double f = 1.0 / det;

    std::vector<double> p1_to_origin = localRay.origin - p1;

    double u = f * CalculateDotProd(p1_to_origin, dir_cross_e2);

    if (u < 0.0 || u > 1.0) {
        return;
    }

    std::vector<double> origin_cross_e1 = CrossProduct(p1_to_origin, e1);

    double v = f * CalculateDotProd(localRay.direction, origin_cross_e1);

    if (v < 0.0 || (u + v) > 1.0) {
        return;
    }

    double t = f * CalculateDotProd(e2, origin_cross_e1);

    // Adjust this line to match how your Intersections class stores intersections.
    intersectionsList.addIntersection(Intersection(t, this));
}

std::vector<double> Triangle::normal_at(const std::vector<double>& worldPoint) const {
    std::vector<double> localPoint = this->world_to_object(worldPoint);
    std::vector<double> localNormal = this->local_normal_at(localPoint);

    return this->normal_to_world(localNormal);
}

std::vector<double> Triangle::local_normal_at(const std::vector<double>& localPoint) const {
    return normal;
}

bound Triangle::local_bounds() const {
    double minX = std::min({p1[0], p2[0], p3[0]});
    double minY = std::min({p1[1], p2[1], p3[1]});
    double minZ = std::min({p1[2], p2[2], p3[2]});

    double maxX = std::max({p1[0], p2[0], p3[0]});
    double maxY = std::max({p1[1], p2[1], p3[1]});
    double maxZ = std::max({p1[2], p2[2], p3[2]});

    return bound(minX, minY, minZ, maxX, maxY, maxZ);
}

std::vector<double> Triangle::getP1() const {
    return p1;
}

std::vector<double> Triangle::getP2() const {
    return p2;
}

std::vector<double> Triangle::getP3() const {
    return p3;
}

std::vector<double> Triangle::getE1() const {
    return e1;
}

std::vector<double> Triangle::getE2() const {
    return e2;
}

std::vector<double> Triangle::getNormal() const {
    return normal;
}