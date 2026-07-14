#ifndef CLRT_CORE_MATH_VEC3_H
#define CLRT_CORE_MATH_VEC3_H

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "core/math/Scalar.h"

namespace clrt::math {

struct Vec3 {
    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};

    constexpr Vec3() noexcept = default;
    constexpr Vec3(Scalar xValue, Scalar yValue, Scalar zValue) noexcept
        : x(xValue), y(yValue), z(zValue) {}

    [[nodiscard]] constexpr Scalar& operator[](std::size_t index) noexcept {
        return index == 0 ? x : (index == 1 ? y : z);
    }

    [[nodiscard]] constexpr Scalar operator[](std::size_t index) const noexcept {
        return index == 0 ? x : (index == 1 ? y : z);
    }

    [[nodiscard]] constexpr Vec3 operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec3 operator-() const noexcept { return {-x, -y, -z}; }

    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vec3& operator-=(const Vec3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vec3& operator*=(Scalar scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vec3& operator/=(Scalar scalar) noexcept {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] Scalar length() const noexcept {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] Vec3 normalized() const {
        const Scalar magnitude = length();
        if (magnitude <= epsilon) {
            throw std::domain_error("Cannot normalize a zero-length vector");
        }
        return {x / magnitude, y / magnitude, z / magnitude};
    }
};

[[nodiscard]] constexpr Vec3 operator+(Vec3 lhs, const Vec3& rhs) noexcept {
    return lhs += rhs;
}

[[nodiscard]] constexpr Vec3 operator-(Vec3 lhs, const Vec3& rhs) noexcept {
    return lhs -= rhs;
}

[[nodiscard]] constexpr Vec3 operator*(Vec3 vector, Scalar scalar) noexcept {
    return vector *= scalar;
}

[[nodiscard]] constexpr Vec3 operator*(Scalar scalar, Vec3 vector) noexcept {
    return vector *= scalar;
}

[[nodiscard]] constexpr Vec3 operator/(Vec3 vector, Scalar scalar) noexcept {
    return vector /= scalar;
}

[[nodiscard]] constexpr Scalar dot(const Vec3& lhs, const Vec3& rhs) noexcept {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

[[nodiscard]] constexpr Vec3 cross(const Vec3& lhs, const Vec3& rhs) noexcept {
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

[[nodiscard]] inline bool nearlyEqual(
    const Vec3& lhs,
    const Vec3& rhs,
    Scalar tolerance = epsilon
) noexcept {
    return nearlyEqual(lhs.x, rhs.x, tolerance)
        && nearlyEqual(lhs.y, rhs.y, tolerance)
        && nearlyEqual(lhs.z, rhs.z, tolerance);
}

[[nodiscard]] constexpr Vec3 reflect(
    const Vec3& incoming,
    const Vec3& normal
) noexcept {
    return incoming - normal * (2.0 * dot(incoming, normal));
}

} // namespace clrt::math

#endif
