#ifndef CLRT_CORE_MATH_RAY_H
#define CLRT_CORE_MATH_RAY_H

#include "core/math/Mat4.h"
#include "core/math/Point3.h"
#include "core/math/Scalar.h"
#include "core/math/Vec3.h"

namespace clrt::math {

struct Ray {
    Point3 origin;
    Vec3 direction;

    constexpr Ray() noexcept = default;
    constexpr Ray(Point3 rayOrigin, Vec3 rayDirection) noexcept
        : origin(rayOrigin), direction(rayDirection) {}

    [[nodiscard]] constexpr Point3 position(Scalar distance) const noexcept {
        return origin + direction * distance;
    }

    [[nodiscard]] Ray transformed(const Mat4& transform) const {
        return {transform * origin, transform * direction};
    }
};

} // namespace clrt::math

#endif
