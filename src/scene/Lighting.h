#ifndef LIGHTING_H
#define LIGHTING_H
#include "math/Operations.h"
#include "scene/Material.h"
#include "scene/LightShadeVector.h"
#include "scene/PointLight.h"
#include "scene/Light.h"

class Lighting { 
private: 
    // Hold a reference or pointer to the abstract base class
    const Light& sceneLight; 

public: 
    // Default Constructor 
    Lighting() = default; 
    // Constructor accepts any light type that inherits from Light
    Lighting(const Light& lightSource) : sceneLight(lightSource) {}

    // // Destructor 
    // ~Lighting() {
    //     delete &sceneLight; 
    // }

    /*POTENTIAL BUG FIX*/
    // Lighting does not own sceneLight because it stores it by reference
    // The caller owns the actual light object
    ~Lighting() = default;

    vector<double> getPos() {
        return sceneLight.getPosition(); 
    }

    // Pass the LightShadeVector dynamically by reference
    Color ProcessLighting(const Shape* shape,
                                const Material& mat, 
                           LightShadeVector& lsv, 
                           const std::vector<double>& point, bool in_shadow);

};









#endif