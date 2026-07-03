#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Shape.h"
#include "Ray.h"
#include "Intersection.h"
#include "bound.h"

#include <vector>

class Triangle : public Shape {
private:
    std::vector<double> p1;
    std::vector<double> p2;
    std::vector<double> p3;

    std::vector<double> e1;
    std::vector<double> e2;

    std::vector<double> normal;

public:
    Triangle(
        const std::vector<double>& point1,
        const std::vector<double>& point2,
        const std::vector<double>& point3
    );

    void intersect(Ray ray, Intersections& intersectionsList) override;

    std::vector<double> normal_at(const std::vector<double>& worldPoint) const override;
    std::vector<double> local_normal_at(const std::vector<double>& localPoint) const;

    bound local_bounds() const override;

    std::vector<double> getP1() const;
    std::vector<double> getP2() const;
    std::vector<double> getP3() const;

    std::vector<double> getE1() const;
    std::vector<double> getE2() const;

    std::vector<double> getNormal() const;
};

#endif