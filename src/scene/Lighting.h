#ifndef LIGHTING_H
#define LIGHTING_H
#include "scene/Material.h"
#include "scene/LightShadeVector.h"
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

    // Destructor 
    ~Lighting() {
        delete &sceneLight; 
    }

    clrt::math::Point3 getPos() const {
        return sceneLight.getPosition(); 
    }

    // Pass the LightShadeVector dynamically by reference
    Color ProcessLighting(const Shape* shape,
                                const Material& mat, 
                           LightShadeVector& lsv, 
                           const clrt::math::Point3& point, bool in_shadow);

};









#endif
