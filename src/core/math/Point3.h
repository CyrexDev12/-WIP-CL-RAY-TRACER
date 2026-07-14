#ifndef CLRT_CORE_MATH_POINT3_H
#define CLRT_CORE_MATH_POINT3_H

#include <cstddef>

#include "core/math/Scalar.h"
#include "core/math/Vec3.h"

namespace clrt::math {

struct Point3 {
    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};

    constexpr Point3() noexcept = default;
    constexpr Point3(Scalar xValue, Scalar yValue, Scalar zValue) noexcept
        : x(xValue), y(yValue), z(zValue) {}

    [[nodiscard]] constexpr Scalar& operator[](std::size_t index) noexcept {
        return index == 0 ? x : (index == 1 ? y : z);
    }

    [[nodiscard]] constexpr Scalar operator[](std::size_t index) const noexcept {
        return index == 0 ? x : (index == 1 ? y : z);
    }
};

[[nodiscard]] constexpr Point3 operator+(Point3 point, const Vec3& offset) noexcept {
    return {point.x + offset.x, point.y + offset.y, point.z + offset.z};
}

[[nodiscard]] constexpr Point3 operator+(const Vec3& offset, Point3 point) noexcept {
    return point + offset;
}

[[nodiscard]] constexpr Point3 operator-(Point3 point, const Vec3& offset) noexcept {
    return {point.x - offset.x, point.y - offset.y, point.z - offset.z};
}

[[nodiscard]] constexpr Vec3 operator-(const Point3& lhs, const Point3& rhs) noexcept {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

[[nodiscard]] inline bool nearlyEqual(
    const Point3& lhs,
    const Point3& rhs,
    Scalar tolerance = epsilon
) noexcept {
    return nearlyEqual(lhs.x, rhs.x, tolerance)
        && nearlyEqual(lhs.y, rhs.y, tolerance)
        && nearlyEqual(lhs.z, rhs.z, tolerance);
}

} // namespace clrt::math

#endif
