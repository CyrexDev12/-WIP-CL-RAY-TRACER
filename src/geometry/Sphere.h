// Sphere.h
#ifndef SPHERE_H
#define SPHERE_H

#include "geometry/Shape.h"
#include "Bound.h"


class Sphere : public Shape {
private: 
    double radius; 
    double diameter; // Spelled correctly from original 'diamater'

public:
    // Constructor sets default sphere radius and initializes inherited values
    Sphere(); 

    // Implements the specific math for a sphere
    void intersect(const Ray& ray, Intersections& intersectionsList) override;

    clrt::math::Vec3 normalAt(const clrt::math::Point3& worldPoint) const override;

    bound local_bounds() const override; 
};

#endif

