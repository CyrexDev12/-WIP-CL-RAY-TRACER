#include <iostream>
#include <string>

#include "geometry/Cube.h"
#include "geometry/Cylinder.h"
#include "geometry/Group.h"
#include "geometry/Plane.h"
#include "geometry/Sphere.h"
#include "geometry/Triangle.h"
#include "loaders/SceneLoader.h"
#include "scene/Pattern.h"

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

void testExtendedScene(const std::string& path) {
    World world;
    Camera camera;
    std::string imageFile;
    bool multithreaded = true;
    expect(LoadSceneFromJson(path, camera, world, imageFile, multithreaded),
           "extended scene loads successfully");

    expect(world.lights().size() == 2, "loader retains multiple point lights");
    expect(world.shapes().size() == 5, "loader retains every root object");
    expect(world.objectCount() == 7, "loader registers recursive group children");
    expect(dynamic_cast<const Plane*>(world.shapes()[0].get()) != nullptr,
           "loader creates planes");
    expect(dynamic_cast<const Cube*>(world.shapes()[1].get()) != nullptr,
           "loader creates cubes");
    expect(dynamic_cast<const Cylinder*>(world.shapes()[2].get()) != nullptr,
           "loader creates cylinders");
    expect(dynamic_cast<const Triangle*>(world.shapes()[3].get()) != nullptr,
           "loader creates triangles");

    const auto* group = dynamic_cast<const Group*>(world.shapes()[4].get());
    expect(group != nullptr && group->get_children().size() == 2,
           "loader creates recursive groups");
    if (group != nullptr) {
        expect(dynamic_cast<const Sphere*>(group->get_child(0)) != nullptr,
               "group retains its sphere child");
        expect(dynamic_cast<const Cube*>(group->get_child(1)) != nullptr,
               "group retains its cube child");
        expect(group->get_child(1)->getMaterial().pattern == nullptr,
               "loader treats a null pattern as no pattern");
        expect(dynamic_cast<PertubedPattern*>(
                   group->get_child(0)->getMaterial().pattern.get()) != nullptr,
               "loader creates nested perturbed patterns");
    }

    expect(dynamic_cast<CheckersPattern*>(
               world.shapes()[0]->getMaterial().pattern.get()) != nullptr,
           "loader creates checker patterns");
    expect(dynamic_cast<StripePattern*>(
               world.shapes()[1]->getMaterial().pattern.get()) != nullptr,
           "loader creates stripe patterns");
    expect(dynamic_cast<RingPattern*>(
               world.shapes()[2]->getMaterial().pattern.get()) != nullptr,
           "loader creates ring patterns");
    expect(dynamic_cast<GradientPattern*>(
               world.shapes()[3]->getMaterial().pattern.get()) != nullptr,
           "loader creates gradient patterns");

    const clrt::math::Mat4 expectedCubeTransform =
        clrt::math::Mat4::translation(-2.0, 0.75, 0.0)
        * clrt::math::Mat4::rotationY(0.35)
        * clrt::math::Mat4::scaling(0.75, 0.75, 0.75);
    expect(clrt::math::nearlyEqual(
               world.shapes()[1]->getTransform(), expectedCubeTransform),
           "loader applies scale, XYZ rotation, then translation");
    expect(imageFile == "extended.ppm" && !multithreaded,
           "loader retains extended scene output settings");
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Expected extended scene fixture path\n";
        return 2;
    }
    testExtendedScene(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " extended loader test(s) failed\n";
        return 1;
    }
    std::cout << "All extended scene loader tests passed\n";
    return 0;
}
