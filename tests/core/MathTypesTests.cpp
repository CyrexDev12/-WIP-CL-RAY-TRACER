#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "core/math/Color.h"
#include "core/math/Mat4.h"
#include "core/math/Point3.h"
#include "core/math/Ray.h"
#include "core/math/Vec3.h"

namespace {

using clrt::math::Color;
using clrt::math::Mat4;
using clrt::math::Point3;
using clrt::math::Ray;
using clrt::math::Scalar;
using clrt::math::Vec3;

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

void expectNear(Scalar actual, Scalar expected, const std::string& message) {
    expect(clrt::math::nearlyEqual(actual, expected, 1.0e-8), message);
}

void testVectorOperations() {
    const Vec3 first{1.0, 2.0, 3.0};
    const Vec3 second{2.0, 3.0, 4.0};

    expect(clrt::math::nearlyEqual(first + second, Vec3{3.0, 5.0, 7.0}),
           "vector addition");
    expectNear(clrt::math::dot(first, second), 20.0, "dot product");
    expect(clrt::math::nearlyEqual(
               clrt::math::cross(first, second), Vec3{-1.0, 2.0, -1.0}),
           "cross product");
    expectNear(Vec3{0.0, 3.0, 4.0}.normalized().length(), 1.0,
               "normalization");
    expect(clrt::math::nearlyEqual(
               clrt::math::reflect(Vec3{1.0, -1.0, 0.0}, Vec3{0.0, 1.0, 0.0}),
               Vec3{1.0, 1.0, 0.0}),
           "reflection");

    bool rejectedZeroVector = false;
    try {
        static_cast<void>(Vec3{}.normalized());
    } catch (const std::domain_error&) {
        rejectedZeroVector = true;
    }
    expect(rejectedZeroVector, "zero-vector normalization is rejected");
}

void testPointSemantics() {
    const Point3 point{1.0, 2.0, 3.0};
    const Vec3 offset{-2.0, 1.0, 0.5};

    expect(clrt::math::nearlyEqual(point + offset, Point3{-1.0, 3.0, 3.5}),
           "point plus vector produces a point");
    expect(clrt::math::nearlyEqual(
               Point3{4.0, 6.0, 8.0} - point, Vec3{3.0, 4.0, 5.0}),
           "point minus point produces a vector");
}

void testColorOperations() {
    const Color first{0.9, 0.6, 0.75};
    const Color second{0.7, 0.1, 0.25};
    expect(clrt::math::nearlyEqual(
               clrt::math::hadamard(first, second), Color{0.63, 0.06, 0.1875}),
           "Hadamard color product");
}

void testMatrixConventions() {
    const Point3 point{-3.0, 4.0, 5.0};
    const Vec3 vector{-3.0, 4.0, 5.0};
    const Mat4 translation = Mat4::translation(5.0, -3.0, 2.0);

    expect(clrt::math::nearlyEqual(
               translation * point, Point3{2.0, 1.0, 7.0}),
           "translation affects points");
    expect(clrt::math::nearlyEqual(translation * vector, vector),
           "translation does not affect vectors");

    const Mat4 composed = translation * Mat4::scaling(2.0, 3.0, 4.0);
    expect(clrt::math::nearlyEqual(
               composed * Point3{1.0, 1.0, 1.0}, Point3{7.0, 0.0, 6.0}),
           "column-vector composition applies the rightmost transform first");

    const Mat4 transform = Mat4::translation(2.0, -3.0, 4.0)
        * Mat4::rotationY(0.37)
        * Mat4::scaling(1.5, 0.5, 2.0);
    const Mat4 inverse = transform.inverse();
    expect(clrt::math::nearlyEqual(inverse * transform, Mat4::identity(), 1.0e-8),
           "matrix inverse round trip");
    expectNear(Mat4::scaling(2.0, 3.0, 4.0).determinant(), 24.0,
               "matrix determinant");

    const Mat4 defaultView = Mat4::viewTransform(
        Point3{0.0, 0.0, 0.0},
        Point3{0.0, 0.0, -1.0},
        Vec3{0.0, 1.0, 0.0});
    expect(clrt::math::nearlyEqual(defaultView, Mat4::identity()),
           "default camera looks down negative Z");
}

void testRayOperations() {
    const Ray ray{Point3{2.0, 3.0, 4.0}, Vec3{1.0, 0.0, 0.0}};
    expect(clrt::math::nearlyEqual(ray.position(2.5), Point3{4.5, 3.0, 4.0}),
           "ray position");

    const Ray moved = ray.transformed(Mat4::translation(3.0, 4.0, 5.0));
    expect(clrt::math::nearlyEqual(moved.origin, Point3{5.0, 7.0, 9.0}),
           "ray origin transform");
    expect(clrt::math::nearlyEqual(moved.direction, ray.direction),
           "ray direction ignores translation");
}

} // namespace

int main() {
    static_assert(std::is_standard_layout_v<Vec3>);
    static_assert(std::is_standard_layout_v<Point3>);
    static_assert(std::is_standard_layout_v<Color>);
    static_assert(std::is_trivially_copyable_v<Vec3>);
    static_assert(std::is_trivially_copyable_v<Point3>);
    static_assert(std::is_trivially_copyable_v<Color>);
    static_assert(std::is_trivially_copyable_v<Mat4>);
    static_assert(std::is_trivially_copyable_v<Ray>);
    static_assert(sizeof(Vec3) == sizeof(Scalar) * 3);
    static_assert(sizeof(Point3) == sizeof(Scalar) * 3);
    static_assert(sizeof(Color) == sizeof(Scalar) * 3);
    static_assert(sizeof(Mat4) == sizeof(Scalar) * 16);
    static_assert(sizeof(Ray) == sizeof(Scalar) * 6);

    testVectorOperations();
    testPointSemantics();
    testColorOperations();
    testMatrixConventions();
    testRayOperations();

    if (failures != 0) {
        std::cerr << failures << " fixed-math test(s) failed\n";
        return 1;
    }

    std::cout << "All fixed-math tests passed\n";
    return 0;
}
