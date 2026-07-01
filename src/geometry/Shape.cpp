#include "Shape.h"
#include "Math/Operations.h"

std::vector<double> Shape::world_to_object(const std::vector<double>& point) const {
  if (parent != nullptr) {
    std::vector<double> parentPoint = parent->world_to_object(point);
    return this->getTransform().inverse().multiplyTuple(parentPoint);
  } else {
    return this->getTransform().inverse().multiplyTuple(point);
  }
}


std::vector<double> Shape::normal_to_world(const std::vector<double>& normal) const {
    vector<double> worldNormal = this->getTransform().inverse().transpose().multiplyTuple(normal);
    worldNormal[3] = 0.0; // Ensure w=0 for normal

    if (parent != nullptr) {
        worldNormal = parent->normal_to_world(worldNormal);
    }

    // Normalize the resulting normal vector
    worldNormal = NormalizeTuple(worldNormal);
    return worldNormal;
}
