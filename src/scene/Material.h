#ifndef MATERIAL_H
#define MATERIAL_H
#include "math/Operations.h"
#include "scene/StableIds.h"
#include <memory>

// MATERIAL STRUCT 
// INCLUDE AMBIENT, DIFFUSE, SPECULAR, SHININESS
// Reflectance values remain between 0-1. Emissive strength is an HDR multiplier
// and may exceed 1; emissive color remains a normalized linear RGB color.

class Pattern; // Forward decleration 

struct Material {
    clrt::scene::MaterialId id;
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

     Material()
         : ambient(0.1),
           diffuse(0.9),
           specular(0.9),
           shininess(200.0),
           reflective(0.0),
           transparency(0.0),
           refractiveIndex(0.0),
           emissiveStrength(0.0),
           color(1.0, 1.0, 1.0),
           emissiveColor(0.0, 0.0, 0.0) {}

};




#endif
