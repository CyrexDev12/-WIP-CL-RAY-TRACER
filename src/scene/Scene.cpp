#include "scene/Scene.h"
#include "geometry/Group.h"

#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace clrt::scene {

namespace {

void inspectUnregisteredTree(
    const Shape& shape,
    std::unordered_set<const Shape*>& visited
) {
    if (!visited.insert(&shape).second) {
        throw std::logic_error("Shape trees cannot contain cycles or repeated objects");
    }
    if (shape.getObjectId() || shape.getMaterialId()) {
        throw std::logic_error("Shape already belongs to an object registry");
    }

    const auto* group = dynamic_cast<const Group*>(&shape);
    if (group != nullptr) {
        for (const auto& child : group->get_children()) {
            inspectUnregisteredTree(*child, visited);
        }
    }
}

} // namespace

SceneObject::SceneObject(std::unique_ptr<Shape> shape)
    : shape_(std::move(shape)) {
    if (!shape_) {
        throw std::invalid_argument("Scene objects cannot own a null shape");
    }
}

Shape& SceneObject::shape() noexcept {
    return *shape_;
}

const Shape& SceneObject::shape() const noexcept {
    return *shape_;
}

const clrt::math::Mat4& SceneObject::transform() const noexcept {
    return shape_->getTransform();
}

const Material& SceneObject::material() const noexcept {
    return shape_->getMaterial();
}

ObjectId SceneObject::id() const noexcept {
    return shape_->getObjectId();
}

MaterialId SceneObject::materialId() const noexcept {
    return shape_->getMaterialId();
}

Camera& Scene::addCamera(Camera camera) {
    cameras_.push_back(std::move(camera));
    return cameras_.back();
}

Light& Scene::addLight(std::unique_ptr<Light> light) {
    if (!light) {
        throw std::invalid_argument("Scene lights cannot be null");
    }

    lights_.push_back(std::move(light));
    return *lights_.back();
}

SceneObject& Scene::addObject(std::unique_ptr<Shape> shape) {
    if (!shape) {
        throw std::invalid_argument("Scene objects cannot own a null shape");
    }

    std::unordered_set<const Shape*> shapeTree;
    inspectUnregisteredTree(*shape, shapeTree);
    const std::size_t additionalObjects = shapeTree.size();
    if (additionalObjects > ObjectId::invalidValue - objectsById_.size()
        || additionalObjects > MaterialId::invalidValue - materialsById_.size()) {
        throw std::length_error("Scene stable ID space is exhausted");
    }
    objectsById_.reserve(objectsById_.size() + additionalObjects);
    materialsById_.reserve(materialsById_.size() + additionalObjects);
    objects_.emplace_back(std::move(shape));
    registerShapeTree(objects_.back().shape());
    return objects_.back();
}

const MeshAsset& Scene::addMeshAsset(std::shared_ptr<const MeshAsset> asset) {
    if (!asset) {
        throw std::invalid_argument("Scene mesh assets cannot be null");
    }
    if (meshAssetLookup_.find(asset->id().value()) != meshAssetLookup_.end()) {
        throw std::logic_error("Scene already contains this mesh asset ID");
    }

    const std::size_t index = meshAssets_.size();
    meshAssets_.push_back(std::move(asset));
    try {
        meshAssetLookup_.emplace(meshAssets_.back()->id().value(), index);
    } catch (...) {
        meshAssets_.pop_back();
        throw;
    }
    return *meshAssets_.back();
}

MeshInstance& Scene::addMeshInstance(
    MeshAssetId assetId,
    const clrt::math::Mat4& transform,
    std::vector<MeshMaterialOverride> materialOverrides,
    std::optional<ObjectId> parentGroupId
) {
    const MeshAsset& asset = meshAsset(assetId);
    for (const MeshMaterialOverride& overrideEntry : materialOverrides) {
        if (overrideEntry.materialSlot >= asset.materialSlots().size()) {
            throw std::out_of_range(
                "Mesh material override references an invalid asset slot");
        }
    }
    if (meshInstances_.size() >= MeshInstanceId::invalidValue) {
        throw std::length_error("Scene mesh instance ID space is exhausted");
    }
    if (parentGroupId) {
        const Shape& parent = resolve(*parentGroupId);
        if (dynamic_cast<const Group*>(&parent) == nullptr) {
            throw std::invalid_argument(
                "Mesh instance parents must reference registered groups");
        }
    }

    meshInstances_.emplace_back(
        MeshInstanceId{static_cast<MeshInstanceId::Value>(meshInstances_.size())},
        assetId,
        transform,
        std::move(materialOverrides),
        parentGroupId);
    return meshInstances_.back();
}

void Scene::registerShapeTree(Shape& shape) {
    shape.assignStableIds(
        ObjectId{static_cast<ObjectId::Value>(objectsById_.size())},
        MaterialId{static_cast<MaterialId::Value>(materialsById_.size())});
    objectsById_.push_back(&shape);
    materialsById_.push_back(&shape.getMaterial());

    auto* group = dynamic_cast<Group*>(&shape);
    if (group != nullptr) {
        for (const auto& child : group->get_children()) {
            registerShapeTree(*child);
        }
    }
}

const Scene::CameraCollection& Scene::cameras() const noexcept {
    return cameras_;
}

const Scene::LightCollection& Scene::lights() const noexcept {
    return lights_;
}

const Scene::ObjectCollection& Scene::objects() const noexcept {
    return objects_;
}

const Scene::MeshAssetCollection& Scene::meshAssets() const noexcept {
    return meshAssets_;
}

const Scene::MeshInstanceCollection& Scene::meshInstances() const noexcept {
    return meshInstances_;
}

Camera& Scene::camera(std::size_t index) {
    return cameras_.at(index);
}

const Camera& Scene::camera(std::size_t index) const {
    return cameras_.at(index);
}

Light& Scene::light(std::size_t index) {
    return *lights_.at(index);
}

const Light& Scene::light(std::size_t index) const {
    return *lights_.at(index);
}

SceneObject& Scene::object(std::size_t index) {
    return objects_.at(index);
}

const SceneObject& Scene::object(std::size_t index) const {
    return objects_.at(index);
}

const MeshAsset& Scene::meshAsset(MeshAssetId id) const {
    if (!id) {
        throw std::out_of_range("Cannot resolve an invalid mesh asset ID");
    }
    const auto found = meshAssetLookup_.find(id.value());
    if (found == meshAssetLookup_.end()) {
        throw std::out_of_range("Scene does not contain the requested mesh asset ID");
    }
    return *meshAssets_.at(found->second);
}

MeshInstance& Scene::meshInstance(MeshInstanceId id) {
    if (!id) {
        throw std::out_of_range("Cannot resolve an invalid mesh instance ID");
    }
    return meshInstances_.at(id.value());
}

const MeshInstance& Scene::meshInstance(MeshInstanceId id) const {
    if (!id) {
        throw std::out_of_range("Cannot resolve an invalid mesh instance ID");
    }
    return meshInstances_.at(id.value());
}

const Shape& Scene::resolve(ObjectId id) const {
    if (!id) {
        throw std::out_of_range("Cannot resolve an invalid object ID");
    }
    return *objectsById_.at(id.value());
}

const Material& Scene::material(MaterialId id) const {
    if (!id) {
        throw std::out_of_range("Cannot resolve an invalid material ID");
    }
    return *materialsById_.at(id.value());
}

std::size_t Scene::registeredObjectCount() const noexcept {
    return objectsById_.size();
}

std::size_t Scene::registeredMaterialCount() const noexcept {
    return materialsById_.size();
}

} // namespace clrt::scene
