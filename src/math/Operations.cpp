#include <iostream>
#include <vector> 
#include <cmath>
#include <limits> 
#include <algorithm>
#include <stdexcept>   // for std::invalid_argument
#include "math/Operations.h"
#include <cstdint>
using namespace std; 

// Tunables 
constexpr double kabsTol  = 1e-9;  // Absolute tolerance for a very small magnitude 
constexpr double KrelTol  = 1e-8;  // Relative tolerance scales with value
constexpr double kTEps    = 1e-7;  // "hit" distance threshold (t > kTEps)
constexpr double kRayBias = 1e-4;  // Ray origin offset factor 

inline bool nearlyEqual(double a, double b,
                        double relTol = KrelTol,
                        double absTol = kabsTol) {
    if (a == b) return true; // Handles infinities and exact matches 

    // If either is not finite, treat as not nearly equal
    if (!isfinite(a) || !isfinite(b)) {
        return false;
    }

    const double diff  = fabs(a - b);
    const double scale = max(fabs(a), fabs(b));

    return diff <= max(absTol, relTol * scale);
}

// ----------------------------------------------------------------------------- //
// Tuple = (x, y, z, w)
// A tuple with w == 1 is a point, and a tuple with w == 0 is a vector 
// ----------------------------------------------------------------------------- //

// Matches: bool isTuple(const vector<double> &point);
bool isTuple(const vector<double> &point) {
    // For this assignment, let's define a "tuple" as a 4D (x,y,z,w) vector.
    return point.size() == 4;
}

vector<double> AddTuples(const vector<double>& a, const vector<double>& b) {
        if (a.size() != b.size()) {
            throw invalid_argument("SubtractTuples: vectors must have same size");
        }

        vector<double> result; 
        result.reserve(a.size()); 

        for (size_t i = 0; i < a.size(); i++) {
            result.push_back(a[i] + b[i]);
        }

        return result; 


}

// Matches: vector<double> SubtractTuples(const vector<double>& x2, const vector<double>& x1);
vector<double> SubtractTuples(const vector<double>& x2, const vector<double>& x1) {
    if (x2.size() != x1.size() || x1.size() != 4) {
        throw invalid_argument("SubtractTuples: vectors must have same size");
    }

    vector<double> result;
    result.reserve(x1.size());

    for (size_t i = 0; i < x1.size(); ++i) {
        result.push_back(x2[i] - x1[i]);
    }

    return result;
}

// Matches: vector<double> NegateTuple(const vector<double>& x);
vector<double> NegateTuple(const vector<double>& x) {
    vector<double> result;
    result.reserve(x.size());

    for (double v : x) {
        result.push_back(-v);   // 0 - v
    }

    return result;
}

// Matches: vector<double> ScaleTuple(const vector<double>& x, double& scalar);
// Remove the '&' symbol from scalar
// Added const for saftey and flexibility 
vector<double> ScaleTuple(const vector<double>& x, double scalar) {
    vector<double> result;
    result.reserve(x.size());

    for (size_t i = 0; i < x.size(); ++i) {
        result.push_back(x[i] * scalar);
    }

    return result; 
}

// Matches: double GetMagnitude(const vector<double>& x);
double GetMagnitude(const vector<double>& x) {
    double sum = 0.0;

    for (size_t i = 0; i < x.size(); ++i) {
        sum += x[i] * x[i];
    }

    return sqrt(sum);
}

// Matches: void PrintTuple(const vector<double>& x);
void PrintTuple(const vector<double>& x) {
    cout << "{";
    for (size_t i = 0; i < x.size(); ++i) {
        cout << x[i];
        if (i + 1 < x.size()) {
            cout << ", ";
        }
    }
    cout << "}\n";
}

