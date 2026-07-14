#ifndef CLRT_CORE_MATH_SCALAR_H
#define CLRT_CORE_MATH_SCALAR_H

#include <cmath>

namespace clrt::math {

using Scalar = double;

inline constexpr Scalar epsilon = 1.0e-9;

[[nodiscard]] inline bool nearlyEqual(
    Scalar lhs,
    Scalar rhs,
    Scalar tolerance = epsilon
) noexcept {
    return std::abs(lhs - rhs) <= tolerance;
}

} // namespace clrt::math

#endif
