#ifndef COMPUTATIONS_H
#define COMPUTATIONS_H

#include <vector>
#include "geometry/Intersection.h"
#include "geometry/Ray.h"



class Shape; 


struct Computations {
    double t; 
    const Shape* object; 


    vector<double> point; 
    vector<double> overPt; // Adjusted pt, slightly in the direction of the normal (Prevents self shadowing)
    vector<double> eyev; 
    vector<double> normalv; 
    vector<double> reflectv; // Reflection vector


    bool inside; 

    void print() const; 
};


Computations prepareComputations(const Intersection& Intersection, Ray ray);



#endif