vector<double> NormalizeTuple(const vector<double>& x) {
    // If it's a standard 3D vector, normalize normally
    if (x.size() == 3) {
        double mag = GetMagnitude(x);
        if (nearlyEqual(mag, 0.0)) throw invalid_argument("Cannot normalize zero-length vector");
        
        double invMag = 1.0 / mag;
        return { x[0] * invMag, x[1] * invMag, x[2] * invMag };
    }

    // If it's a 4D homogeneous coordinate
    if (x.size() == 4) {
        // If w == 0, it's a directional vector. Normalize x, y, z and keep w = 0.
        if (nearlyEqual(x[3], 0.0)) {
            double mag = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
            if (nearlyEqual(mag, 0.0)) throw invalid_argument("Cannot normalize zero-length vector");
            
            double invMag = 1.0 / mag;
            return { x[0] * invMag, x[1] * invMag, x[2] * invMag, 0.0 };
        } 
        
        // If w != 0, it's a point. Project to 3D space by dividing by w.
        double invW = 1.0 / x[3];
        return { x[0] * invW, x[1] * invW, x[2] * invW, 1.0 };
    }

    throw invalid_argument("Unsupported vector size");
}


// Matches: bool TuplesEqual(const vector<double> &tup1, const vector<double> &tup2);
bool TuplesEqual(const vector<double> &tup1, const vector<double> &tup2) {
    if (tup1.size() != tup2.size()) return false;

    for (size_t i = 0; i < tup1.size(); ++i) {
        if (!nearlyEqual(tup1[i], tup2[i])) {
            return false; 
        }
    }
    return true;
}
// BUG FIX: It incorrectly was multiplying and adding the w products together. 
// Status: Fixed 
double CalculateDotProd(const vector<double>& x1, const vector<double>& x2) {
    if (x1.size() != x2.size() || x1.size() < 3) {
        throw std::invalid_argument("Vectors must be the same length for dot product.");
    }
   

    return x1[0] * x2[0] + x1[1] * x2[1] + x1[2] * x2[2]; // Only consider x, y, z components for dot product
}



vector<double> CrossProduct(const vector<double>& a, const vector<double>& b) {
    if (a.size() != 4 || b.size() != 4) {
        throw invalid_argument("CrossProduct requires 4D vectors (x,y,z,w).");
    }

    // Use only x, y, z components
    double cx = a[1] * b[2] - a[2] * b[1];
    double cy = a[2] * b[0] - a[0] * b[2];
    double cz = a[0] * b[1] - a[1] * b[0];

    // w = 0 → result is a direction vector
    return {cx, cy, cz, 0.0};
}


// Creates a floating-point color from a vector
Color makeColor(const std::vector<double>& rgb) {
    if (rgb.size() < 3) {
        std::cerr << "makeColor: rgb vector has fewer than 3 components\n";
        return Color{0.0, 0.0, 0.0};
    }
    return Color{rgb[0], rgb[1], rgb[2]};
}

Color addColors(const Color& c1, const Color& c2) {
    return Color{c1.r + c2.r, c1.g + c2.g, c1.b + c2.b};
}

Color subtractColors(const Color& c1, const Color& c2) {
    return Color{c1.r - c2.r, c1.g - c2.g, c1.b - c2.b};
}

Color multiplyColors(const Color& c1, const Color& c2) {
    return Color{c1.r * c2.r, c1.g * c2.g, c1.b * c2.b};
}

Color multiplyByScalar(const Color& c1, double scalar) {
    return Color{c1.r * scalar, c1.g * scalar, c1.b * scalar};
}

Color hadamard_product(const Color& c1, const Color& c2) {
    // Component-wise multiplication (identical to multiplyColors)
    return Color{c1.r * c2.r, c1.g * c2.g, c1.b * c2.b};
}

void PrintColor(const std::string& label, const Color& c) {
    std::cout << label << ": {" << c.r << ", " << c.g << ", " << c.b << "}\n";
}

double toRadians(double degrees) {
    const double PI = 3.14159265358979323846;
    return degrees * (/*M_PI*/ PI / 180.0);
}


bool isPoint(const vector<double> &point) {
    if (point.size() != 4){
        throw invalid_argument("isPoint: input must be a 4D tuple");
    }

    return isTuple(point) && nearlyEqual(point[3], 1.0);
}

bool isVector(const vector<double> &point) {
    if (point.size() != 4){
        throw invalid_argument("isVector: input must be a 4D tuple");
    }
    return isTuple(point) && nearlyEqual(point[3], 0.0);
}

// Params normal v -> surface normal
// invec -> incoming vector 
vector<double> reflect(const vector<double>& normalv, const vector<double>& invec) {
    double dotProd = CalculateDotProd(normalv, invec); 
    return invec - 2 * (dotProd) * normalv; 
}

