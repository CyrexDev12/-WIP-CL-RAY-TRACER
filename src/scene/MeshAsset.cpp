#include "scene/MeshAsset.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace clrt::scene {
namespace {

bool finite(const clrt::math::Point3& point) {
    return std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z);
}

bool finite(const clrt::math::Vec3& vector) {
    return std::isfinite(vector.x) && std::isfinite(vector.y) && std::isfinite(vector.z);
}

MeshBounds calculateBounds(const std::vector<MeshVertex>& vertices) {
    if (vertices.empty()) {
        throw std::invalid_argument("MeshAsset requires at least one vertex");
    }

    MeshBounds bounds{vertices.front().position, vertices.front().position};
    for (const MeshVertex& vertex : vertices) {
        if (!finite(vertex.position) || !finite(vertex.normal)) {
            throw std::invalid_argument("MeshAsset vertices must contain finite values");
        }
        const double normalLength = vertex.normal.length();
        if (!std::isfinite(normalLength) || std::fabs(normalLength - 1.0) > 1.0e-6) {
            throw std::invalid_argument("MeshAsset normals must be normalized");
        }
        if (vertex.hasTexCoord
            && (!std::isfinite(vertex.texCoord[0]) || !std::isfinite(vertex.texCoord[1]))) {
            throw std::invalid_argument("MeshAsset texture coordinates must be finite");
        }

        bounds.minimum.x = std::min(bounds.minimum.x, vertex.position.x);
        bounds.minimum.y = std::min(bounds.minimum.y, vertex.position.y);
        bounds.minimum.z = std::min(bounds.minimum.z, vertex.position.z);
        bounds.maximum.x = std::max(bounds.maximum.x, vertex.position.x);
        bounds.maximum.y = std::max(bounds.maximum.y, vertex.position.y);
        bounds.maximum.z = std::max(bounds.maximum.z, vertex.position.z);
    }
    return bounds;
}

} // namespace

MeshAsset::MeshAsset(
    MeshAssetId id,
    std::vector<MeshVertex> vertices,
    std::vector<Index> indices,
    std::vector<std::string> materialSlots,
    std::vector<MeshMaterialSlotRange> materialSlotRanges,
    MeshSourceMetadata source
) : id_(id),
    vertices_(std::move(vertices)),
    indices_(std::move(indices)),
    bounds_(calculateBounds(vertices_)),
    materialSlots_(std::move(materialSlots)),
    materialSlotRanges_(std::move(materialSlotRanges)),
    source_(std::move(source)) {
    if (!id_) {
        throw std::invalid_argument("MeshAsset requires a valid ID");
    }
    if (indices_.empty() || indices_.size() % 3 != 0) {
        throw std::invalid_argument("MeshAsset indices must contain complete triangles");
    }
    if (vertices_.size() > std::numeric_limits<Index>::max()) {
        throw std::length_error("MeshAsset vertex buffer exceeds the index type");
    }
    for (Index index : indices_) {
        if (index >= vertices_.size()) {
            throw std::out_of_range("MeshAsset index is outside the vertex buffer");
        }
    }
    if (source_.normalizedPath.empty()) {
        throw std::invalid_argument("MeshAsset requires a normalized source path");
    }
    for (const std::string& slot : materialSlots_) {
        if (slot.empty()) {
            throw std::invalid_argument("MeshAsset material slot names cannot be empty");
        }
    }

    std::uint64_t previousEnd = 0;
    for (const MeshMaterialSlotRange& range : materialSlotRanges_) {
        const std::uint64_t end =
            static_cast<std::uint64_t>(range.firstIndex) + range.indexCount;
        if (range.indexCount == 0
            || range.firstIndex % 3 != 0
            || range.indexCount % 3 != 0
            || end > indices_.size()) {
            throw std::invalid_argument(
                "MeshAsset material ranges must cover complete in-bounds triangles");
        }
        if (range.firstIndex < previousEnd) {
            throw std::invalid_argument("MeshAsset material ranges must not overlap");
        }
        if (range.materialSlot >= materialSlots_.size()) {
            throw std::out_of_range("MeshAsset material range references an invalid slot");
        }
        previousEnd = end;
    }
}

} // namespace clrt::scene
