#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "core/math/Mat4.h"
#include "core/math/Point3.h"
#include "core/math/Vec3.h"
#include "geometry/Ray.h"
#include "geometry/Computations.h"
#include "geometry/Intersection.h"
#include "geometry/Plane.h"
#include "geometry/Sphere.h"
#include "geometry/Triangle.h"
#include "math/LegacyMathAdapters.h"
#include "math/Matrix.h"
#include "scene/Camera.h"
#include "scene/LightShadeVector.h"
#include "scene/Pattern.h"
#include "scene/World.h"
#include "scene/canvas.h"

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

void testTupleAdapters() {
    const clrt::math::Point3 point{1.0, 2.0, 3.0};
    const clrt::math::Vec3 vector{4.0, 5.0, 6.0};

    expect(clrt::math::nearlyEqual(
               clrt::compat::pointFromLegacyTuple(
                   clrt::compat::toLegacyTuple(point)),
               point),
           "point adapter round trip");
    expect(clrt::math::nearlyEqual(
               clrt::compat::vectorFromLegacyTuple(
                   clrt::compat::toLegacyTuple(vector)),
               vector),
           "vector adapter round trip");
}

void testMatrixAdapters() {
    const clrt::math::Mat4 fixed = clrt::math::Mat4::translation(2.0, 3.0, 4.0)
        * clrt::math::Mat4::rotationY(0.25)
        * clrt::math::Mat4::scaling(2.0, 1.0, 0.5);
    const Matrix legacy = clrt::compat::matrixToLegacy(fixed);
    expect(clrt::math::nearlyEqual(
               clrt::compat::matrixFromLegacy(legacy), fixed),
           "matrix adapter round trip");
}

void testMigratedRay() {
    const Ray ray(
        std::vector<double>{1.0, 2.0, 3.0, 1.0},
        std::vector<double>{0.0, 1.0, 0.0, 0.0});
    expect(clrt::math::nearlyEqual(ray.origin, clrt::math::Point3{1.0, 2.0, 3.0}),
           "legacy ray constructor converts its origin");
    expect(clrt::math::nearlyEqual(ray.direction, clrt::math::Vec3{0.0, 1.0, 0.0}),
           "legacy ray constructor converts its direction");

    Matrix matrix;
    const Matrix translation = matrix.translation(3.0, 4.0, 5.0);
    const Ray moved = ray.transform(translation);
    expect(clrt::math::nearlyEqual(moved.origin, clrt::math::Point3{4.0, 6.0, 8.0}),
           "legacy matrix transforms fixed ray origin");
    expect(clrt::math::nearlyEqual(moved.direction, ray.direction),
           "legacy translation leaves fixed ray direction unchanged");
}

void testMigratedCamera() {
    Camera camera(201, 101, M_PI / 2.0);
    const Ray centerRay = ray_for_pixel(camera, 100, 50);
    expect(clrt::math::nearlyEqual(
               centerRay.origin, clrt::math::Point3{0.0, 0.0, 0.0}),
           "camera center ray origin");
    expect(clrt::math::nearlyEqual(
               centerRay.direction, clrt::math::Vec3{0.0, 0.0, -1.0}),
           "camera center ray direction");

    const clrt::math::Mat4 view = clrt::math::Mat4::rotationY(M_PI / 4.0)
        * clrt::math::Mat4::translation(0.0, -2.0, 5.0);
    camera.setTransform(view);
    expect(clrt::math::nearlyEqual(
               camera.getInverseTransform() * camera.getTransform(),
               clrt::math::Mat4::identity(),
               1.0e-8),
           "camera caches the inverse transform");
}

void testMigratedShapeTransforms() {
    Sphere sphere;
    sphere.setTransform(clrt::math::Mat4::scaling(2.0, 2.0, 2.0));
    expect(clrt::math::nearlyEqual(
               sphere.getInverseTransform() * sphere.getTransform(),
               clrt::math::Mat4::identity()),
           "shape caches its inverse transform");

    const Ray ray{
        clrt::math::Point3{0.0, 0.0, -5.0},
        clrt::math::Vec3{0.0, 0.0, 1.0}
    };
    Intersections intersections;
    sphere.intersect(ray, intersections);
    const auto& hits = intersections.getIntersections();
    expect(hits.size() == 2, "scaled sphere produces two intersections");
    if (hits.size() == 2) {
        expect(clrt::math::nearlyEqual(hits[0].getT(), 3.0),
               "scaled sphere first intersection");
        expect(clrt::math::nearlyEqual(hits[1].getT(), 7.0),
               "scaled sphere second intersection");
    }
}

void testMigratedPlane() {
    Plane plane;
    plane.setTransform(clrt::math::Mat4::translation(0.0, 1.0, 0.0));
    const Ray ray{
        clrt::math::Point3{0.0, 3.0, 0.0},
        clrt::math::Vec3{0.0, -1.0, 0.0}
    };
    Intersections intersections;
    plane.intersect(ray, intersections);
    expect(intersections.getIntersections().size() == 1,
           "translated plane produces one intersection");
    if (!intersections.getIntersections().empty()) {
        expect(clrt::math::nearlyEqual(
                   intersections.getIntersections()[0].getT(), 2.0),
               "translated plane intersection uses object transform");
    }

    plane.setTransform(clrt::math::Mat4::rotationZ(M_PI / 2.0));
    const auto normalTuple = plane.normal_at({0.0, 0.0, 0.0, 1.0});
    const auto normal = clrt::compat::vectorFromLegacyTuple(normalTuple);
    expect(clrt::math::nearlyEqual(normal, clrt::math::Vec3{-1.0, 0.0, 0.0}),
           "rotated plane normal uses inverse transpose");
}

