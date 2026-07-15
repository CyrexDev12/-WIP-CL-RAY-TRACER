#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "scene/Scene.h"

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

std::shared_ptr<const clrt::scene::MeshAsset> makeAsset() {
    using namespace clrt::scene;
    std::vector<MeshVertex> vertices{
        MeshVertex{{-1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0}, true},
        MeshVertex{{1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}, true},
        MeshVertex{{0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {0.5, 1.0}, true}};
    return std::make_shared<const MeshAsset>(
        MeshAssetId{7},
        std::move(vertices),
        std::vector<MeshAsset::Index>{0, 1, 2},
        std::vector<std::string>{"Body", "Trim"},
        std::vector<MeshMaterialSlotRange>{{0, 3, 0}},
        MeshSourceMetadata{
            "models/fixture.obj",
            "C:/scene/models/fixture.obj",
            {"Fixture"},
            {"Main"}});
}

void testInstancesShareAssetGeometry() {
    using namespace clrt::scene;
    Scene scene;
    auto asset = makeAsset();
    const MeshVertex* const vertexStorage = asset->vertices().data();
    const MeshAsset* const assetAddress = asset.get();
    scene.addMeshAsset(asset);

    const clrt::math::Mat4 firstTransform =
        clrt::math::Mat4::translation(-2.0, 0.0, 0.0);
    const clrt::math::Mat4 secondTransform =
        clrt::math::Mat4::translation(2.0, 0.0, 0.0)
        * clrt::math::Mat4::scaling(2.0, 1.0, 0.5);

    MeshInstance& first = scene.addMeshInstance(
        asset->id(), firstTransform, {{1, MaterialId{4}}});
    const MeshInstanceId firstId = first.id();
    MeshInstance& second = scene.addMeshInstance(asset->id(), secondTransform);

    expect(firstId == MeshInstanceId{0} && second.id() == MeshInstanceId{1},
           "scene assigns deterministic mesh instance IDs");
    expect(scene.meshInstances().size() == 2 && scene.meshAssets().size() == 1,
           "instances and assets are stored in separate collections");
    expect(scene.meshInstance(firstId).assetId() == MeshAssetId{7}
               && second.assetId() == MeshAssetId{7},
           "multiple instances reference the same asset ID");
    expect(&scene.meshAsset(MeshAssetId{7}) == assetAddress,
           "scene retains the immutable asset allocation");
    expect(scene.meshAsset(MeshAssetId{7}).vertices().data() == vertexStorage,
           "instances do not duplicate vertex storage");
    expect(clrt::math::nearlyEqual(
               second.inverseTransform() * second.transform(),
               clrt::math::Mat4::identity()),
           "mesh instance caches its inverse transform");
    expect(scene.meshInstance(firstId).materialOverride(1) == MaterialId{4},
           "mesh instance retains slot-specific material overrides");
    expect(!second.materialOverride(0).has_value(),
           "mesh instance can use asset material slots without overrides");
}

void testInstanceValidation() {
    using namespace clrt::scene;
    Scene scene;
    auto asset = makeAsset();
    scene.addMeshAsset(asset);

    bool rejectedUnknownAsset = false;
    try {
        scene.addMeshInstance(MeshAssetId{99});
    } catch (const std::out_of_range&) {
        rejectedUnknownAsset = true;
    }
    expect(rejectedUnknownAsset, "scene rejects instances of unknown assets");

    bool rejectedInvalidSlot = false;
    try {
        scene.addMeshInstance(asset->id(), {}, {{2, MaterialId{1}}});
    } catch (const std::out_of_range&) {
        rejectedInvalidSlot = true;
    }
    expect(rejectedInvalidSlot, "scene rejects overrides outside asset material slots");

    bool rejectedDuplicateSlot = false;
    try {
        scene.addMeshInstance(
            asset->id(), {}, {{0, MaterialId{1}}, {0, MaterialId{2}}});
    } catch (const std::invalid_argument&) {
        rejectedDuplicateSlot = true;
    }
    expect(rejectedDuplicateSlot, "instance rejects duplicate overrides for one slot");

    bool rejectedDuplicateAsset = false;
    try {
        scene.addMeshAsset(asset);
    } catch (const std::logic_error&) {
        rejectedDuplicateAsset = true;
    }
    expect(rejectedDuplicateAsset, "scene rejects duplicate mesh asset IDs");

    bool rejectedSingularTransform = false;
    try {
        scene.addMeshInstance(
            asset->id(), clrt::math::Mat4::scaling(1.0, 0.0, 1.0));
    } catch (const std::domain_error&) {
        rejectedSingularTransform = true;
    }
    expect(rejectedSingularTransform, "instance rejects non-invertible transforms");
}

} // namespace

int main() {
    testInstancesShareAssetGeometry();
    testInstanceValidation();

    if (failures != 0) {
        std::cerr << failures << " mesh instance test(s) failed\n";
        return 1;
    }

    std::cout << "All mesh instance tests passed\n";
    return 0;
}
