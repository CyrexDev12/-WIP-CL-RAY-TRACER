#ifndef CLRT_SCENE_SCENE_H
#define CLRT_SCENE_SCENE_H

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include "geometry/Shape.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "scene/MeshAsset.h"
#include "scene/MeshInstance.h"
#include "scene/ObjectResolver.h"

namespace clrt::scene {

// Owns one renderable shape and exposes the data shared render backends need to
// enumerate. Transform and material storage remains on Shape during the legacy
// geometry migration, so this wrapper never creates divergent copies.
class SceneObject {
public:
    explicit SceneObject(std::unique_ptr<Shape> shape);

    SceneObject(SceneObject&&) noexcept = default;
    SceneObject& operator=(SceneObject&&) noexcept = default;
    SceneObject(const SceneObject&) = delete;
    SceneObject& operator=(const SceneObject&) = delete;

    [[nodiscard]] Shape& shape() noexcept;
    [[nodiscard]] const Shape& shape() const noexcept;
    [[nodiscard]] const clrt::math::Mat4& transform() const noexcept;
    [[nodiscard]] const Material& material() const noexcept;
    [[nodiscard]] ObjectId id() const noexcept;
    [[nodiscard]] MaterialId materialId() const noexcept;

private:
    std::unique_ptr<Shape> shape_;
};

// Backend-neutral ownership boundary for scene resources. It is intentionally
// move-only: cameras are values, while lights and objects may be polymorphic.
// Collection access is read-only so callers can enumerate without invalidating
// ownership; individual mutable resources are returned by the add/access methods.
class Scene : public ObjectResolver {
public:
    using CameraCollection = std::vector<Camera>;
    using LightCollection = std::vector<std::unique_ptr<Light>>;
    using ObjectCollection = std::vector<SceneObject>;
    using MeshAssetCollection = std::vector<std::shared_ptr<const MeshAsset>>;
    using MeshInstanceCollection = std::vector<MeshInstance>;

    Scene() = default;
    ~Scene() = default;

    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Camera& addCamera(Camera camera);
    Light& addLight(std::unique_ptr<Light> light);
    SceneObject& addObject(std::unique_ptr<Shape> shape);
    const MeshAsset& addMeshAsset(std::shared_ptr<const MeshAsset> asset);
    MeshInstance& addMeshInstance(
        MeshAssetId assetId,
        const clrt::math::Mat4& transform = clrt::math::Mat4::identity(),
        std::vector<MeshMaterialOverride> materialOverrides = {},
        std::optional<ObjectId> parentGroupId = std::nullopt);

    [[nodiscard]] const CameraCollection& cameras() const noexcept;
    [[nodiscard]] const LightCollection& lights() const noexcept;
    [[nodiscard]] const ObjectCollection& objects() const noexcept;
    [[nodiscard]] const MeshAssetCollection& meshAssets() const noexcept;
    [[nodiscard]] const MeshInstanceCollection& meshInstances() const noexcept;

    [[nodiscard]] Camera& camera(std::size_t index);
    [[nodiscard]] const Camera& camera(std::size_t index) const;
    [[nodiscard]] Light& light(std::size_t index);
    [[nodiscard]] const Light& light(std::size_t index) const;
    [[nodiscard]] SceneObject& object(std::size_t index);
    [[nodiscard]] const SceneObject& object(std::size_t index) const;
    [[nodiscard]] const MeshAsset& meshAsset(MeshAssetId id) const;
    [[nodiscard]] MeshInstance& meshInstance(MeshInstanceId id);
    [[nodiscard]] const MeshInstance& meshInstance(MeshInstanceId id) const;
    [[nodiscard]] const Shape& resolve(ObjectId id) const override;
    [[nodiscard]] const Material& material(MaterialId id) const;
    [[nodiscard]] std::size_t registeredObjectCount() const noexcept;
    [[nodiscard]] std::size_t registeredMaterialCount() const noexcept;

private:
    void registerShapeTree(Shape& shape);

    CameraCollection cameras_;
    LightCollection lights_;
    ObjectCollection objects_;
    MeshAssetCollection meshAssets_;
    MeshInstanceCollection meshInstances_;
    std::unordered_map<MeshAssetId::Value, std::size_t> meshAssetLookup_;
    std::vector<const Shape*> objectsById_;
    std::vector<const Material*> materialsById_;
};

} // namespace clrt::scene

#endif
