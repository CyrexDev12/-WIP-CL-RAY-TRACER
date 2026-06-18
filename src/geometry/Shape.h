// Shape.h
#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include <stdexcept>
#include "geometry/Ray.h"
#include "scene/Material.h" 
#include "math/Matrix.h" 
#include "geometry/Intersection.h"
#include <memory>

// 1. FORWARD DECLARATION 
class Pattern; 

class Shape {
protected:
    std::vector<double> position;
    Matrix transformMatrix; 
    Material material; 

public:
    virtual ~Shape() = default;

    virtual void intersect(Ray ray, Intersections& intersectionsList) = 0;
    virtual std::vector<double> normal_at(const std::vector<double>& worldPoint) const = 0; // Fixed missing std::

    // --- Common Getters & Setters ---
    Matrix getTransform() const { return transformMatrix; }
    void setTransform(const Matrix& m) { this->transformMatrix = m; } 

    std::vector<double> getPosition() const { return position; }
    void setPosition(const std::vector<double>& pos) { this->position = pos; }

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

    // Pass pattern by reference, stores its address cleanly
    void setMaterialPattern(shared_ptr<Pattern> pattern) {
        material.pattern = pattern; 
    }
};

#endif
