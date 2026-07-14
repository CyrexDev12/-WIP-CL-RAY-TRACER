#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Shape.h"
#include "Ray.h"
#include "Intersection.h"
#include "Bound.h"

#include <vector>

class Triangle : public Shape {
private:
    clrt::math::Point3 p1;
    clrt::math::Point3 p2;
    clrt::math::Point3 p3;

    clrt::math::Vec3 e1;
    clrt::math::Vec3 e2;

    clrt::math::Vec3 normal;

public:
    Triangle(
        const std::vector<double>& point1,
        const std::vector<double>& point2,
        const std::vector<double>& point3
    );
    Triangle(
        const clrt::math::Point3& point1,
        const clrt::math::Point3& point2,
        const clrt::math::Point3& point3
    );

    void intersect(const Ray& ray, Intersections& intersectionsList) override;

    clrt::math::Vec3 normalAt(const clrt::math::Point3& worldPoint) const override;
    clrt::math::Vec3 localNormalAt(const clrt::math::Point3& localPoint) const;

    bound local_bounds() const override;

    std::vector<double> getP1() const;
    std::vector<double> getP2() const;
    std::vector<double> getP3() const;

    std::vector<double> getE1() const;
    std::vector<double> getE2() const;

    std::vector<double> getNormal() const;
};

#endif
