#ifndef GROUP_H
#define GROUP_H

#include "Shape.h"
#include "bound.h" // Ensures the 'bound' structure is visible
#include <vector>
#include <memory>

class Group : public Shape {
    private:

    std::vector<std::shared_ptr<Shape>> children;
    
    // Mutable allows const functions (like local_bounds) to update these caching values
    mutable bound cached_bounds;
    mutable bool bounds_dirty;


public:
    Group();
    
    void add_child(std::shared_ptr<Shape> shape);
    Shape* get_child(size_t index) const { return children.at(index).get(); }
    const std::vector<std::shared_ptr<Shape>>& get_children() const { return children; }
    void intersect(Ray ray, Intersections& intersectionsList) override;
    std::vector<double> normal_at(const std::vector<double>& worldPoint) const override;
    
    // Bounding Box Overrides
    bound world_bounds() const; // Function to compute world bounds based on local bounds and transform
    bound local_bounds() const; 
    std::vector<double> local_normal_at(const std::vector<double>& localPoint) const;


};




#endif
