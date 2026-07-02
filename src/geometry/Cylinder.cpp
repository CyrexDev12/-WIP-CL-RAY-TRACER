#include "Cylinder.h"
#include <cmath>
#include <algorithm>

const double EPSILON = 1e-6;

Cylinder::Cylinder()
    : min(-INFINITY), max(INFINITY), closed(false) {}


// --- Intersection ---
void Cylinder::intersect(Ray ray, Intersections& intersectionsList) {
    Ray localRay = ray.transform(this->getTransform().inverse());

    double a = localRay.direction[0] * localRay.direction[0] +
               localRay.direction[2] * localRay.direction[2];

    // Ray parallel to cylinder walls
    if (std::abs(a) < EPSILON) {
        intersect_caps(localRay, intersectionsList);
        return;
    }

    double b = 2 * (localRay.origin[0] * localRay.direction[0] +
                    localRay.origin[2] * localRay.direction[2]);

    double c = localRay.origin[0] * localRay.origin[0] +
               localRay.origin[2] * localRay.origin[2] - 1;

    double discriminant = b*b - 4*a*c;
    if (discriminant < 0) return;

    double sqrtDisc = std::sqrt(discriminant);

    double t0 = (-b - sqrtDisc) / (2*a);
    double t1 = (-b + sqrtDisc) / (2*a);

    if (t0 > t1) std::swap(t0, t1);

    double y0 = localRay.origin[1] + t0 * localRay.direction[1];
    if (min < y0 && y0 < max) {
        intersectionsList.addIntersection(Intersection(t0, this));
    }

    double y1 = localRay.origin[1] + t1 * localRay.direction[1];
    if (min < y1 && y1 < max) {
        intersectionsList.addIntersection(Intersection(t1, this));
    }

    intersect_caps(localRay, intersectionsList);
}


// --- Caps helpers ---
bool Cylinder::check_cap(const Ray& ray, double t) const {
    double x = ray.origin[0] + t * ray.direction[0];
    double z = ray.origin[2] + t * ray.direction[2];
    return (x*x + z*z) <= 1.0;
}

void Cylinder::intersect_caps(const Ray& ray, Intersections& xs) const {
    if (!closed || std::abs(ray.direction[1]) < EPSILON) return;

    // bottom
    double t = (min - ray.origin[1]) / ray.direction[1];
    if (check_cap(ray, t)) {
        xs.addIntersection(Intersection(t, this));
    }

    // top
    t = (max - ray.origin[1]) / ray.direction[1];
    if (check_cap(ray, t)) {
        xs.addIntersection(Intersection(t, this));
    }
}


// --- Normal ---
std::vector<double> Cylinder::normal_at(const std::vector<double>& worldPoint) const {
    Matrix inv = this->getTransform().inverse();
    std::vector<double> point = inv.multiplyTuple(worldPoint);

    double dist = point[0]*point[0] + point[2]*point[2];

    std::vector<double> objectNormal;

    // --- Cap normals (MUST be w=0)
    if (dist < 1.0 && point[1] >= max - EPSILON) {
        objectNormal = {0.0, 1.0, 0.0, 0.0};
    }
    else if (dist < 1.0 && point[1] <= min + EPSILON) {
        objectNormal = {0.0, -1.0, 0.0, 0.0};
    }
    else {
        objectNormal = {point[0], 0.0, point[2], 0.0};
    }

    // Transform normal to world space
    Matrix transInv = inv.transpose();
    std::vector<double> worldNormal = transInv.multiplyTuple(objectNormal);

    // Force correct tuple type
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

bound Cylinder::local_bounds() const {
    return bound(-1.0, min, -1.0,
                  1.0, max,  1.0);
}