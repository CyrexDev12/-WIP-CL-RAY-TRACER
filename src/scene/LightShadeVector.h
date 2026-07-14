#ifndef LIGHTSHADEVECTOR_H
#define LIGHTSHADEVECTOR_H
#include <iostream>
#include "core/math/Point3.h"
#include "core/math/Vec3.h"
#include <vector>
using namespace std;

/* We define this as a class that represents vectors from the point p 

- E is the eye vector, pointing from p to the origin of the ray.
- L is the light vector, poiting from p to the position of the light source.
- N is the surface normal, a vector that is perpendicular to the surface at P. 
- R is the reflection vector, poiting in the direction that incoming light would bounce, or reflect. 

*/

class Shape; 


struct LightShadeVector {
    clrt::math::Vec3 E; // Eye vector
    clrt::math::Vec3 L; // Light vector
    clrt::math::Vec3 N; // Surface normal
    clrt::math::Vec3 R; // Reflection vector

    void printVectors() {
        cout << "Eye Vector (E): ";
        cout << E.x << ' ' << E.y << ' ' << E.z;
        cout << "\nLight Vector (L): ";
        cout << L.x << ' ' << L.y << ' ' << L.z;
        cout << "\nSurface Normal (N): ";
        cout << N.x << ' ' << N.y << ' ' << N.z;
        cout << "\nReflection Vector (R): ";
        cout << R.x << ' ' << R.y << ' ' << R.z;
        cout << endl;
    }


    void CalculateEyeVector(const clrt::math::Vec3& rayDirection);
    void CalculateLightVector(const clrt::math::Point3& lightPosition, const clrt::math::Point3& point);
    void CalculateNormalVector(const clrt::math::Point3& point, const Shape& shape);

    // Transitional overloads for legacy tuple call sites.
    void CalculateEyeVector(const vector<double>& rayDirection);
    void CalculateLightVector(const vector<double>& lightPosition, const vector<double>& point);
    void CalculateNormalVector(const vector<double>& point, const Shape& shape);

    void CalculateReflectionVector();

};






#endif
