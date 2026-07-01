#ifndef CYLINDER_H
#define CYLINDER_H

#include "Shape.h"

class Cylinder : public Shape {
public:
    Cylinder();

    void intersect(Ray ray, Intersections& intersectionsList) override;
    std::vector<double> normal_at(const std::vector<double>& worldPoint) const override;

    void setMin(double m) { min = m; }
    void setMax(double m) { max = m; }
    void setClosed(bool c) { closed = c; }

private:
    double min;
    double max;
    bool closed;

    void intersect_caps(const Ray& ray, Intersections& xs) const;
    bool check_cap(const Ray& ray, double t) const;
};

#endif
