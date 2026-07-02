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
    void intersect(Ray ray, Intersections& intersectionsList) override;

    vector<double> normal_at(const vector<double>& worldPoint) const override;

    bound local_bounds() const override; 
};

#endif

