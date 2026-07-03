#ifndef MATERIAL_H
#define MATERIAL_H
#include "math/Operations.h"
#include <memory>

// MATERIAL STRUCT 
// INCLUDE AMBIENT, DIFFUSE, SPECULAR, SHININESS
// MUST BE UNSIGNED NEGATIVE FLOATING POINTS BETWEEN 0-1, SHININESS VALUES 10-200

class Pattern; // Forward decleration 

struct Material {
    double ambient; 
    double diffuse; 
    double specular; 
    double shininess; 
    double reflective; 
    double transparency;
    double refractiveIndex;
    double emissiveStrength; 

    Color color;
    Color emissiveColor; 
   shared_ptr<Pattern> pattern{nullptr}; // Upgrade from raw pointer to shared 

     Material() : ambient(0.1), diffuse(0.9), specular(0.9), shininess(200.0), reflective(0.0), transparency(0.0), refractiveIndex(0.0), color(1.0, 1.0, 1.0),  emissiveStrength(0) {}

};




#endif