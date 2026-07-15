#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "geometry/Group.h"
#include "geometry/Sphere.h"
#include "scene/Scene.h"
#include "scene/SceneSnapshot.h"

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
    return std::make_shared<const MeshAsset>(
        MeshAssetId{4},
        std::vector<MeshVertex>{
            MeshVertex{{0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}},
            MeshVertex{{1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}},
            MeshVertex{{0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}},
        std::vector<MeshAsset::Index>{0, 1, 2},
        std::vector<std::string>{"Body"},
        std::vector<MeshMaterialSlotRange>{{0, 3, 0}},
        MeshSourceMetadata{"mesh.obj", "C:/scene/mesh.obj", {}, {}});
}

void testHierarchyIsFlattenedIntoLeaves() {
    using namespace clrt::scene;
    Scene scene;

    auto root = std::make_unique<Group>();
    root->setTransform(clrt::math::Mat4::translation(10.0, 0.0, 0.0));
    auto nested = std::make_shared<Group>();
    nested->setTransform(clrt::math::Mat4::rotationY(M_PI / 2.0));
    auto sphere = std::make_shared<Sphere>();
    sphere->setTransform(clrt::math::Mat4::scaling(-2.0, 2.0, 2.0));
    nested->add_child(sphere);
    root->add_child(nested);
    scene.addObject(std::move(root));

    const SceneSnapshot snapshot = buildSceneSnapshot(scene);
    expect(snapshot.objects().size() == 1,
           "snapshot emits renderable leaves but omits group nodes");

    const clrt::math::Mat4 expected =
        clrt::math::Mat4::translation(10.0, 0.0, 0.0)
        * clrt::math::Mat4::rotationY(M_PI / 2.0)
        * clrt::math::Mat4::scaling(-2.0, 2.0, 2.0);
    const FlattenedObject& leaf = snapshot.objects().front();
    expect(leaf.objectId == sphere->getObjectId(),
           "flattened leaf preserves its stable object ID");
    expect(clrt::math::nearlyEqual(leaf.worldTransform, expected),
           "snapshot composes transforms as parent world times local");
    expect(clrt::math::nearlyEqual(
               leaf.inverseTransform * leaf.worldTransform,
               clrt::math::Mat4::identity()),
           "snapshot caches the final inverse transform");
    expect(leaf.reversesOrientation,
           "snapshot records negative-determinant world transforms");
}

void testSnapshotRebuildTracksGroupChangesWithoutMutatingOldSnapshot() {
    using namespace clrt::scene;
    Scene scene;
    auto group = std::make_unique<Group>();
    auto sphere = std::make_shared<Sphere>();
    group->add_child(sphere);
    Group* const groupAddress = group.get();
    groupAddress->setTransform(clrt::math::Mat4::translation(1.0, 0.0, 0.0));
    scene.addObject(std::move(group));

    const SceneSnapshot first = buildSceneSnapshot(scene);
    groupAddress->setTransform(clrt::math::Mat4::translation(5.0, 0.0, 0.0));
    const SceneSnapshot rebuilt = buildSceneSnapshot(scene);

    expect(clrt::math::nearlyEqual(
               first.objects().front().worldTransform,
               clrt::math::Mat4::translation(1.0, 0.0, 0.0)),
           "existing snapshot remains immutable after hierarchy changes");
    expect(clrt::math::nearlyEqual(
               rebuilt.objects().front().worldTransform,
               clrt::math::Mat4::translation(5.0, 0.0, 0.0)),
           "rebuilt snapshot reflects the changed group transform");
}

void testGroupedMeshInstanceUsesSharedAsset() {
    using namespace clrt::scene;
    Scene scene;
    auto group = std::make_unique<Group>();
    group->setTransform(clrt::math::Mat4::translation(3.0, 0.0, 0.0));
    Group* const groupAddress = group.get();
    scene.addObject(std::move(group));

    auto asset = makeAsset();
    const MeshVertex* const vertexStorage = asset->vertices().data();
    scene.addMeshAsset(asset);
    const clrt::math::Mat4 local = clrt::math::Mat4::scaling(2.0, 2.0, 2.0);
    scene.addMeshInstance(
        asset->id(),
        local,
        {{0, MaterialId{8}}},
        groupAddress->getObjectId());

    const SceneSnapshot snapshot = buildSceneSnapshot(scene);
    expect(snapshot.meshInstances().size() == 1,
           "snapshot emits each mesh instance once");
    const FlattenedMeshInstance& mesh = snapshot.meshInstances().front();
    expect(clrt::math::nearlyEqual(
               mesh.worldTransform,
               groupAddress->getTransform() * local),
           "mesh instance receives its parent group's final transform");
    expect(mesh.assetId == asset->id()
               && mesh.materialOverrides.front().materialId == MaterialId{8},
           "flattened mesh retains asset identity and material overrides");
    expect(scene.meshAsset(asset->id()).vertices().data() == vertexStorage,
           "snapshot building never duplicates or mutates asset geometry");
}

} // namespace

int main() {
    testHierarchyIsFlattenedIntoLeaves();
    testSnapshotRebuildTracksGroupChangesWithoutMutatingOldSnapshot();
    testGroupedMeshInstanceUsesSharedAsset();

    if (failures != 0) {
        std::cerr << failures << " scene snapshot test(s) failed\n";
        return 1;
    }

    std::cout << "All scene snapshot tests passed\n";
    return 0;
}
