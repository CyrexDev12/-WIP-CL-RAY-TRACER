#include "core/math/Mat4.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace clrt::math {

Mat4 Mat4::translation(Scalar x, Scalar y, Scalar z) noexcept {
    Mat4 result;
    result(0, 3) = x;
    result(1, 3) = y;
    result(2, 3) = z;
    return result;
}

Mat4 Mat4::scaling(Scalar x, Scalar y, Scalar z) noexcept {
    Mat4 result;
    result(0, 0) = x;
    result(1, 1) = y;
    result(2, 2) = z;
    return result;
}

Mat4 Mat4::rotationX(Scalar radians) noexcept {
    Mat4 result;
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    result(1, 1) = cosine;
    result(1, 2) = -sine;
    result(2, 1) = sine;
    result(2, 2) = cosine;
    return result;
}

Mat4 Mat4::rotationY(Scalar radians) noexcept {
    Mat4 result;
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    result(0, 0) = cosine;
    result(0, 2) = sine;
    result(2, 0) = -sine;
    result(2, 2) = cosine;
    return result;
}

Mat4 Mat4::rotationZ(Scalar radians) noexcept {
    Mat4 result;
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    result(0, 0) = cosine;
    result(0, 1) = -sine;
    result(1, 0) = sine;
    result(1, 1) = cosine;
    return result;
}

Mat4 Mat4::shearing(
    Scalar xy,
    Scalar xz,
    Scalar yx,
    Scalar yz,
    Scalar zx,
    Scalar zy
) noexcept {
    Mat4 result;
    result(0, 1) = xy;
    result(0, 2) = xz;
    result(1, 0) = yx;
    result(1, 2) = yz;
    result(2, 0) = zx;
    result(2, 1) = zy;
    return result;
}

Mat4 Mat4::viewTransform(const Point3& from, const Point3& to, const Vec3& up) {
    const Vec3 forward = (to - from).normalized();
    const Vec3 normalizedUp = up.normalized();
    const Vec3 left = cross(forward, normalizedUp);
    const Vec3 trueUp = cross(left, forward);

    const Mat4 orientation({
        left.x, left.y, left.z, 0.0,
        trueUp.x, trueUp.y, trueUp.z, 0.0,
        -forward.x, -forward.y, -forward.z, 0.0,
        0.0, 0.0, 0.0, 1.0,
    });

    return orientation * translation(-from.x, -from.y, -from.z);
}

Mat4 Mat4::transposed() const noexcept {
    Mat4 result;
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            result(row, column) = (*this)(column, row);
        }
    }
    return result;
}

Scalar Mat4::determinant() const noexcept {
    std::array<std::array<Scalar, 4>, 4> working{};
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            working[row][column] = (*this)(row, column);
        }
    }

    Scalar determinantValue = 1.0;
    int sign = 1;

    for (std::size_t pivotColumn = 0; pivotColumn < 4; ++pivotColumn) {
        std::size_t pivotRow = pivotColumn;
        for (std::size_t row = pivotColumn + 1; row < 4; ++row) {
            if (std::abs(working[row][pivotColumn])
                > std::abs(working[pivotRow][pivotColumn])) {
                pivotRow = row;
            }
        }

        if (std::abs(working[pivotRow][pivotColumn]) <= epsilon) {
            return 0.0;
        }

        if (pivotRow != pivotColumn) {
            std::swap(working[pivotRow], working[pivotColumn]);
            sign = -sign;
        }

        const Scalar pivot = working[pivotColumn][pivotColumn];
        determinantValue *= pivot;

        for (std::size_t row = pivotColumn + 1; row < 4; ++row) {
            const Scalar factor = working[row][pivotColumn] / pivot;
            for (std::size_t column = pivotColumn + 1; column < 4; ++column) {
                working[row][column] -= factor * working[pivotColumn][column];
            }
        }
    }

    return determinantValue * static_cast<Scalar>(sign);
}

Mat4 Mat4::inverse() const {
    std::array<std::array<Scalar, 8>, 4> augmented{};

    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            augmented[row][column] = (*this)(row, column);
            augmented[row][column + 4] = row == column ? 1.0 : 0.0;
        }
    }

    for (std::size_t pivotColumn = 0; pivotColumn < 4; ++pivotColumn) {
        std::size_t pivotRow = pivotColumn;
        for (std::size_t row = pivotColumn + 1; row < 4; ++row) {
            if (std::abs(augmented[row][pivotColumn])
                > std::abs(augmented[pivotRow][pivotColumn])) {
                pivotRow = row;
            }
        }

        if (std::abs(augmented[pivotRow][pivotColumn]) <= epsilon) {
            throw std::domain_error("Cannot invert a singular matrix");
        }

        if (pivotRow != pivotColumn) {
            std::swap(augmented[pivotRow], augmented[pivotColumn]);
        }

        const Scalar pivot = augmented[pivotColumn][pivotColumn];
        for (Scalar& value : augmented[pivotColumn]) {
            value /= pivot;
        }

        for (std::size_t row = 0; row < 4; ++row) {
            if (row == pivotColumn) {
                continue;
            }

            const Scalar factor = augmented[row][pivotColumn];
            for (std::size_t column = 0; column < 8; ++column) {
                augmented[row][column] -= factor * augmented[pivotColumn][column];
            }
        }
    }

    Storage inverseValues{};
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            inverseValues[row * 4 + column] = augmented[row][column + 4];
        }
    }
    return Mat4(inverseValues);
}

Point3 Mat4::transformPoint(const Point3& point) const {
    const Scalar x = (*this)(0, 0) * point.x + (*this)(0, 1) * point.y
        + (*this)(0, 2) * point.z + (*this)(0, 3);
    const Scalar y = (*this)(1, 0) * point.x + (*this)(1, 1) * point.y
        + (*this)(1, 2) * point.z + (*this)(1, 3);
    const Scalar z = (*this)(2, 0) * point.x + (*this)(2, 1) * point.y
        + (*this)(2, 2) * point.z + (*this)(2, 3);
    const Scalar w = (*this)(3, 0) * point.x + (*this)(3, 1) * point.y
        + (*this)(3, 2) * point.z + (*this)(3, 3);

    if (std::abs(w) <= epsilon) {
        throw std::domain_error("Point transform produced a zero homogeneous component");
    }
    if (nearlyEqual(w, 1.0)) {
        return {x, y, z};
    }
    return {x / w, y / w, z / w};
}

Vec3 Mat4::transformVector(const Vec3& vector) const noexcept {
    return {
        (*this)(0, 0) * vector.x + (*this)(0, 1) * vector.y
            + (*this)(0, 2) * vector.z,
        (*this)(1, 0) * vector.x + (*this)(1, 1) * vector.y
            + (*this)(1, 2) * vector.z,
        (*this)(2, 0) * vector.x + (*this)(2, 1) * vector.y
            + (*this)(2, 2) * vector.z,
    };
}

Mat4 operator*(const Mat4& lhs, const Mat4& rhs) noexcept {
    Mat4 result(Mat4::Storage{});
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            for (std::size_t index = 0; index < 4; ++index) {
                result(row, column) += lhs(row, index) * rhs(index, column);
            }
        }
    }
    return result;
}

bool nearlyEqual(const Mat4& lhs, const Mat4& rhs, Scalar tolerance) noexcept {
    for (std::size_t index = 0; index < 16; ++index) {
        if (!nearlyEqual(lhs.storage()[index], rhs.storage()[index], tolerance)) {
            return false;
        }
    }
    return true;
}

} // namespace clrt::math
