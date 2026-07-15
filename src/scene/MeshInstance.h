#ifndef CLRT_SCENE_MESH_INSTANCE_H
#define CLRT_SCENE_MESH_INSTANCE_H

#include <cstdint>
#include <optional>
#include <vector>

#include "core/math/Mat4.h"
#include "scene/StableIds.h"

namespace clrt::scene {

struct MeshMaterialOverride {
    std::uint32_t materialSlot{0};
    MaterialId materialId;
};

// Per-scene placement of an immutable MeshAsset. Instances contain no positions,
// normals, UVs, or indices; all geometry is referenced by MeshAssetId.
class MeshInstance {
public:
    MeshInstance(
        MeshInstanceId id,
        MeshAssetId assetId,
        const clrt::math::Mat4& transform = clrt::math::Mat4::identity(),
        std::vector<MeshMaterialOverride> materialOverrides = {},
        std::optional<ObjectId> parentGroupId = std::nullopt);

    [[nodiscard]] MeshInstanceId id() const noexcept { return id_; }
    [[nodiscard]] MeshAssetId assetId() const noexcept { return assetId_; }
    [[nodiscard]] const clrt::math::Mat4& transform() const noexcept { return transform_; }
    [[nodiscard]] const clrt::math::Mat4& inverseTransform() const noexcept {
        return inverseTransform_;
    }
    [[nodiscard]] const clrt::math::Mat4& inverseTranspose() const noexcept {
        return inverseTranspose_;
    }
    [[nodiscard]] const std::vector<MeshMaterialOverride>& materialOverrides() const noexcept {
        return materialOverrides_;
    }
    [[nodiscard]] std::optional<ObjectId> parentGroupId() const noexcept {
        return parentGroupId_;
    }
    [[nodiscard]] std::optional<MaterialId> materialOverride(
        std::uint32_t materialSlot) const noexcept;

    void setTransform(const clrt::math::Mat4& transform);

private:
    MeshInstanceId id_;
    MeshAssetId assetId_;
    clrt::math::Mat4 transform_;
    clrt::math::Mat4 inverseTransform_;
    clrt::math::Mat4 inverseTranspose_;
    std::vector<MeshMaterialOverride> materialOverrides_;
    std::optional<ObjectId> parentGroupId_;
};

} // namespace clrt::scene

#endif
