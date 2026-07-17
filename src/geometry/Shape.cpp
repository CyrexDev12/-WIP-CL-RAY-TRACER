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


bound Shape::parent_space_bounds() const {
    bound local = this->local_bounds();

    // Empty bounds check
    if (local.min_x > local.max_x) {
        return local;
    }

    bound result;

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

    Matrix transformMatrix = this->getTransform();

    for (int i = 0; i < 8; ++i) {
        std::vector<double> p = {
            corners[i][0],
            corners[i][1],
            corners[i][2],
            1.0
        };

        std::vector<double> transformed = transformMatrix.multiplyTuple(p);

        result.add_point(
            transformed[0],
            transformed[1],
            transformed[2]
        );
    }

    return result;
}

void Shape::setEmissiveColor(const Color& color) {
    material.emissiveColor = color;
}

void Shape::setEmissiveStrength(double strength) {
    if (strength < 0.0 || strength > 20.0) {
        throw std::invalid_argument("Emissive strength must be between 0 and 20!");
    }
    material.emissiveStrength = strength;
}
