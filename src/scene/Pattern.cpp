#include "scene/pattern.h"
#include "geometry/Shape.h"
#include <cmath> 
#include <random>
#include <algorithm>
#include <stdexcept>

// Step 1: Convert World Point to Object Space
Color Pattern::PatternAtShape(const Shape* shape, const std::vector<double>& world_point) {
    if (!shape) return Color{0, 0, 0}; 
    std::vector<double> object_point = shape->world_to_object(world_point);
    return PatternAtPoint(object_point);
}

// Step 2: Convert Object Space to Pattern Local Space
Color Pattern::PatternAtPoint(const std::vector<double>& object_point) {
    std::vector<double> mapped_point = object_point;
    if (mapping == Mapping::Spherical) {
        double radius = std::sqrt(
            object_point[0] * object_point[0]
            + object_point[1] * object_point[1]
            + object_point[2] * object_point[2]
        );
        if (radius > 1e-9) {
            constexpr double pi = 3.14159265358979323846;
            double u = 0.5 + std::atan2(object_point[0], object_point[2]) / (2.0 * pi);
            u -= std::floor(u);
            double normalizedY = std::clamp(object_point[1] / radius, -1.0, 1.0);
            double v = std::acos(normalizedY) / pi;
            mapped_point = {u, v, 0.0, 1.0};
        }
    }
    std::vector<double> pattern_point = this->transform.inverse().multiplyTuple(mapped_point);
    return LocalPatternAt(pattern_point);
}

void Pattern::setMapping(const std::string& value) {
    if (value == "object") {
        mapping = Mapping::Object;
    } else if (value == "spherical") {
        mapping = Mapping::Spherical;
    } else {
        throw std::invalid_argument("Unsupported pattern mapping: " + value);
    }
}

// Stripe Pattern
Color StripePattern::LocalPatternAt(const std::vector<double>& pattern_point)  {
    if (static_cast<int>(std::floor(pattern_point[0])) % 2 == 0) {
        return colorA;
    }
    return colorB;
}

// Checkers Pattern (with surface-fighting offset fix)
Color CheckersPattern::LocalPatternAt(const std::vector<double>& pattern_point) {
    double sum = std::floor(pattern_point[0] + 0.00001) + 
                 std::floor(pattern_point[1] + 0.00001) + 
                 std::floor(pattern_point[2] + 0.00001);
                 
    if (static_cast<int>(sum) % 2 == 0) {
        return colorA;
    }
    return colorB;
}

// Gradient Pattern
Color GradientPattern::LocalPatternAt(const std::vector<double>& pattern_point) {
    Color distance = colorB - colorA;
    double x = pattern_point[0];
    double fraction = (x + 1.0) / 2.0;

    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;

    return colorA + distance * fraction;
}

// Ring Pattern (Corrected 2D Concentric Math using X and Z)
Color RingPattern::LocalPatternAt(const std::vector<double>& pattern_point) {
    double eval = std::floor(std::sqrt(std::pow(pattern_point[0], 2) + std::pow(pattern_point[2], 2))); 

    if (static_cast<int>(eval) % 2 == 0) {
        return colorA; 
    }
    return colorB; 
}

// Pertubed Pattern Constructor (Using modern initialization list to bind base_pattern safely!)
PertubedPattern::PertubedPattern(std::shared_ptr<Pattern> base, double scale, double freq) 
    : base_pattern(base), distortion_scale(scale), noise_frequency(freq) {
    
    unsigned int custom_seed = 42;
    std::mt19937 gen(custom_seed);
    for (int i = 0; i < 256; ++i) p[i] = i;
    std::shuffle(p, p + 256, gen);
    for (int i = 0; i < 256; ++i) p[256 + i] = p[i];
}

// Distort coordinates cleanly without duplicating matrix mathematical logic
Color PertubedPattern::LocalPatternAt(const std::vector<double>& pattern_point) {
    if (!base_pattern) return Color{0, 0, 0}; // Safety guard against nulls

    double px = pattern_point[0] * noise_frequency;
    double py = pattern_point[1] * noise_frequency;
    double pz = pattern_point[2] * noise_frequency;

    // Distort coordinates using 3D Perlin noise spatial shifts
    std::vector<double> perturbed_point = {
        pattern_point[0] + Noise3D(px, py, pz) * distortion_scale,
        pattern_point[1] + Noise3D(px + 11.5, py + 22.3, pz + 33.1) * distortion_scale,
        pattern_point[2] + Noise3D(px + 44.2, py + 55.6, pz + 66.7) * distortion_scale,
        pattern_point[3] // Maintains clean homogeneous layout tracking (1.0)
    }; 

    // Zero code duplication! This automatically transforms the point using the sub-pattern's matrix
    return base_pattern->PatternAtPoint(perturbed_point); 
}

// Perlin 3D Lookup Function
double PertubedPattern::Noise3D(double x, double y, double z) {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    double xf = x - std::floor(x);
    double yf = y - std::floor(y);
    double zf = z - std::floor(z);

    double u = fade(xf); double v = fade(yf); double w = fade(zf);

    int aaa = p[p[p[X] + Y] + Z];     int baa = p[p[p[X+1] + Y] + Z];
    int aba = p[p[p[X] + Y+1] + Z];   int bba = p[p[p[X+1] + Y+1] + Z];
    int aab = p[p[p[X] + Y] + Z+1];   int bab = p[p[p[X+1] + Y] + Z+1];
    int abb = p[p[p[X] + Y+1] + Z+1]; int bbb = p[p[p[X+1] + Y+1] + Z+1];

    auto dot = [](const std::vector<double>& g, double dx, double dy, double dz) {
        return g[0]*dx + g[1]*dy + g[2]*dz;
    };

    double x1 = lerp(u, dot(GRADIENTS[aaa%12], xf, yf, zf),     dot(GRADIENTS[baa%12], xf-1, yf, zf));
    double x2 = lerp(u, dot(GRADIENTS[aba%12], xf, yf-1, zf),   dot(GRADIENTS[bba%12], xf-1, yf-1, zf));
    double y1 = lerp(v, x1, x2);

    double x3 = lerp(u, dot(GRADIENTS[aab%12], xf, yf, zf-1),   dot(GRADIENTS[bab%12], xf-1, yf, zf-1));
    double x4 = lerp(u, dot(GRADIENTS[abb%12], xf, yf-1, zf-1), dot(GRADIENTS[bbb%12], xf-1, yf-1, zf-1));
    double y2 = lerp(v, x3, x4);

    return lerp(w, y1, y2); 
}
