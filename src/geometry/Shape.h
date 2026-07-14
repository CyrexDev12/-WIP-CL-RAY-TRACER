// Shape.h
#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include <stdexcept>
#include "core/math/Mat4.h"
#include "core/math/Point3.h"
#include "geometry/Ray.h"
#include "scene/Material.h" 
#include "math/Matrix.h" 
#include "geometry/Intersection.h"
#include <memory>
#include "Bound.h" // Include the bound header for bounding box functionality

// 1. FORWARD DECLARATION 
class Pattern; 

class Shape {
protected:
    clrt::math::Point3 position;
    clrt::math::Mat4 transformMatrix;
    clrt::math::Mat4 inverseTransform;
    clrt::math::Mat4 inverseTranspose;
    Material material; 
    Shape* parent{nullptr}; // Pointer to parent shape, default to nullptr

public:
    virtual ~Shape() = default;


    // Virtual functions
    virtual void intersect(const Ray& ray, Intersections& intersectionsList) = 0;
    virtual clrt::math::Vec3 normalAt(const clrt::math::Point3& worldPoint) const = 0;
    virtual bound local_bounds() const = 0;

    // --- Common Getters & Setters ---
    const clrt::math::Mat4& getTransform() const noexcept { return transformMatrix; }
    const clrt::math::Mat4& getInverseTransform() const noexcept { return inverseTransform; }
    const clrt::math::Mat4& getInverseTranspose() const noexcept { return inverseTranspose; }
    void setTransform(const clrt::math::Mat4& matrix);
    void setTransform(const Matrix& matrix);

    clrt::math::Point3 getPosition() const noexcept { return position; }
    void setPosition(const clrt::math::Point3& point) noexcept { position = point; }
    void setPosition(const std::vector<double>& point);

    // NEW: Add const Material& to return value, so we are not returning a new copy every time
    const Material& getMaterial() const { return material; }
    Color getMaterialColor() const { return material.color; }
    void setMaterialColor(const Color& color) { material.color = color; }

    void setAmbient(double num) {
        if (num < 0.0 || num > 1.0) throw std::invalid_argument("Must be a value between 0-1!");
        material.ambient = num; 
    }

    void setDiffuse(double num) {
        if (num < 0.0 || num > 1.0) throw std::invalid_argument("Must be a value between 0-1!");
        material.diffuse = num; 
    }

    void setSpecular(double num) {
        if (num < 0.0 || num > 1.0) throw std::invalid_argument("Must be a value between 0-1!");
        material.specular = num; 
    }

    void setShininess(double num) {
        if (num < 10.0 || num > 200.0) throw std::invalid_argument("Must be a value between 10-200!");
        material.shininess = num; 
    }

    void setReflective(double num) {
    if (num < 0.0 || num > 1.0) {
        throw std::invalid_argument("Reflective must be between 0 and 1!");
    }

    material.reflective = num;
}

void setTransparency(double num) {
    if (num < 0.0 || num > 1.0) {
        throw std::invalid_argument("Transparency must be between 0 and 1!");
    }

    material.transparency = num;
}

void setRefractiveIndex(double num) {
    if (num <= 0.0) {
        throw std::invalid_argument("Refractive index must be greater than 0!");
    }

    material.refractiveIndex = num;
}

    // Pass pattern by reference, stores its address cleanly
    void setMaterialPattern(shared_ptr<Pattern> pattern) {
        material.pattern = pattern; 
    }

    Shape* getParent() const { return parent; }
    void setParent(Shape* p) { parent = p; }

    clrt::math::Point3 worldToObject(const clrt::math::Point3& point) const;
    clrt::math::Vec3 normalToWorld(const clrt::math::Vec3& normal) const;

    // Transitional wrappers for legacy tests and callers.
    std::vector<double> normal_at(const std::vector<double>& worldPoint) const;
    std::vector<double> world_to_object(const std::vector<double>& point) const;
    std::vector<double> normal_to_world(const std::vector<double>& normal) const;

    bound parent_space_bounds() const;

    void setEmissiveColor(const Color& color);
    void setEmissiveStrength(double strength);

};



#endif
