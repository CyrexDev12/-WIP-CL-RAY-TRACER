#ifndef LIGHTING_H
#define LIGHTING_H
#include "scene/Material.h"
#include "scene/LightShadeVector.h"
#include "scene/Light.h"

class Lighting { 
private: 
    // Non-owning reference. The caller must keep the light alive while this
    // shading helper is in use.
    const Light& sceneLight; 

public: 
    // Constructor accepts any light type that inherits from Light
    explicit Lighting(const Light& lightSource) : sceneLight(lightSource) {}

    ~Lighting() = default;

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
