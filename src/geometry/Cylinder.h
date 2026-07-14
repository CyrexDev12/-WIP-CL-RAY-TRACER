#ifndef CYLINDER_H
#define CYLINDER_H

#include "Shape.h"
#include "Bound.h"

class Cylinder : public Shape {
public:
    Cylinder();

    void intersect(const Ray& ray, Intersections& intersectionsList) override;
    clrt::math::Vec3 normalAt(const clrt::math::Point3& worldPoint) const override;

    void setMin(double m) { min = m; }
    void setMax(double m) { max = m; }
    void setClosed(bool c) { closed = c; }

private:
    double min;
    double max;
    bool closed;

    void intersect_caps(const Ray& ray, Intersections& xs) const;
    bool check_cap(const Ray& ray, double t) const;
    bound local_bounds() const override; 
};

#endif
