#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "scene/MeshAsset.h"

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    }
}

std::vector<clrt::scene::MeshVertex> triangleVertices() {
    using clrt::scene::MeshVertex;
    return {
        MeshVertex{{-1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0}, true},
        MeshVertex{{1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}, true},
        MeshVertex{{0.0, 2.0, 0.0}, {0.0, 0.0, 1.0}, {0.5, 1.0}, true}
    };
}

clrt::scene::MeshSourceMetadata sourceMetadata() {
    return {
        "models/triangle.obj",
        "C:/scene/models/triangle.obj",
        {"Triangle"},
        {"Fixture"}
    };
}

void testTypedStableIds() {
    using namespace clrt::scene;
    static_assert(sizeof(ObjectId) == sizeof(std::uint32_t));
    static_assert(!std::is_convertible_v<ObjectId, MaterialId>);
    static_assert(!std::is_convertible_v<MeshAssetId, MeshInstanceId>);
    static_assert(std::is_trivially_copyable_v<MeshInstanceId>);

    expect(!ObjectId{}.valid(), "default stable ID is invalid");
    expect(ObjectId{7}.valid() && ObjectId{7}.value() == 7,
           "explicit stable ID retains its value");
}

void testImmutableMeshAsset() {
    using namespace clrt::scene;
    static_assert(!std::is_copy_assignable_v<MeshAsset>);
    static_assert(!std::is_move_assignable_v<MeshAsset>);

    MeshAsset asset{
        MeshAssetId{3},
        triangleVertices(),
        {0, 1, 2},
        {"Paint"},
        {{0, 3, 0}},
        sourceMetadata()};

    expect(asset.id() == MeshAssetId{3}, "mesh retains its stable asset ID");
    expect(asset.triangleCount() == 1, "mesh exposes its triangle count");
    expect(asset.vertices().size() == 3 && asset.indices().size() == 3,
           "mesh exposes unified immutable buffers");
    expect(clrt::math::nearlyEqual(
               asset.bounds().minimum,
               clrt::math::Point3{-1.0, 0.0, 0.0}),
           "mesh computes its minimum bound");
    expect(clrt::math::nearlyEqual(
               asset.bounds().maximum,
               clrt::math::Point3{1.0, 2.0, 0.0}),
           "mesh computes its maximum bound");
    expect(asset.materialSlots().front() == "Paint"
               && asset.materialSlotRanges().front().indexCount == 3,
           "mesh retains material-slot ranges");
    expect(asset.source().objectNames.front() == "Triangle",
           "mesh retains source metadata");
}

void testMeshValidation() {
    using namespace clrt::scene;

    bool rejectedInvalidIndex = false;
    try {
        MeshAsset asset{
            MeshAssetId{0}, triangleVertices(), {0, 1, 4}, {}, {}, sourceMetadata()};
        static_cast<void>(asset);
    } catch (const std::out_of_range&) {
        rejectedInvalidIndex = true;
    }
    expect(rejectedInvalidIndex, "mesh rejects indices outside the vertex buffer");

    bool rejectedPartialTriangleRange = false;
    try {
        MeshAsset asset{
            MeshAssetId{0},
            triangleVertices(),
            {0, 1, 2},
            {"Paint"},
            {{0, 2, 0}},
            sourceMetadata()};
        static_cast<void>(asset);
    } catch (const std::invalid_argument&) {
        rejectedPartialTriangleRange = true;
    }
    expect(rejectedPartialTriangleRange,
           "mesh rejects material ranges that split triangles");

    bool rejectedInvalidNormal = false;
    try {
        auto vertices = triangleVertices();
        vertices.front().normal = {0.0, 0.0, 2.0};
        MeshAsset asset{
            MeshAssetId{0}, std::move(vertices), {0, 1, 2}, {}, {}, sourceMetadata()};
        static_cast<void>(asset);
    } catch (const std::invalid_argument&) {
        rejectedInvalidNormal = true;
    }
    expect(rejectedInvalidNormal, "mesh rejects non-normalized normals");
}

} // namespace

int main() {
    testTypedStableIds();
    testImmutableMeshAsset();
    testMeshValidation();

    if (failures != 0) {
        std::cerr << failures << " mesh asset test(s) failed\n";
        return 1;
    }

    std::cout << "All mesh asset tests passed\n";
    return 0;
}
