#include "scene/SceneSnapshot.h"

#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "geometry/Group.h"
#include "scene/Scene.h"

namespace clrt::scene {
namespace {

struct TransformSet {
    clrt::math::Mat4 world;
    clrt::math::Mat4 inverse;
    clrt::math::Mat4 inverseTranspose;
    bool reversesOrientation;
};

TransformSet makeTransformSet(const clrt::math::Mat4& world) {
    const clrt::math::Mat4 inverse = world.inverse();
    return {
        world,
        inverse,
        inverse.transposed(),
        world.determinant() < 0.0};
}

void flattenShape(
    const Shape& shape,
    const clrt::math::Mat4& parentWorld,
    std::vector<FlattenedObject>& objects,
    std::unordered_map<ObjectId::Value, clrt::math::Mat4>& groupWorldTransforms
) {
    const clrt::math::Mat4 world = parentWorld * shape.getTransform();
    const auto* group = dynamic_cast<const Group*>(&shape);
    if (group != nullptr) {
        groupWorldTransforms.emplace(shape.getObjectId().value(), world);
        for (const auto& child : group->get_children()) {
            flattenShape(*child, world, objects, groupWorldTransforms);
        }
        return;
    }

    TransformSet transforms = makeTransformSet(world);
    objects.push_back({
        shape.getObjectId(),
        shape.getMaterialId(),
        transforms.world,
        transforms.inverse,
        transforms.inverseTranspose,
        transforms.reversesOrientation});
}

} // namespace

SceneSnapshot::SceneSnapshot(
    std::vector<FlattenedObject> objects,
    std::vector<FlattenedMeshInstance> meshInstances
) : objects_(std::move(objects)),
    meshInstances_(std::move(meshInstances)) {}

SceneSnapshot buildSceneSnapshot(const Scene& scene) {
    std::vector<FlattenedObject> objects;
    objects.reserve(scene.registeredObjectCount());
    std::unordered_map<ObjectId::Value, clrt::math::Mat4> groupWorldTransforms;

    for (const SceneObject& root : scene.objects()) {
        flattenShape(
            root.shape(),
            clrt::math::Mat4::identity(),
            objects,
            groupWorldTransforms);
    }

    std::vector<FlattenedMeshInstance> meshes;
    meshes.reserve(scene.meshInstances().size());
    for (const MeshInstance& instance : scene.meshInstances()) {
        clrt::math::Mat4 parentWorld = clrt::math::Mat4::identity();
        if (instance.parentGroupId()) {
            const auto found = groupWorldTransforms.find(
                instance.parentGroupId()->value());
            if (found == groupWorldTransforms.end()) {
                throw std::logic_error(
                    "Mesh instance parent group is absent from the scene hierarchy");
            }
            parentWorld = found->second;
        }

        TransformSet transforms = makeTransformSet(
            parentWorld * instance.transform());
        meshes.push_back({
            instance.id(),
            instance.assetId(),
            transforms.world,
            transforms.inverse,
            transforms.inverseTranspose,
            transforms.reversesOrientation,
            instance.materialOverrides()});
    }

    return SceneSnapshot{std::move(objects), std::move(meshes)};
}

} // namespace clrt::scene
