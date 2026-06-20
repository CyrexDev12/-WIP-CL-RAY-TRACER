#ifndef OPERATIONS_H
#define OPERATIONS_H
#include <vector>
#include <iostream>
#include <cstdint>
using namespace std; 

struct Color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    Color() = default; 
    Color(double giveR, double giveG, double giveB) : r(giveR), g(giveG), b(giveB) {}
};




bool isTuple(const vector<double> &point);
bool isPoint(const vector<double> &point);
bool isVector(const vector<double> &point);
bool TuplesEqual(const vector<double> &tup1, const vector<double> &tup2);
vector<double> AddTuples(const vector<double>& a, const vector<double>& b);
vector<double> SubtractTuples(const vector<double>& x2, const vector<double>& x1);
vector<double> NegateTuple(const vector<double>& x); 
vector<double> ScaleTuple(const vector<double>& x, double scalar); 
vector<double> reflect(const vector<double>& normalv, const vector<double>& invec); 

inline vector<double> operator-(const vector<double>& x2, const vector<double>& x1) {
    return SubtractTuples(x2, x1);
}

inline vector<double> operator+(const vector<double>& x2, const vector<double>& x1) {
    return AddTuples(x2, x1); 
}

inline vector<double> operator*(const vector<double>& x, double scalar) {
   return ScaleTuple(x, scalar); 
}

inline std::vector<double> operator*(double scalar, const std::vector<double>& x) {
    return x * scalar; // Reuses your existing operator*
}

double GetMagnitude(const vector<double>& x); 
void PrintTuple(const vector<double>& x); 
vector<double> NormalizeTuple(const vector<double>& x);
double CalculateDotProd(const vector<double>& x1, const vector<double>& x2);
vector<double> CrossProduct(const vector<double>& a, const vector<double>& b);
double toRadians(double degrees);

// Color 

// ============================================================================
// BASIC RGBA COLORS REFERENCE
// ============================================================================
// Format: { Red, Green, Blue, Alpha/W }
// Range:  0.0 (completely off) to 1.0 (maximum intensity)
//
// Black:   { 0.0, 0.0, 0.0, 1.0 }
// White:   { 1.0, 1.0, 1.0, 1.0 }
// Red:     { 1.0, 0.0, 0.0, 1.0 }
// Green:   { 0.0, 1.0, 0.0, 1.0 }
// Blue:    { 0.0, 0.0, 1.0, 1.0 }
// Yellow:  { 1.0, 1.0, 0.0, 1.0 }
// Cyan:    { 0.0, 1.0, 1.0, 1.0 }
// Magenta: { 1.0, 0.0, 1.0, 1.0 }
// Gray:    { 0.5, 0.5, 0.5, 1.0 }
// ============================================================================

Color addColors(const Color& c1, const Color& c2);
Color subtractColors(const Color& c1, const Color& c2);
Color multiplyColors(const Color& c1, const Color& c2);
Color multiplyByScalar(const Color& c1, double scalar);
// Color hadamard_product(const Color& c1, const Color& c2); Dont need, same as multiply colors
void PrintColor(const std::string& label, const Color& c);

// Operators for conveinance 
inline Color operator+(const Color&c1, const Color& c2) {
    return addColors(c1, c2);
}

inline Color operator-(const Color&c1, const Color& c2) {
    return subtractColors(c1, c2); 
}

inline Color operator*(const Color&c1, const Color& c2) {
    return multiplyColors(c1, c2); 
}

inline Color operator*(const Color&c1, double scalar) {
    return multiplyByScalar(c1, scalar); 
}


#endif