#ifndef CLRT_CORE_MATH_MAT4_H
#define CLRT_CORE_MATH_MAT4_H

#include <array>
#include <cstddef>

#include "core/math/Point3.h"
#include "core/math/Scalar.h"
#include "core/math/Vec3.h"

namespace clrt::math {

class Mat4 {
public:
    using Storage = std::array<Scalar, 16>;

    constexpr Mat4() noexcept
        : values_{
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        } {}

    explicit constexpr Mat4(Storage values) noexcept : values_(values) {}

    [[nodiscard]] static constexpr Mat4 identity() noexcept { return Mat4{}; }
    [[nodiscard]] static Mat4 translation(Scalar x, Scalar y, Scalar z) noexcept;
    [[nodiscard]] static Mat4 scaling(Scalar x, Scalar y, Scalar z) noexcept;
    [[nodiscard]] static Mat4 rotationX(Scalar radians) noexcept;
    [[nodiscard]] static Mat4 rotationY(Scalar radians) noexcept;
    [[nodiscard]] static Mat4 rotationZ(Scalar radians) noexcept;
    [[nodiscard]] static Mat4 shearing(
        Scalar xy,
        Scalar xz,
        Scalar yx,
        Scalar yz,
        Scalar zx,
        Scalar zy
    ) noexcept;
    [[nodiscard]] static Mat4 viewTransform(
        const Point3& from,
        const Point3& to,
        const Vec3& up
    );

    [[nodiscard]] constexpr Scalar& operator()(
        std::size_t row,
        std::size_t column
    ) noexcept {
        return values_[row * 4 + column];
    }

    [[nodiscard]] constexpr Scalar operator()(
        std::size_t row,
        std::size_t column
    ) const noexcept {
        return values_[row * 4 + column];
    }

    [[nodiscard]] constexpr const Storage& storage() const noexcept { return values_; }
    [[nodiscard]] constexpr const Scalar* data() const noexcept { return values_.data(); }

    [[nodiscard]] Mat4 transposed() const noexcept;
    [[nodiscard]] Scalar determinant() const noexcept;
    [[nodiscard]] Mat4 inverse() const;
    [[nodiscard]] Point3 transformPoint(const Point3& point) const;
    [[nodiscard]] Vec3 transformVector(const Vec3& vector) const noexcept;

private:
    Storage values_;
};

[[nodiscard]] Mat4 operator*(const Mat4& lhs, const Mat4& rhs) noexcept;

[[nodiscard]] inline Point3 operator*(const Mat4& matrix, const Point3& point) {
    return matrix.transformPoint(point);
}

[[nodiscard]] inline Vec3 operator*(const Mat4& matrix, const Vec3& vector) noexcept {
    return matrix.transformVector(vector);
}

[[nodiscard]] bool nearlyEqual(
    const Mat4& lhs,
    const Mat4& rhs,
    Scalar tolerance = epsilon
) noexcept;

} // namespace clrt::math

#endif
