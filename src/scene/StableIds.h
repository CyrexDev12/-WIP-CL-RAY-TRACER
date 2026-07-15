#ifndef CLRT_SCENE_STABLE_IDS_H
#define CLRT_SCENE_STABLE_IDS_H

#include <cstdint>
#include <limits>
#include <type_traits>

namespace clrt::scene {

template <typename Tag>
class StableId {
public:
    using Value = std::uint32_t;
    static constexpr Value invalidValue = std::numeric_limits<Value>::max();

    constexpr StableId() noexcept = default;
    explicit constexpr StableId(Value value) noexcept : value_(value) {}

    [[nodiscard]] constexpr bool valid() const noexcept {
        return value_ != invalidValue;
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return valid();
    }

    [[nodiscard]] constexpr Value value() const noexcept { return value_; }

    friend constexpr bool operator==(StableId lhs, StableId rhs) noexcept {
        return lhs.value_ == rhs.value_;
    }

    friend constexpr bool operator!=(StableId lhs, StableId rhs) noexcept {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(StableId lhs, StableId rhs) noexcept {
        return lhs.value_ < rhs.value_;
    }

private:
    Value value_{invalidValue};
};

struct ObjectIdTag;
struct MaterialIdTag;
struct MeshAssetIdTag;
struct MeshInstanceIdTag;

using ObjectId = StableId<ObjectIdTag>;
using MaterialId = StableId<MaterialIdTag>;
using MeshAssetId = StableId<MeshAssetIdTag>;
using MeshInstanceId = StableId<MeshInstanceIdTag>;

static_assert(std::is_trivially_copyable_v<ObjectId>);
static_assert(sizeof(ObjectId) == sizeof(std::uint32_t));

} // namespace clrt::scene

#endif
