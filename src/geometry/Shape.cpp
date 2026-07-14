#include "Shape.h"
#include "math/Operations.h"
#include "math/LegacyMathAdapters.h"

void Shape::setTransform(const clrt::math::Mat4& matrix) {
    transformMatrix = matrix;
    inverseTransform = matrix.inverse();
    inverseTranspose = inverseTransform.transposed();
}

void Shape::setTransform(const Matrix& matrix) {
    setTransform(clrt::compat::matrixFromLegacy(matrix));
}

void Shape::setPosition(const std::vector<double>& point) {
    position = clrt::compat::pointFromLegacyTuple(point);
}

clrt::math::Point3 Shape::worldToObject(const clrt::math::Point3& point) const {
  if (parent != nullptr) {
    return inverseTransform * parent->worldToObject(point);
  }
  return inverseTransform * point;
}

clrt::math::Vec3 Shape::normalToWorld(const clrt::math::Vec3& normal) const {
    clrt::math::Vec3 worldNormal = inverseTranspose * normal;

    if (parent != nullptr) {
        return parent->normalToWorld(worldNormal);
    }

    return worldNormal.normalized();
}

std::vector<double> Shape::normal_at(const std::vector<double>& worldPoint) const {
    return clrt::compat::toLegacyTuple(
        normalAt(clrt::compat::pointFromLegacyTuple(worldPoint))
    );
}

std::vector<double> Shape::world_to_object(const std::vector<double>& point) const {
    return clrt::compat::toLegacyTuple(
        worldToObject(clrt::compat::pointFromLegacyTuple(point))
    );
}

std::vector<double> Shape::normal_to_world(const std::vector<double>& normal) const {
    return clrt::compat::toLegacyTuple(
        normalToWorld(clrt::compat::vectorFromLegacyTuple(normal))
    );
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

    for (int i = 0; i < 8; ++i) {
        const clrt::math::Point3 point{
            corners[i][0],
            corners[i][1],
            corners[i][2]
        };
        const clrt::math::Point3 transformed = transformMatrix * point;

        result.add_point(
            transformed.x,
            transformed.y,
            transformed.z
        );
    }

    return result;
}

void Shape::setEmissiveColor(const Color& color) {
    material.emissiveColor = color;
}

void Shape::setEmissiveStrength(double strength) {
    material.emissiveStrength = strength;
}
