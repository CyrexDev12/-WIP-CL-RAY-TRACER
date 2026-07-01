#ifndef BOUND_H
#define BOUND_H

#include <algorithm>
#include <limits>
#include "Ray.h" // Replace with your actual Ray header path

struct bound {
    // Using explicit fields instead of vector prevents heap allocations
    double min_x, min_y, min_z;
    double max_x, max_y, max_z;

    // Default constructor initializes an empty, invalid box.
    // This allows it to automatically resize correctly when you add points to it.
    bound() {
        double inf = std::numeric_limits<double>::infinity();
        min_x = min_y = min_z = inf;
        max_x = max_y = max_z = -inf;
    }

    // Constructor for explicit bounds
    bound(double minX, double minY, double minZ, double maxX, double maxY, double maxZ)
        : min_x(minX), min_y(minY), min_z(minZ), max_x(maxX), max_y(maxY), max_z(maxZ) {}

    // Expands this box to enclose a specific 3D point
    void add_point(double x, double y, double z) {
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        min_z = std::min(min_z, z);
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
        max_z = std::max(max_z, z);
    }

    // Merges another bounding box into this one (crucial for Group shapes)
    void add_box(const bound& other) {
        min_x = std::min(min_x, other.min_x);
        min_y = std::min(min_y, other.min_y);
        min_z = std::min(min_z, other.min_z);
        max_x = std::max(max_x, other.max_x);
        max_y = std::max(max_y, other.max_y);
        max_z = std::max(max_z, other.max_z);
    }

    // Fast AABB ray intersection test (Kay-Kajiya slab method)
    bool intersect(const Ray& ray) const {
        // X slab
        double inv_dir_x = 1.0 / ray.direction[0];
        double t1 = (min_x - ray.origin[0]) * inv_dir_x;
        double t2 = (max_x - ray.origin[0]) * inv_dir_x;
        double tmin = std::min(t1, t2);
        double tmax = std::max(t1, t2);

        // Y slab
        double inv_dir_y = 1.0 / ray.direction[1];
        double t3 = (min_y - ray.origin[1]) * inv_dir_y;
        double t4 = (max_y - ray.origin[1]) * inv_dir_y;
        tmin = std::max(tmin, std::min(t3, t4));
        tmax = std::min(tmax, std::max(t3, t4));

        // Z slab
        double inv_dir_z = 1.0 / ray.direction[2];
        double t5 = (min_z - ray.origin[2]) * inv_dir_z;
        double t6 = (max_z - ray.origin[2]) * inv_dir_z;
        tmin = std::max(tmin, std::min(t5, t6));
        tmax = std::min(tmax, std::max(t5, t6));

        // The ray intersects if the entry point is before the exit point,
        // and the exit point is in front of the camera (positive t).
        return tmax >= std::max(0.0, tmin);
    }

    
};

#endif
