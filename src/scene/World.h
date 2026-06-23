#ifndef WORLD_H
#define WORLD_H

#include <vector>  // Added: explicitly include vector
#include "geometry/Shape.h"
#include "scene/Lighting.h"
#include "geometry/Computations.h"

class World {
private: 
    std::vector<Shape*> shapesList; // List holding all shapes
    Lighting* lighting = nullptr;   // Initialized to nullptr to avoid dangling pointers 


public: 
    // Default constructor
    World();
    World(Lighting& ling) {
        lighting = &ling; 
    }

    // DESTRUCTOR: Cleans up heap memory allocated for shapes
    ~World() {
        for (Shape* shape : shapesList) {
            delete shape; // Safely deletes each shape object
        }
        shapesList.clear(); // Empties the vector container

        // Note: We DO NOT delete 'lighting' here because your addLighting() 
        // function takes a reference to an object managed outside this class.
    }

    // Disable Copy Constructor and Copy Assignment to prevent double-deletion crashes
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    void AddShape(Shape* shape) {
        shapesList.push_back(shape);
    }

    void setLighting(Lighting& ling) {
        lighting = &ling; 
    }

    Intersections intersect_world(const Ray& ray); 

    bool is_shadowed(const vector<double>& pt); 

    Color reflected_color(const Computations& comps, int remaining);
    
    Color refracted_color(const Computations& comps, int remaining);

    Color shade_hit(const Computations& comps, int remaining);

    Color Color_at(const Ray& ray, int remaining = 5);
};

#endif
