#ifndef CUBE_H
#define CUBE_H

#include "Shape.h"

class Cube : public Shape {
public:
    Cube() = default;
    virtual ~Cube() = default;

    void intersect(Ray ray, Intersections& intersectionsList) override;
    std::vector<double> normal_at(const std::vector<double>& worldPoint) const override;
    bound local_bounds() const override;

    
private:
    // Helper for slab intersection
    std::pair<double, double> check_axis(double origin, double direction) const;
};

#endif