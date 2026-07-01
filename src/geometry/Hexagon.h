#ifndef HEXAGON_H
#define HEXAGON_H

#include "Group.h"
#include <memory>

// Forward declarations of your primitive shapes
class Cylinder;
class Sphere;

// Function to generate the master hexagon group
std::shared_ptr<Group> create_hexagon();

// Helper functions to construct individual components
std::shared_ptr<Shape> create_hexagon_corner();
std::shared_ptr<Shape> create_hexagon_edge();
std::shared_ptr<Group> create_hexagon_side();

#endif // HEXAGON_H
