#ifndef CLRT_SCENE_SCENE_SNAPSHOT_H
#define CLRT_SCENE_SCENE_SNAPSHOT_H

#include <vector>

#include "core/math/Mat4.h"
#include "scene/MeshInstance.h"
#include "scene/StableIds.h"

namespace clrt::scene {

class Scene;

struct FlattenedObject {
    ObjectId objectId;
    MaterialId materialId;
    clrt::math::Mat4 worldTransform;
    clrt::math::Mat4 inverseTransform;
    clrt::math::Mat4 inverseTranspose;
    bool reversesOrientation{false};
};

struct FlattenedMeshInstance {
    MeshInstanceId instanceId;
    MeshAssetId assetId;
    clrt::math::Mat4 worldTransform;
    clrt::math::Mat4 inverseTransform;
    clrt::math::Mat4 inverseTranspose;
    bool reversesOrientation{false};
    std::vector<MeshMaterialOverride> materialOverrides;
};

// Immutable renderer-facing transform snapshot. Rebuild it after changing any
// source hierarchy or local transform.
class SceneSnapshot {
public:
    SceneSnapshot(
        std::vector<FlattenedObject> objects,
        std::vector<FlattenedMeshInstance> meshInstances);

    [[nodiscard]] const std::vector<FlattenedObject>& objects() const noexcept {
        return objects_;
    }
    [[nodiscard]] const std::vector<FlattenedMeshInstance>& meshInstances() const noexcept {
        return meshInstances_;
    }

private:
    std::vector<FlattenedObject> objects_;
    std::vector<FlattenedMeshInstance> meshInstances_;
};

[[nodiscard]] SceneSnapshot buildSceneSnapshot(const Scene& scene);

} // namespace clrt::scene

#endif
