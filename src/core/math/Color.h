#ifndef CLRT_CORE_MATH_COLOR_H
#define CLRT_CORE_MATH_COLOR_H

#include "core/math/Scalar.h"

namespace clrt::math {

struct Color {
    Scalar r{0.0};
    Scalar g{0.0};
    Scalar b{0.0};

    constexpr Color() noexcept = default;
    constexpr Color(Scalar red, Scalar green, Scalar blue) noexcept
        : r(red), g(green), b(blue) {}

    constexpr Color& operator+=(const Color& other) noexcept {
        r += other.r;
        g += other.g;
        b += other.b;
        return *this;
    }

    constexpr Color& operator-=(const Color& other) noexcept {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        return *this;
    }

    constexpr Color& operator*=(Scalar scalar) noexcept {
        r *= scalar;
        g *= scalar;
        b *= scalar;
        return *this;
    }
};

[[nodiscard]] constexpr Color operator+(Color lhs, const Color& rhs) noexcept {
    return lhs += rhs;
}

[[nodiscard]] constexpr Color operator-(Color lhs, const Color& rhs) noexcept {
    return lhs -= rhs;
}

[[nodiscard]] constexpr Color operator*(Color color, Scalar scalar) noexcept {
    return color *= scalar;
}

[[nodiscard]] constexpr Color operator*(Scalar scalar, Color color) noexcept {
    return color *= scalar;
}

[[nodiscard]] constexpr Color operator*(
    const Color& lhs,
    const Color& rhs
) noexcept {
    return {lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b};
}

[[nodiscard]] constexpr Color hadamard(
    const Color& lhs,
    const Color& rhs
) noexcept {
    return {lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b};
}

[[nodiscard]] inline bool nearlyEqual(
    const Color& lhs,
    const Color& rhs,
    Scalar tolerance = epsilon
) noexcept {
    return nearlyEqual(lhs.r, rhs.r, tolerance)
        && nearlyEqual(lhs.g, rhs.g, tolerance)
        && nearlyEqual(lhs.b, rhs.b, tolerance);
}

} // namespace clrt::math

#endif
