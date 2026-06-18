// Pattern.h
#ifndef PATTERN_H
#define PATTERN_H

#include "math/Matrix.h"
#include "math/Operations.h"
#include <cmath>

// Current Patterns: Stripe, Checkers, Gradient, RingPattern 
// TODO:: Nested patterns, Blended Patterns, Pertubed Patterns, Wave pattern
// NOTE: Checkers cause issues, render distortion 

class Shape; // FORWARD DECLERATION 

class Pattern {
public:
    Matrix transform; 

    Pattern() = default; 
    virtual ~Pattern() = default;

    Color PatternAtShape(const Shape* shape, const std::vector<double>& world_point);
    virtual Color LocalPatternAt(const std::vector<double>& pattern_point) = 0;
};

class StripePattern : public Pattern {
private:
    Color colorA;
    Color colorB;

public:
    StripePattern(Color a, Color b) : colorA(a), colorB(b) {}

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;


};

class CheckersPattern : public Pattern {
private:
    Color colorA;
    Color colorB;

public:
    CheckersPattern(Color a, Color b) : colorA(a), colorB(b) {}

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;


};

// Linear Gradient Pattern 
// Blending function, takes two values and interpolates the values between them. 
class GradientPattern : public Pattern {
private:

    Color colorA; 
    Color colorB; 

    public: 

    GradientPattern(Color a, Color b) : colorA(a), colorB(b) {}

    Color LocalPatternAt(const std::vector<double>& pattern_point) override;


};


// Ring Pattern
// Should extend in both x and y
class RingPattern : public Pattern {
    private: 
    Color colorA; 
    Color colorB; 

    public: 

    RingPattern(Color a, Color b) : colorA(a), colorB(b) {}

     Color LocalPatternAt(const std::vector<double>& pattern_point) override;


};

#endif
