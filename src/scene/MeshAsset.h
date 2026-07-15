#ifndef CLRT_SCENE_MESH_ASSET_H
#define CLRT_SCENE_MESH_ASSET_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "core/math/Point3.h"
#include "core/math/Vec3.h"
#include "scene/StableIds.h"

namespace clrt::scene {

struct MeshVertex {
    clrt::math::Point3 position;
    clrt::math::Vec3 normal;
    std::array<clrt::math::Scalar, 2> texCoord{0.0, 0.0};
    bool hasTexCoord{false};
};

struct MeshBounds {
    clrt::math::Point3 minimum;
    clrt::math::Point3 maximum;
};

struct MeshMaterialSlotRange {
    std::uint32_t firstIndex{0};
    std::uint32_t indexCount{0};
    std::uint32_t materialSlot{0};
};

struct MeshSourceMetadata {
    std::string originalPath;
    std::string normalizedPath;
    std::vector<std::string> objectNames;
    std::vector<std::string> groupNames;
};

// Immutable, backend-neutral indexed geometry. Construction validates the complete
// record, after which only const access is exposed.
class MeshAsset {
public:
    using Index = std::uint32_t;

    MeshAsset(
        MeshAssetId id,
        std::vector<MeshVertex> vertices,
        std::vector<Index> indices,
        std::vector<std::string> materialSlots,
        std::vector<MeshMaterialSlotRange> materialSlotRanges,
        MeshSourceMetadata source);

    MeshAsset(const MeshAsset&) = default;
    MeshAsset(MeshAsset&&) noexcept = default;
    MeshAsset& operator=(const MeshAsset&) = delete;
    MeshAsset& operator=(MeshAsset&&) = delete;

    [[nodiscard]] MeshAssetId id() const noexcept { return id_; }
    [[nodiscard]] const std::vector<MeshVertex>& vertices() const noexcept { return vertices_; }
    [[nodiscard]] const std::vector<Index>& indices() const noexcept { return indices_; }
    [[nodiscard]] const MeshBounds& bounds() const noexcept { return bounds_; }
    [[nodiscard]] const std::vector<std::string>& materialSlots() const noexcept {
        return materialSlots_;
    }
    [[nodiscard]] const std::vector<MeshMaterialSlotRange>& materialSlotRanges() const noexcept {
        return materialSlotRanges_;
    }
    [[nodiscard]] const MeshSourceMetadata& source() const noexcept { return source_; }
    [[nodiscard]] std::size_t triangleCount() const noexcept { return indices_.size() / 3; }

private:
    MeshAssetId id_;
    std::vector<MeshVertex> vertices_;
    std::vector<Index> indices_;
    MeshBounds bounds_;
    std::vector<std::string> materialSlots_;
    std::vector<MeshMaterialSlotRange> materialSlotRanges_;
    MeshSourceMetadata source_;
};

} // namespace clrt::scene

#endif
