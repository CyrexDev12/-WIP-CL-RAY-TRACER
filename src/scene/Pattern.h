#ifndef PATTERN_H
#define PATTERN_H

#include "math/Matrix.h"
#include "math/Operations.h"
#include <cmath>
#include <memory>
#include <vector>

// Forward declaration of Shape class to avoid circular dependencies
class Shape; 

// =========================================================================
// 1. BASE PATTERN CLASS
// =========================================================================
class Pattern {
public:
    Matrix transform; 

    Pattern() = default; 
    virtual ~Pattern() = default;

    // Handles World-Space to Object-Space transformation
    Color PatternAtShape(const Shape* shape, const std::vector<double>& world_point);
    
    // Handles Object-Space to Pattern-Space transformation (Enables safe nesting!)
    Color PatternAtPoint(const std::vector<double>& object_point); 
    
    // Pure mathematical formula evaluated locally by derived classes
    virtual Color LocalPatternAt(const std::vector<double>& pattern_point) = 0;
};

// =========================================================================
// 2. STRIPE PATTERN
// =========================================================================
class StripePattern : public Pattern {
private:
    Color colorA;
    Color colorB;

public:
    StripePattern(Color a, Color b) : colorA(a), colorB(b) {}

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;
};

// =========================================================================
// 3. CHECKERS PATTERN
// =========================================================================
class CheckersPattern : public Pattern {
private:
    Color colorA;
    Color colorB;

public:
    CheckersPattern(Color a, Color b) : colorA(a), colorB(b) {}

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;
};

// =========================================================================
// 4. GRADIENT PATTERN
// =========================================================================
class GradientPattern : public Pattern {
private:
    Color colorA; 
    Color colorB; 

public: 
    GradientPattern(Color a, Color b) : colorA(a), colorB(b) {}

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;
};

// =========================================================================
// 5. RING PATTERN
// =========================================================================
class RingPattern : public Pattern {
private: 
    Color colorA; 
    Color colorB; 

public: 
    RingPattern(Color a, Color b) : colorA(a), colorB(b) {}

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;
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

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;
};

#endif // PATTERN_H
