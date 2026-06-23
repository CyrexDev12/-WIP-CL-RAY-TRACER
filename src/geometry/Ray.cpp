#include "geometry/Ray.h"


// Compute the point at the given distance t along the ray
  vector<double> Ray::position(double t) const {
    vector<double> scaledDirection = ScaleTuple(direction, t);
    return AddTuples(origin, scaledDirection);
}

// Apply a transformation to the ray by transforming both its origin and direction using the provided matrix
// Then return a new ray with the transformed origin and direction
Ray Ray::transform(Matrix m) {
    Ray transformed_ray; 


    transformed_ray.origin = m.multiplyTuple(this->origin);
    transformed_ray.direction = m.multiplyTuple(this->direction);

  return transformed_ray;

}