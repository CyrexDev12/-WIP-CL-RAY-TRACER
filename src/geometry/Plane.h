#ifndef PLANE_H
#define PLANE_H

#include "geometry/Shape.h"
#include "Bound.h"

//- Normal is always straight up at (0, 1, 0)
// - Local Intersect: If the ray is parallel to the plane (ray.direction.y is close to 0), it misses completeley. Otherwise, t = -rayorigin.y  ray.direction.y 
// Rests on the XZ 

class Plane : public Shape {

    private:

    public: 

    Plane() = default; 

    void intersect(Ray ray, Intersections& intersectionsList) override; 
    vector<double> normal_at(const vector<double>& worldPoint) const override;

    bound local_bounds() const override; 

};














#endif
