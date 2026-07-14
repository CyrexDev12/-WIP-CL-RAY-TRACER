#ifndef CLRT_MATH_LEGACYMATHADAPTERS_H
#define CLRT_MATH_LEGACYMATHADAPTERS_H

#include <vector>

#include "core/math/Mat4.h"
#include "core/math/Point3.h"
#include "core/math/Vec3.h"
#include "math/Matrix.h"

namespace clrt::compat {

[[nodiscard]] math::Point3 pointFromLegacyTuple(const std::vector<double>& tuple);
[[nodiscard]] math::Vec3 vectorFromLegacyTuple(const std::vector<double>& tuple);
[[nodiscard]] std::vector<double> toLegacyTuple(const math::Point3& point);
[[nodiscard]] std::vector<double> toLegacyTuple(const math::Vec3& vector);
[[nodiscard]] math::Mat4 matrixFromLegacy(const Matrix& matrix);
[[nodiscard]] Matrix matrixToLegacy(const math::Mat4& matrix);

} // namespace clrt::compat

#endif
