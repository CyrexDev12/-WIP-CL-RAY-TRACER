#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "loaders/SceneLoader.h"
#include "geometry/Sphere.h"
#include "geometry/Group.h"
#include "scene/Lighting.h"
#include "scene/PointLight.h"
#include "scene/World.h"

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
        return {0.0, 1.0, -1.0};
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

void testWorldOwnsResources() {
    static_assert(!std::is_copy_constructible_v<World>);
    static_assert(std::is_move_constructible_v<World>);

    int lightDestructions = 0;
    int shapeDestructions = 0;
    {
        World world(std::make_unique<TrackedLight>(lightDestructions));
        world.AddShape(std::make_unique<TrackedShape>(shapeDestructions));
        expect(world.shapes().size() == 1, "world enumerates its owned shapes");

        World moved = std::move(world);
        expect(moved.shapes().size() == 1, "moving world retains shape ownership");
        expect(lightDestructions == 0, "moving world retains light ownership");
        expect(shapeDestructions == 0, "moving world retains shape lifetime");
    }

    expect(lightDestructions == 1, "world destroys its light exactly once");
    expect(shapeDestructions == 1, "world destroys each shape exactly once");
}

void testLightingDoesNotOwnBorrowedLight() {
    int lightDestructions = 0;
    auto light = std::make_unique<TrackedLight>(lightDestructions);
    {
        Lighting lighting(*light);
        expect(clrt::math::nearlyEqual(
                   lighting.getPos(),
                   clrt::math::Point3{0.0, 1.0, -1.0}),
               "lighting can inspect its borrowed light");
    }

    expect(lightDestructions == 0, "lighting never deletes a borrowed light");
    light.reset();
    expect(lightDestructions == 1, "the actual light owner controls destruction");
}

void testIntersectionsUseStableIds() {
    World world;
    Shape& first = world.AddShape(std::make_unique<Sphere>());
    Shape& second = world.AddShape(std::make_unique<Sphere>());

    expect(first.getObjectId() == clrt::scene::ObjectId{0}
               && second.getObjectId() == clrt::scene::ObjectId{1},
           "world assigns deterministic object IDs in insertion order");
    expect(first.getMaterialId() == clrt::scene::MaterialId{0}
               && second.getMaterialId() == clrt::scene::MaterialId{1},
           "world assigns deterministic material IDs in insertion order");

    const Ray ray{
        clrt::math::Point3{0.0, 0.0, -5.0},
        clrt::math::Vec3{0.0, 0.0, 1.0}};
    const Intersections intersections = world.intersect_world(ray);
    const auto& hits = intersections.getIntersections();
    expect(!hits.empty(), "registered geometry produces intersections");
    for (const Intersection& hit : hits) {
        expect(hit.getObjectId().valid(),
               "every world intersection carries a stable object ID");
    }
}

void testGroupIdsUseDeterministicPreorder() {
    World world;
    auto group = std::make_unique<Group>();
    auto firstChild = std::make_shared<Sphere>();
    auto secondChild = std::make_shared<Sphere>();
    group->add_child(firstChild);
    group->add_child(secondChild);
    Group* groupAddress = group.get();
    world.AddShape(std::move(group));

    expect(groupAddress->getObjectId() == clrt::scene::ObjectId{0},
           "group root receives the first preorder ID");
    expect(firstChild->getObjectId() == clrt::scene::ObjectId{1}
               && secondChild->getObjectId() == clrt::scene::ObjectId{2},
           "group children receive deterministic preorder IDs");
    expect(world.objectCount() == 3 && world.materialCount() == 3,
           "group descendants are registered as objects and materials");

    bool rejectedLateChild = false;
    try {
        groupAddress->add_child(std::make_shared<Sphere>());
    } catch (const std::logic_error&) {
        rejectedLateChild = true;
    }
    expect(rejectedLateChild,
           "registered groups reject mutations that would destabilize IDs");
}

void testLoaderTransfersOwnership(const std::string& validScenePath) {
    int previousShapeDestructions = 0;
    World world;
    world.AddShape(std::make_unique<TrackedShape>(previousShapeDestructions));
    Camera camera;
    std::string imageFile = "unchanged.ppm";
    bool multithreaded = false;

    const bool loaded = LoadSceneFromJson(
        validScenePath,
        camera,
        world,
        imageFile,
        multithreaded);

    expect(loaded, "valid scene loads successfully");
    expect(previousShapeDestructions == 1,
           "successful load releases the world's previous owned shapes");
    expect(world.shapes().size() == 1, "world owns the loaded shape");
    expect(camera.gethSize() == 64.0 && camera.getvSize() == 32.0,
           "successful load commits camera data");
    expect(imageFile == "raii-test.ppm", "successful load commits image data");
    expect(multithreaded, "successful load commits render settings");
    expect(clrt::math::nearlyEqual(
               world.getLight().getPosition(),
               clrt::math::Point3{-4.0, 6.0, -8.0}),
           "world owns the light created by the loader");
}

void testLoaderFailureIsTransactional(const std::string& invalidScenePath) {
    int shapeDestructions = 0;
    World world;
    Shape& originalShape = world.AddShape(
        std::make_unique<TrackedShape>(shapeDestructions));
    Camera camera{12.0, 6.0, M_PI / 3.0};
    std::string imageFile = "original.ppm";
    std::string loadError;
    bool multithreaded = true;

    const bool loaded = LoadSceneFromJson(
        invalidScenePath,
        camera,
        world,
        imageFile,
        multithreaded,
        &loadError);

    expect(!loaded, "invalid scene reports failure");
    expect(!loadError.empty(), "invalid scene reports a useful diagnostic");
    expect(shapeDestructions == 0,
           "failed load preserves the world's existing owned shape");
    expect(world.shapes().size() == 1
               && world.shapes().front().get() == &originalShape,
           "failed load leaves the world unchanged");
    expect(camera.gethSize() == 12.0 && camera.getvSize() == 6.0,
           "failed load leaves the camera unchanged");
    expect(imageFile == "original.ppm", "failed load leaves image data unchanged");
    expect(multithreaded, "failed load leaves render settings unchanged");
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Expected valid and invalid fixture paths\n";
        return 2;
    }

    testWorldOwnsResources();
    testLightingDoesNotOwnBorrowedLight();
    testIntersectionsUseStableIds();
    testGroupIdsUseDeterministicPreorder();
    testLoaderTransfersOwnership(argv[1]);
    testLoaderFailureIsTransactional(argv[2]);

    if (failures != 0) {
        std::cerr << failures << " world ownership test(s) failed\n";
        return 1;
    }

    std::cout << "All World ownership tests passed\n";
    return 0;
}
