#ifndef COMPUTATIONS_H
#define COMPUTATIONS_H

#include "core/math/Point3.h"
#include "core/math/Vec3.h"
#include "geometry/Intersection.h"
#include "geometry/Ray.h"
#include "scene/ObjectResolver.h"



struct Computations {
    double t; 
    clrt::scene::ObjectId objectId;
    clrt::scene::MaterialId materialId;


    clrt::math::Point3 point;
    clrt::math::Point3 overPt; // Offset along the normal to prevent self-shadowing.
    clrt::math::Point3 underPt; // Offset beneath the surface for refraction rays.
    clrt::math::Vec3 eyev;
    clrt::math::Vec3 normalv;
    clrt::math::Vec3 reflectv;
    double n1{1.0};
    double n2{1.0};

    bool inside; 

    void print() const; 
};


// Returns a value from 0 to 1, this number returned is called the reflectance and represents what fraction of the light is reflected
double schlick(const Computations& comps);
Computations prepareComputations(
    const Intersection& intersection,
    const Ray& ray,
    const clrt::scene::ObjectResolver& resolver);
Computations prepareComputations(
    const Intersection& intersection,
    const Ray& ray,
    const Intersections& intersections,
    const clrt::scene::ObjectResolver& resolver);



#endif
