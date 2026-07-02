#include "Group.h" 
#include "Math/Operations.h"

Group::Group() : Shape(), bounds_dirty(true) {}

void Group::add_child(std::shared_ptr<Shape> shape) {
    children.push_back(shape);
    shape->setParent(this); // Let the child know this group is its parent
    bounds_dirty = true;     // Force bounding box recalculation next time it's needed
}

bound Group::world_bounds() const {
    // 1. Get the local bounding box of this group
    bound local = this->local_bounds();
    
    // 2. If the local box is uninitialized/empty, return it immediately
    if (local.min_x > local.max_x) {
        return local;
    }

    // 3. Define the 8 corner points of the local bounding box
    double corners[8][3] = {
        {local.min_x, local.min_y, local.min_z},
        {local.min_x, local.min_y, local.max_z},
        {local.min_x, local.max_y, local.min_z},
        {local.min_x, local.max_y, local.max_z},
        {local.max_x, local.min_y, local.min_z},
        {local.max_x, local.min_y, local.max_z},
        {local.max_x, local.max_y, local.min_z},
        {local.max_x, local.max_y, local.max_z}
    };

    // 4. Create an empty world bounding box (starts infinitely inverted)
    bound world;

    // 5. Get the group's forward transformation matrix
    //    (Replace 'this->getTransform()' with your actual matrix getter function)
    auto transformMatrix = this->getTransform();

    // 6. Transform all 8 corners and expand the world box to fit them
    for (int i = 0; i < 8; ++i) {
        // Construct a 4D point (w = 1.0 for points to allow translation)
        std::vector<double> local_point = {corners[i][0], corners[i][1], corners[i][2], 1.0};
        
        // Multiply matrix by vector (using your Math/Operations setup)
        // Adjust this syntax if your library uses operators like: transformMatrix * local_point
        std::vector<double> world_point = transformMatrix.multiplyTuple(local_point);

        // Add the transformed 3D coordinates to our world bounding box
        world.add_point(world_point[0], world_point[1], world_point[2]);
    }

    return world;
}

// Calculates the group's bounding box by combining the world bounds of all children
bound Group::local_bounds() const {
    if (bounds_dirty) {
        cached_bounds = bound();

        for (const auto& child : children) {
            cached_bounds.add_box(child->parent_space_bounds());
        }

        bounds_dirty = false;
    }

    return cached_bounds;
}

void Group::intersect(Ray ray, Intersections& intersectionsList) {
    // 1. Transform the ray into the group's local space
    Ray localRay = ray.transform(this->getTransform().inverse());

    // 2. Optimization: Check if the ray misses this group's bounding box
    //    If it misses, stop immediately and don't check any children!
    if (!this->local_bounds().intersect(localRay)) {
        return; 
    }

    // 3. If it hits the bounding box, check actual intersections with each child shape
    for (const auto& child : children) {
        child->intersect(localRay, intersectionsList);
    }
}

// When an intersection occurs with a group, the intersection record itself references the intersected child shape. 
// Groups themselves don't have a unique surface, so this should never be directly hit.
std::vector<double> Group::normal_at(const std::vector<double>& worldPoint) const {
    std::vector<double> local_pt = this->world_to_object(worldPoint);
    std::vector<double> local_normal = this->local_normal_at(local_pt); 

    return this->normal_to_world(local_normal);
}

// Stub implementation since a group container has no physical surface structure of its own
std::vector<double> Group::local_normal_at(const std::vector<double>& localPoint) const {
    return {0.0, 0.0, 0.0};
}