void testMigratedTriangle() {
    Triangle triangle(
        clrt::math::Point3{0.0, 1.0, 0.0},
        clrt::math::Point3{-1.0, 0.0, 0.0},
        clrt::math::Point3{1.0, 0.0, 0.0});
    const Ray ray{
        clrt::math::Point3{0.0, 0.5, -2.0},
        clrt::math::Vec3{0.0, 0.0, 1.0}
    };
    Intersections intersections;
    triangle.intersect(ray, intersections);
    expect(intersections.getIntersections().size() == 1,
           "fixed triangle intersection");
}

void testFixedHitComputations() {
    World world;
    auto sphere = std::make_unique<Sphere>();
    Shape& registeredSphere = world.AddShape(std::move(sphere));
    const Ray ray{
        clrt::math::Point3{0.0, 0.0, -5.0},
        clrt::math::Vec3{0.0, 0.0, 1.0}
    };
    const Computations computations = prepareComputations(
        Intersection{4.0, registeredSphere.getObjectId()},
        ray,
        world);

    expect(computations.objectId == registeredSphere.getObjectId(),
           "hit computations retain stable object ID");
    expect(computations.materialId == registeredSphere.getMaterialId(),
           "hit computations retain stable material ID");

    expect(clrt::math::nearlyEqual(computations.point, clrt::math::Point3{0.0, 0.0, -1.0}),
           "hit point uses fixed Point3");
    expect(clrt::math::nearlyEqual(computations.eyev, clrt::math::Vec3{0.0, 0.0, -1.0}),
           "hit eye direction uses fixed Vec3");
    expect(clrt::math::nearlyEqual(computations.normalv, clrt::math::Vec3{0.0, 0.0, -1.0}),
           "hit normal uses fixed Vec3");
    expect(computations.overPt.z < computations.point.z,
           "over point is offset above the surface");
}

void testFixedPatternAndLightVectors() {
    StripePattern stripes({1.0, 1.0, 1.0}, {0.0, 0.0, 0.0});
    stripes.setTransform(clrt::math::Mat4::scaling(2.0, 2.0, 2.0));
    expect(clrt::math::nearlyEqual(
               stripes.PatternAtPoint(clrt::math::Point3{2.5, 0.0, 0.0}),
               clrt::math::Color{0.0, 0.0, 0.0}),
           "pattern caches and applies its inverse transform");

    LightShadeVector vectors;
    vectors.L = clrt::math::Vec3{0.0, -1.0, 0.0};
    vectors.N = clrt::math::Vec3{0.0, 1.0, 0.0};
    vectors.CalculateReflectionVector();
    expect(clrt::math::nearlyEqual(vectors.R, clrt::math::Vec3{0.0, -1.0, 0.0}),
           "lighting reflection uses fixed Vec3");
}

void testEmissiveMaterialContributesHdrColor() {
    World world;
    auto sphere = std::make_unique<Sphere>();
    sphere->setAmbient(0.0);
    sphere->setDiffuse(0.0);
    sphere->setSpecular(0.0);
    sphere->setEmissiveColor(Color{0.25, 0.5, 1.0});
    sphere->setEmissiveStrength(4.0);
    world.AddShape(std::move(sphere));

    const Color color = world.Color_at(Ray{
        clrt::math::Point3{0.0, 0.0, -5.0},
        clrt::math::Vec3{0.0, 0.0, 1.0}
    });
    expect(clrt::math::nearlyEqual(color, Color{1.0, 2.0, 4.0}),
           "emissive material contributes unclamped HDR color");
}

void testBloomIsAppliedDuringPpmConversion() {
    Canvas canvas{3, 1};
    canvas.writePixel(1, 0, Color{1.2, 0.0, 0.0});
    canvas.bloomEnabled = true;
    canvas.bloomIntensity = 0.3;
    canvas.bloomThreshold = 1.0;
    canvas.bloomRadius = 1;

    const std::string ppm = canvas.convertToPpm();
    expect(ppm.find("46 0 0 255 0 0 46 0 0") != std::string::npos,
           "PPM conversion applies bloom to neighboring pixels before clamping");
}

} // namespace

int main() {
    testTupleAdapters();
    testMatrixAdapters();
    testMigratedRay();
    testMigratedCamera();
    testMigratedShapeTransforms();
    testMigratedPlane();
    testMigratedTriangle();
    testFixedHitComputations();
    testFixedPatternAndLightVectors();
    testEmissiveMaterialContributesHdrColor();
    testBloomIsAppliedDuringPpmConversion();

    if (failures != 0) {
        std::cerr << failures << " migration test(s) failed\n";
        return 1;
    }

    std::cout << "All math migration tests passed\n";
    return 0;
}
