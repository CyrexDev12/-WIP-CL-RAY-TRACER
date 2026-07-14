#include "geometry/Computations.h"
#include "geometry/Shape.h"
#include <algorithm>
#include <cmath>
#include <iostream>




namespace {
bool sameIntersection(
    const Intersection& first,
    const Intersection& second
) {
    const double EPSILON = 1e-5;

    return first.getObject() == second.getObject() &&
           std::fabs(first.getT() - second.getT()) < EPSILON;
}
}


 double schlick(const Computations& comps) {
    // Find the cosine of the angle between the eye and normal vectors
    double cos = clrt::math::dot(comps.eyev, comps.normalv);

    // Total internal reflection can only occur if n1 > n2
    if (comps.n1 > comps.n2) {
        double n = comps.n1 / comps.n2;
        double sin2_t = n * n * (1.0 - cos * cos);

        // Total internal reflection
        if (sin2_t > 1.0) {
            return 1.0;
        }

        // Compute cosine of theta_t using trig identity
        double cos_t = sqrt(1.0 - sin2_t);

        // Use cos(theta_t) instead
        cos = cos_t;
    }

    double r0 = pow((comps.n1 - comps.n2) / (comps.n1 + comps.n2), 2);

    return r0 + (1.0 - r0) * pow(1.0 - cos, 5);
}

Computations prepareComputations(const Intersection& intersection, const Ray& ray, const Intersections& intersections) {
    Computations comps;

    comps.t = intersection.getT();
    comps.object = intersection.getObject();

    comps.point = ray.position(comps.t);
    comps.eyev = -ray.direction;
    comps.normalv = comps.object->normalAt(comps.point);

    if (clrt::math::dot(comps.normalv, comps.eyev) < 0) {
        comps.inside = true;
        comps.normalv = -comps.normalv;
    } else {
        comps.inside = false;
    }

    // Offset points prevent reflection and refraction rays from intersecting the surface they originate from, which can cause artifacts in the rendered image.
    // Over Point = point + (normal * epsilon)
    const double EPSILON = 1e-5;
    const clrt::math::Vec3 normalOffset = comps.normalv * EPSILON;
    comps.overPt = comps.point + normalOffset;
    comps.underPt = comps.point - normalOffset;

    // Reflection Vector 
    // Compute reflectv by reflecting the ray's direction vector around the objects normal vector 
    comps.reflectv = clrt::math::reflect(ray.direction, comps.normalv);

// Track which transparent objects the ray is currently inside.
vector<const Shape*> containers;

for (const Intersection& current :
     intersections.getIntersections()) {

    bool isCurrentIntersection =
        sameIntersection(current, intersection);

    // n1 is the refractive index of the material being exited.
    if (isCurrentIntersection) {
        comps.n1 = containers.empty()
            ? 1.0
            : containers.back()
                  ->getMaterial()
                  .refractiveIndex;
    }

    const Shape* currentObject = current.getObject();

    auto found = std::find(
        containers.begin(),
        containers.end(),
        currentObject
    );

    if (found != containers.end()) {
        containers.erase(found);
    } else {
        containers.push_back(currentObject);
    }

    // n2 is the refractive index of the material being entered.
    if (isCurrentIntersection) {
        comps.n2 = containers.empty()
            ? 1.0
            : containers.back()
                  ->getMaterial()
                  .refractiveIndex;

        break;
    }
}

    return comps;
}

Computations prepareComputations(const Intersection& intersection,const Ray& ray) {
    Intersections intersections;
    intersections.addIntersection(intersection);

    return prepareComputations(
        intersection,
        ray,
        intersections
    );
}


void Computations::print() const {
    std::cout << "--- Computations ---\n";

    std::cout << "t: " << t << "\n";
    std::cout << "object address: " << object << "\n";
    std::cout << "point: " << point.x << ' ' << point.y << ' ' << point.z << "\n";
    std::cout << "eyev: " << eyev.x << ' ' << eyev.y << ' ' << eyev.z << "\n";
    std::cout << "normalv: " << normalv.x << ' ' << normalv.y << ' ' << normalv.z << "\n";
    std::cout << "reflectionv: " << reflectv.x << ' ' << reflectv.y << ' ' << reflectv.z << "\n";
    std::cout << "underPt: " << underPt.x << ' ' << underPt.y << ' ' << underPt.z << "\n";
    std::cout << "n1: " << n1 << "\n";
    std::cout << "n2: " << n2 << "\n";
    std::cout << "inside: " << (inside ? "true" : "false") << "\n";

}
