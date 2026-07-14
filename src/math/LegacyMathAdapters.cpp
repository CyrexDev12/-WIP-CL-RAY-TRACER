#include "math/LegacyMathAdapters.h"

#include <stdexcept>
#include <vector>

namespace clrt::compat {

namespace {

void requireTupleSize(const std::vector<double>& tuple) {
    if (tuple.size() != 4) {
        throw std::invalid_argument("Legacy tuple must contain four components");
    }
}

} // namespace

math::Point3 pointFromLegacyTuple(const std::vector<double>& tuple) {
    requireTupleSize(tuple);
    if (!math::nearlyEqual(tuple[3], 1.0)) {
        throw std::invalid_argument("Legacy point tuple must have w = 1");
    }
    return {tuple[0], tuple[1], tuple[2]};
}

math::Vec3 vectorFromLegacyTuple(const std::vector<double>& tuple) {
    requireTupleSize(tuple);
    if (!math::nearlyEqual(tuple[3], 0.0)) {
        throw std::invalid_argument("Legacy vector tuple must have w = 0");
    }
    return {tuple[0], tuple[1], tuple[2]};
}

std::vector<double> toLegacyTuple(const math::Point3& point) {
    return {point.x, point.y, point.z, 1.0};
}

std::vector<double> toLegacyTuple(const math::Vec3& vector) {
    return {vector.x, vector.y, vector.z, 0.0};
}

math::Mat4 matrixFromLegacy(const Matrix& matrix) {
    if (matrix.getRows() != 4 || matrix.getCols() != 4) {
        throw std::invalid_argument("A shared Mat4 requires a legacy 4x4 matrix");
    }

    math::Mat4::Storage values{};
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            values[static_cast<std::size_t>(row * 4 + column)] = matrix.at(row, column);
        }
    }
    return math::Mat4(values);
}

Matrix matrixToLegacy(const math::Mat4& matrix) {
    const auto& storage = matrix.storage();
    const std::vector<double> values(storage.begin(), storage.end());
    return Matrix(4, 4, values);
}

} // namespace clrt::compat
