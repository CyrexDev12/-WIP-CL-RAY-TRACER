#include "scene/pattern.h"
#include "geometry/Shape.h"
#include <cmath> 

Color Pattern::PatternAtShape(const Shape* shape, const std::vector<double>& world_point) {
        // 1. Convert World Point to Object Local Space
        std::vector<double> object_point = shape->getTransform().inverse().multiplyTuple(world_point);
        
        // 2. Convert Object Local Point to Pattern Local Space
        std::vector<double> pattern_point = this->transform.inverse().multiplyTuple(object_point);
        
        // 3. Evaluate the actual mathematical formula
        return LocalPatternAt(pattern_point);
    }


    Color StripePattern::LocalPatternAt(const std::vector<double>& pattern_point)  {
        if (static_cast<int>(std::floor(pattern_point[0])) % 2 == 0) {
            return colorA;
        }
        return colorB;
    }


    Color CheckersPattern::LocalPatternAt(const std::vector<double>& pattern_point) {
         double sum = std::floor(pattern_point[0]) + 
                     std::floor(pattern_point[1]) + 
                     std::floor(pattern_point[2]);
                     
        if (static_cast<int>(sum) % 2 == 0) {
            return colorA;
        }
        return colorB;
    }

    Color GradientPattern::LocalPatternAt(const std::vector<double>& pattern_point) {
        Color distance = colorB - colorA;

    double x = pattern_point[0];

    // Map sphere object-space x from [-1, 1] to [0, 1]
    double fraction = (x + 1.0) / 2.0;

    // Clamp so it does not go below 0 or above 1
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;

    return colorA + distance * fraction;

    }


    Color RingPattern::LocalPatternAt(const std::vector<double>& pattern_point) {

        double square = pow(pattern_point[0], 2); 
        double eval = floor(sqrt(square + square)); 

        if (static_cast<int>(eval) % 2 == 0) {
            return colorA; 
        }

        return colorB; 
        
    }