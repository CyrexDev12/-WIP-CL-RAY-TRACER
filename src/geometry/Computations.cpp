#include "geometry/Computations.h"
#include "geometry/Shape.h"
#include "scene/LightShadeVector.h"
#include <algorithm>
#include <cmath>




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

Computations prepareComputations(const Intersection& intersection, const Ray& ray, const Intersections& intersections) {
    Computations comps;

    comps.t = intersection.getT();
    comps.object = intersection.getObject();

    comps.point = ray.position(comps.t);

    comps.eyev = {
        -ray.direction[0],
        -ray.direction[1],
        -ray.direction[2],
        0
    };

    comps.normalv = comps.object->normal_at(comps.point);

    if (CalculateDotProd(comps.normalv, comps.eyev) < 0) {
        comps.inside = true;

        comps.normalv = {
            -comps.normalv[0],
            -comps.normalv[1],
            -comps.normalv[2],
            0
        };
    } else {
        comps.inside = false;
    }

    // Offset points prevent reflection and refraction rays from intersecting the surface they originate from, which can cause artifacts in the rendered image.
    // Over Point = point + (normal * epsilon)
    const double EPSILON = 1e-5;
    vector<double> normEps = ScaleTuple(comps.normalv, EPSILON); 
    comps.overPt = AddTuples(comps.point, normEps); 
    comps.underPt = SubtractTuples(comps.point, normEps);

    // Reflection Vector 
    // Compute reflectv by reflecting the ray's direction vector around the objects normal vector 
    comps.reflectv = reflect(comps.normalv, ray.direction); 

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
    cout << "--- Computations ---\n";

    cout << "t: " << t << "\n";
    cout << "object address: " << object << "\n";

    cout << "point: ";
    for (double value : point) {
        cout << value << " ";
    }
    cout << "\n";

    cout << "eyev: ";
    for (double value : eyev) {
        cout << value << " ";
    }
    cout << "\n";

    cout << "normalv: ";
    for (double value : normalv) {
        cout << value << " ";
    }
    cout << "\n";

    cout << "reflectionv: ";
    for (double value : reflectv) {
        cout << value << " ";
    }
    cout << "\n";
    
    cout << "underPt: ";
    for (double value : underPt) {
        cout << value << " ";
    }
    cout << "\n";

    cout << "n1: " << n1 << "\n";
    cout << "n2: " << n2 << "\n";
    
    cout << "inside: " << (inside ? "true" : "false") << "\n";

}