#include "scene/MeshInstance.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace clrt::scene {

MeshInstance::MeshInstance(
    MeshInstanceId id,
    MeshAssetId assetId,
    const clrt::math::Mat4& transform,
    std::vector<MeshMaterialOverride> materialOverrides,
    std::optional<ObjectId> parentGroupId
) : id_(id),
    assetId_(assetId),
    materialOverrides_(std::move(materialOverrides)),
    parentGroupId_(parentGroupId) {
    if (!id_) {
        throw std::invalid_argument("MeshInstance requires a valid instance ID");
    }
    if (!assetId_) {
        throw std::invalid_argument("MeshInstance requires a valid asset ID");
    }
    if (parentGroupId_ && !*parentGroupId_) {
        throw std::invalid_argument("MeshInstance parent group ID must be valid");
    }

    std::sort(
        materialOverrides_.begin(),
        materialOverrides_.end(),
        [](const MeshMaterialOverride& lhs, const MeshMaterialOverride& rhs) {
            return lhs.materialSlot < rhs.materialSlot;
        });
    for (std::size_t index = 0; index < materialOverrides_.size(); ++index) {
        if (!materialOverrides_[index].materialId) {
            throw std::invalid_argument("Mesh material overrides require valid material IDs");
        }
        if (index != 0
            && materialOverrides_[index - 1].materialSlot
                == materialOverrides_[index].materialSlot) {
            throw std::invalid_argument("Mesh material slots can only be overridden once");
        }
    }

    setTransform(transform);
}

std::optional<MaterialId> MeshInstance::materialOverride(
    std::uint32_t materialSlot
) const noexcept {
    const auto found = std::lower_bound(
        materialOverrides_.begin(),
        materialOverrides_.end(),
        materialSlot,
        [](const MeshMaterialOverride& entry, std::uint32_t slot) {
            return entry.materialSlot < slot;
        });
    if (found == materialOverrides_.end() || found->materialSlot != materialSlot) {
        return std::nullopt;
    }
    return found->materialId;
}

void MeshInstance::setTransform(const clrt::math::Mat4& transform) {
    const clrt::math::Mat4 inverse = transform.inverse();
    transform_ = transform;
    inverseTransform_ = inverse;
    inverseTranspose_ = inverse.transposed();
}

} // namespace clrt::scene
