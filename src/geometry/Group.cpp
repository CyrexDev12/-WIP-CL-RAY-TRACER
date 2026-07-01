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
        cached_bounds = bound(); // Resets to an empty, inverted box
        
        for (const auto& child : children) {
            // 1. Check if the child is another Group object
            auto childGroup = std::dynamic_pointer_cast<Group>(child);
            
            if (childGroup != nullptr) {
                // If it's a group, it knows how to calculate its world bounds
                cached_bounds.add_box(childGroup->world_bounds());
            } 
            else {
                // 2. If it's a regular shape, handle its bounds manually.
                // For a standard ray tracer where basic primitives are centered at (0,0,0) 
                // with a radius/size of 1 unit in local space:
                bound primitiveLocalBounds(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
                
                // Transform those local primitive bounds into the Group's space
                bound primitiveInGroupSpace;
                auto transformMatrix = child->getTransform(); // Forward transform of the primitive

                double corners[8][3] = {
                    {primitiveLocalBounds.min_x, primitiveLocalBounds.min_y, primitiveLocalBounds.min_z},
                    {primitiveLocalBounds.min_x, primitiveLocalBounds.min_y, primitiveLocalBounds.max_z},
                    {primitiveLocalBounds.min_x, primitiveLocalBounds.max_y, primitiveLocalBounds.min_z},
                    {primitiveLocalBounds.min_x, primitiveLocalBounds.max_y, primitiveLocalBounds.max_z},
                    {primitiveLocalBounds.max_x, primitiveLocalBounds.min_y, primitiveLocalBounds.min_z},
                    {primitiveLocalBounds.max_x, primitiveLocalBounds.min_y, primitiveLocalBounds.max_z},
                    {primitiveLocalBounds.max_x, primitiveLocalBounds.max_y, primitiveLocalBounds.min_z},
                    {primitiveLocalBounds.max_x, primitiveLocalBounds.max_y, primitiveLocalBounds.max_z}
                };

                for (int i = 0; i < 8; ++i) {
                    std::vector<double> local_point = {corners[i][0], corners[i][1], corners[i][2], 1.0};
                    std::vector<double> transformed_point = transformMatrix.multiplyTuple(local_point); 
                    primitiveInGroupSpace.add_point(transformed_point[0], transformed_point[1], transformed_point[2]);
                }

                // Add the transformed primitive box to the group's local box
                cached_bounds.add_box(primitiveInGroupSpace);
            }
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
