#ifndef PATTERN_H
#define PATTERN_H

#include "core/math/Color.h"
#include "core/math/Mat4.h"
#include "core/math/Point3.h"
#include "math/Matrix.h"
#include <cmath>
#include <memory>
#include <vector>

// Forward declaration of Shape class to avoid circular dependencies
class Shape; 

// =========================================================================
// 1. BASE PATTERN CLASS
// =========================================================================
class Pattern {
private:
    clrt::math::Mat4 transform;
    clrt::math::Mat4 inverseTransform;

public:
    Pattern() = default; 
    virtual ~Pattern() = default;

    void setTransform(const clrt::math::Mat4& matrix);
    void setTransform(const Matrix& matrix);
    [[nodiscard]] const clrt::math::Mat4& getTransform() const noexcept { return transform; }
    [[nodiscard]] const clrt::math::Mat4& getInverseTransform() const noexcept { return inverseTransform; }

    // Handles World-Space to Object-Space transformation
    clrt::math::Color PatternAtShape(const Shape* shape, const clrt::math::Point3& worldPoint);
    clrt::math::Color PatternAtShape(const Shape* shape, const std::vector<double>& worldPoint);
    
    // Handles Object-Space to Pattern-Space transformation (Enables safe nesting!)
    clrt::math::Color PatternAtPoint(const clrt::math::Point3& objectPoint);
    clrt::math::Color PatternAtPoint(const std::vector<double>& objectPoint);
    
    // Pure mathematical formula evaluated locally by derived classes
    virtual clrt::math::Color LocalPatternAt(const clrt::math::Point3& patternPoint) = 0;
};

// =========================================================================
// 2. STRIPE PATTERN
// =========================================================================
class StripePattern : public Pattern {
private:
    clrt::math::Color colorA;
    clrt::math::Color colorB;

public:
    StripePattern(clrt::math::Color a, clrt::math::Color b) : colorA(a), colorB(b) {}

    clrt::math::Color LocalPatternAt(const clrt::math::Point3& patternPoint) override;
};

// =========================================================================
// 3. CHECKERS PATTERN
// =========================================================================
class CheckersPattern : public Pattern {
private:
    clrt::math::Color colorA;
    clrt::math::Color colorB;

public:
    CheckersPattern(clrt::math::Color a, clrt::math::Color b) : colorA(a), colorB(b) {}

    clrt::math::Color LocalPatternAt(const clrt::math::Point3& patternPoint) override;
};

// =========================================================================
// 4. GRADIENT PATTERN
// =========================================================================
class GradientPattern : public Pattern {
private:
    clrt::math::Color colorA;
    clrt::math::Color colorB;

public: 
    GradientPattern(clrt::math::Color a, clrt::math::Color b) : colorA(a), colorB(b) {}

    clrt::math::Color LocalPatternAt(const clrt::math::Point3& patternPoint) override;
};

// =========================================================================
// 5. RING PATTERN
// =========================================================================
class RingPattern : public Pattern {
private: 
    clrt::math::Color colorA;
    clrt::math::Color colorB;

public: 
    RingPattern(clrt::math::Color a, clrt::math::Color b) : colorA(a), colorB(b) {}

    clrt::math::Color LocalPatternAt(const clrt::math::Point3& patternPoint) override;
};

// =========================================================================
// 6. PERTURBED PATTERN (3D Perlin Noise Decorator)
// =========================================================================
class PertubedPattern : public Pattern {
private: 
    int p[512]; // Fixed-size array allocation instead of uninitialized tracking
    std::shared_ptr<Pattern> base_pattern; 
    double distortion_scale; 
    double noise_frequency; 

    // Constant 3D unit gradient vectors for noise interpolation calculations
    const std::vector<std::vector<double>> GRADIENTS = {
        {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
        {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
        {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
    };

    // Quintic polynomial fade curve: 6t^5 - 15t^4 + 10t^3
    double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    
    // Standard Linear Interpolation
    double lerp(double t, double a, double b) { return a + t * (b - a); }
    
    // Internal Perlin 3D noise sample logic
    double Noise3D(double x, double y, double z);

public: 
    // Constructor requires an underlying pattern to wrap with noise spatial shifts
    PertubedPattern(std::shared_ptr<Pattern> base, double scale = 0.2, double freq = 2.0);

    clrt::math::Color LocalPatternAt(const clrt::math::Point3& patternPoint) override;
};

#endif // PATTERN_H
