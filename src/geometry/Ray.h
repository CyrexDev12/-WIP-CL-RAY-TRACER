#ifndef RAY_H
#define RAY_H
#include <iostream>
#include <vector>
#include "math/Operations.h"
#include "math/Matrix.h"


using namespace std; 


// RAY CLASS 

// RAY CASTING: Creating a ray or line, and finding the intersections of that ray with the objects within the scene

// RAY Includes; A starting point Origin, and a vector called the direction which says where it points

// We think of the rays direction vector as its speed

struct Ray {
    vector<double> origin; // POINT
    vector<double> direction; // VECTOR
    
    Ray() = default; 
    Ray(vector<double> org, vector<double> dir) : origin(org), direction(dir) {}

    // Compute the point at the given distance t along the ray
    vector<double> position(double t) const;
    Ray transform(Matrix m);

    // Debug function
    void printRay() {
        cout << "Ray Origin: ";
        for (double value : origin) {
            cout << value << " ";
        }
        cout << "\nRay Direction: ";
        for (double value : direction) {
            cout << value << " ";
        }
        cout << endl;
    }

    
};

















#endif