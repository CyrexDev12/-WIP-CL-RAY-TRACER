#include "Group.h" 
#include "math/Operations.h"

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
    const auto& transformMatrix = getTransform();

    // 6. Transform all 8 corners and expand the world box to fit them
    for (int i = 0; i < 8; ++i) {
        const clrt::math::Point3 localPoint{
            corners[i][0], corners[i][1], corners[i][2]
        };
        const clrt::math::Point3 worldPoint = transformMatrix * localPoint;

        // Add the transformed 3D coordinates to our world bounding box
        world.add_point(worldPoint.x, worldPoint.y, worldPoint.z);
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

void Group::intersect(const Ray& ray, Intersections& intersectionsList) {
    // 1. Transform the ray into the group's local space
    const Ray localRay = ray.transform(getInverseTransform());

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
clrt::math::Vec3 Group::normalAt(const clrt::math::Point3& worldPoint) const {
    const clrt::math::Point3 localPoint = worldToObject(worldPoint);
    return normalToWorld(localNormalAt(localPoint));
}

// Stub implementation since a group container has no physical surface structure of its own
clrt::math::Vec3 Group::localNormalAt(const clrt::math::Point3&) const {
    return {0.0, 0.0, 0.0};
}


void ApplyMaterialRecursive(std::shared_ptr<Shape> shape) {
    shape->setMaterialColor(Color{0.2, 0.6, 1.0});
    shape->setAmbient(0.1);
    shape->setDiffuse(0.7);
    shape->setSpecular(0.3);
    shape->setShininess(100);

    Group* group = dynamic_cast<Group*>(shape.get());

    if (group != nullptr) {
        for (auto& child : group->get_children()) {
            ApplyMaterialRecursive(child);
        }
    }
}
