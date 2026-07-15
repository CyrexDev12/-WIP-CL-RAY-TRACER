#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "geometry/Sphere.h"
#include "scene/PointLight.h"
#include "scene/Scene.h"

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

class TrackedLight final : public Light {
public:
    explicit TrackedLight(int& destructionCount)
        : destructionCount_(destructionCount) {}

    ~TrackedLight() override {
        ++destructionCount_;
    }

    clrt::math::Color getIntensity() const override {
        return {1.0, 1.0, 1.0};
    }

    clrt::math::Point3 getPosition() const override {
        return {0.0, 1.0, 0.0};
    }

private:
    int& destructionCount_;
};

class TrackedShape final : public Shape {
public:
    explicit TrackedShape(int& destructionCount)
        : destructionCount_(destructionCount) {}

    ~TrackedShape() override {
        ++destructionCount_;
    }

    void intersect(const Ray&, Intersections&) override {}

    clrt::math::Vec3 normalAt(const clrt::math::Point3&) const override {
        return {0.0, 1.0, 0.0};
    }

    bound local_bounds() const override {
        return {-1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
    }

private:
    int& destructionCount_;
};

void testSceneIsOwnedAndMoveOnly() {
    static_assert(!std::is_copy_constructible_v<clrt::scene::Scene>);
    static_assert(!std::is_copy_assignable_v<clrt::scene::Scene>);
    static_assert(std::is_move_constructible_v<clrt::scene::Scene>);
    static_assert(std::is_move_assignable_v<clrt::scene::Scene>);

    int lightDestructions = 0;
    int shapeDestructions = 0;

    {
        clrt::scene::Scene scene;
        scene.addLight(std::make_unique<TrackedLight>(lightDestructions));
        scene.addObject(std::make_unique<TrackedShape>(shapeDestructions));

        expect(lightDestructions == 0, "scene retains owned light lifetime");
        expect(shapeDestructions == 0, "scene retains owned object lifetime");

        clrt::scene::Scene moved = std::move(scene);
        expect(moved.lights().size() == 1, "moving scene retains lights");
        expect(moved.objects().size() == 1, "moving scene retains objects");
    }

    expect(lightDestructions == 1, "scene destroys each owned light exactly once");
    expect(shapeDestructions == 1, "scene destroys each owned object exactly once");
}

void testResourcesAreEnumerable() {
    clrt::scene::Scene scene;

    Camera camera{320.0, 180.0, M_PI / 3.0};
    const clrt::math::Mat4 cameraTransform =
        clrt::math::Mat4::translation(0.0, -1.0, 5.0);
    camera.setTransform(cameraTransform);
    scene.addCamera(std::move(camera));

    const auto lightPosition = clrt::math::Point3{-10.0, 10.0, -10.0};
    scene.addLight(std::make_unique<PointLight>(
        lightPosition,
        clrt::math::Color{1.0, 0.8, 0.6}));

    auto sphere = std::make_unique<Sphere>();
    const clrt::math::Mat4 objectTransform =
        clrt::math::Mat4::translation(1.0, 2.0, 3.0)
        * clrt::math::Mat4::scaling(2.0, 2.0, 2.0);
    sphere->setTransform(objectTransform);
    sphere->setMaterialColor({0.2, 0.4, 0.6});
    Shape* const sphereAddress = sphere.get();
    scene.addObject(std::move(sphere));

    std::size_t cameraCount = 0;
    for (const Camera& item : scene.cameras()) {
        ++cameraCount;
        expect(clrt::math::nearlyEqual(item.getTransform(), cameraTransform),
               "camera enumeration exposes its transform");
    }

    std::size_t lightCount = 0;
    for (const auto& item : scene.lights()) {
        ++lightCount;
        expect(item != nullptr, "light enumeration never contains null entries");
        expect(clrt::math::nearlyEqual(item->getPosition(), lightPosition),
               "light enumeration exposes light data");
    }

    std::size_t objectCount = 0;
    for (const clrt::scene::SceneObject& item : scene.objects()) {
        ++objectCount;
        expect(&item.shape() == sphereAddress,
               "object enumeration preserves the owned shape identity");
        expect(item.id() == clrt::scene::ObjectId{0},
               "scene assigns a deterministic object ID");
        expect(item.materialId() == clrt::scene::MaterialId{0},
               "scene assigns a deterministic material ID");
        expect(clrt::math::nearlyEqual(item.transform(), objectTransform),
               "object enumeration exposes transforms");
        expect(clrt::math::nearlyEqual(
                   item.material().color,
                   clrt::math::Color{0.2, 0.4, 0.6}),
               "object enumeration exposes materials");
    }

    expect(cameraCount == 1, "all cameras are enumerable");
    expect(lightCount == 1, "all lights are enumerable");
    expect(objectCount == 1, "all objects are enumerable");
    expect(&scene.resolve(clrt::scene::ObjectId{0}) == sphereAddress,
           "scene resolves object IDs without intersection pointers");
    expect(scene.material(clrt::scene::MaterialId{0}).id
               == clrt::scene::MaterialId{0},
           "scene resolves stable material IDs");
}

void testNullResourcesAreRejected() {
    clrt::scene::Scene scene;

    bool rejectedNullLight = false;
    try {
        scene.addLight(nullptr);
    } catch (const std::invalid_argument&) {
        rejectedNullLight = true;
    }
    expect(rejectedNullLight, "null lights are rejected");

    bool rejectedNullObject = false;
    try {
        scene.addObject(nullptr);
    } catch (const std::invalid_argument&) {
        rejectedNullObject = true;
    }
    expect(rejectedNullObject, "null objects are rejected");
}

} // namespace

int main() {
    testSceneIsOwnedAndMoveOnly();
    testResourcesAreEnumerable();
    testNullResourcesAreRejected();

    if (failures != 0) {
        std::cerr << failures << " scene test(s) failed\n";
        return 1;
    }

    std::cout << "All scene ownership tests passed\n";
    return 0;
}
