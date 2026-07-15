#include "World.h"
#include "scene/LightShadeVector.h"
#include "scene/PointLight.h"
#include "geometry/Group.h"
#include <stdexcept>
#include <cmath>
#include <memory>
#include <unordered_set>
#include <utility>

namespace {

void inspectUnregisteredTree(
    const Shape& shape,
    std::unordered_set<const Shape*>& visited
) {
    if (!visited.insert(&shape).second) {
        throw std::logic_error("Shape trees cannot contain cycles or repeated objects");
    }
    if (shape.getObjectId() || shape.getMaterialId()) {
        throw std::logic_error("Shape already belongs to an object registry");
    }

    const auto* group = dynamic_cast<const Group*>(&shape);
    if (group != nullptr) {
        for (const auto& child : group->get_children()) {
            inspectUnregisteredTree(*child, visited);
        }
    }
}

} // namespace

World::World()
    : World(std::make_unique<PointLight>(
          clrt::math::Point3{-10, 10, -10},
          Color{1, 1, 1})) {}

World::World(std::unique_ptr<Light> light) {
    setLight(std::move(light));
}

Shape& World::AddShape(std::unique_ptr<Shape> shape) {
    if (!shape) {
        throw std::invalid_argument("World cannot own a null shape");
    }

    std::unordered_set<const Shape*> shapeTree;
    inspectUnregisteredTree(*shape, shapeTree);
    const std::size_t additionalObjects = shapeTree.size();
    if (additionalObjects > clrt::scene::ObjectId::invalidValue - objectsById.size()
        || additionalObjects > clrt::scene::MaterialId::invalidValue - materialsById.size()) {
        throw std::length_error("World stable ID space is exhausted");
    }
    objectsById.reserve(objectsById.size() + additionalObjects);
    materialsById.reserve(materialsById.size() + additionalObjects);
    shapesList.push_back(std::move(shape));
    registerShapeTree(*shapesList.back());
    return *shapesList.back();
}

void World::registerShapeTree(Shape& shape) {
    const auto objectValue = static_cast<clrt::scene::ObjectId::Value>(
        objectsById.size());
    const auto materialValue = static_cast<clrt::scene::MaterialId::Value>(
        materialsById.size());
    shape.assignStableIds(
        clrt::scene::ObjectId{objectValue},
        clrt::scene::MaterialId{materialValue});
    objectsById.push_back(&shape);
    materialsById.push_back(&shape.getMaterial());

    auto* group = dynamic_cast<Group*>(&shape);
    if (group != nullptr) {
        for (const auto& child : group->get_children()) {
            registerShapeTree(*child);
        }
    }
}

const Shape& World::resolve(clrt::scene::ObjectId id) const {
    if (!id) {
        throw std::out_of_range("Cannot resolve an invalid object ID");
    }
    return *objectsById.at(id.value());
}

const Material& World::material(clrt::scene::MaterialId id) const {
    if (!id) {
        throw std::out_of_range("Cannot resolve an invalid material ID");
    }
    return *materialsById.at(id.value());
}

void World::setLight(std::unique_ptr<Light> light) {
    if (!light) {
        throw std::invalid_argument("World cannot own a null light");
    }
    clearLights();
    addLight(std::move(light));
}

Light& World::addLight(std::unique_ptr<Light> light) {
    if (!light) {
        throw std::invalid_argument("World cannot own a null light");
    }

    auto newLighting = std::make_unique<Lighting>(*light);
    sceneLights.push_back(std::move(light));
    try {
        lighting.push_back(std::move(newLighting));
    } catch (...) {
        sceneLights.pop_back();
        throw;
    }
    return *sceneLights.back();
}

void World::clearLights() noexcept {
    lighting.clear();
    sceneLights.clear();
}

const Light& World::getLight() const {
    if (sceneLights.empty()) {
        throw std::runtime_error("World has no lighting configured.");
    }
    return *sceneLights.front();
}


// Intersect World Function 
// Iterate over all of the objects that have been added to the world, intersecting them with the ray. 
// Aggregate the intersections into a single collection 
Intersections World::intersect_world(const Ray& ray) {
    Intersections intersectionList; 

    for (const auto& s : shapesList) {
        s->intersect(ray, intersectionList); // Call intersect on each shape in the list, and add the intersections to the intersection list
    }

    intersectionList.Sort(); // Sort, then return in ascending order
    return intersectionList; 
}

// Measure the distance from point to the light source by subtracting point from the light position, and take the magnitude of the resulting vector (distance)
// Create a ray from point toward the light source by normalizing the vector from step 1
// Intersect the World with that Ray
// Check to see if there was a hit, and if so wether t is less than distance. If so, the hit lies between the point and the light source, and the point is in shadow. 
bool World::is_shadowed(const clrt::math::Point3& point) {
    if (lighting.empty()) {
        throw std::runtime_error("World has no lighting configured.");
    }
    return is_shadowed(point, *lighting.front());
}

bool World::is_shadowed(
    const clrt::math::Point3& point,
    const Lighting& light
) {
    const clrt::math::Vec3 lightOffset = light.getPos() - point;
    const double distance = lightOffset.length();
    const clrt::math::Vec3 direction = lightOffset.normalized();

    Ray ray(point, direction);
    Intersections ints; 
    ints = intersect_world(ray); 

   const Intersection* intersection = ints.hit(); 

   if (intersection != nullptr && intersection->getT() < distance) {
        return true; 
   }

   return false; 
}


// Create a new ray originating at the hits location and pointing in the diretion of reflectv. Find the color of the new ray via color_at(). 
// Then multiply the result by the reflective value. If reflective is set to something between 0-1, it will give you partial reflection. 
Color World::reflected_color(const Computations& comps, int remaining) {
    if (remaining <= 0) {
        return Color{0, 0, 0}; // Return black 
    }

    const Material& material = this->material(comps.materialId);
    if (material.reflective == 0) {
        return Color{0, 0, 0}; 
    }

    Ray reflect_ray(comps.overPt, comps.reflectv); 
    Color color = Color_at(reflect_ray, remaining - 1); 

    return color * material.reflective;
}

/*CODE REVIEW; refracted_color() calculates the color contribution from light passing through a transparent object */

// NEW: calculate the color contributed by a ray passing through a transparent object
Color World::refracted_color(const Computations& comps,int remaining) {
    const Material& material = this->material(comps.materialId);

    // stop recursion or skip completely opaque materials
    if (remaining <= 0 || material.transparency <= 0.0) {
        return Color{0, 0, 0};
    }

    // Snell's law.. ratio of the two refractive indices
    const double nRatio = comps.n1 / comps.n2;

    // Angle between the eye vector and surface normal
    const double cosI =
        clrt::math::dot(comps.eyev, comps.normalv);

    // Calculate the squared sine of the transmitted angle
    const double sin2T =
        nRatio * nRatio * (1.0 - cosI * cosI);

    // Total internal reflection means no refracted ray exists
    if (sin2T > 1.0) {
        return Color{0, 0, 0};
    }

    const double cosT = std::sqrt(1.0 - sin2T);

    // implemented to calculate the direction of the refracted ray
    const clrt::math::Vec3 direction =
        comps.normalv * (nRatio * cosI - cosT)
        - comps.eyev * nRatio;

    // underPt begins slightly beneath the surface so the ray
    // does not immediately intersect the same object again
    Ray refractRay(comps.underPt, direction);

    const Color color =
        Color_at(refractRay, remaining - 1);

    return color * material.transparency;
}

//Implementing shading... We check if pt is a shadow or not, then pass it to process lighting
// NEW CODE REVIEW: we are adding the refracted color contribution
Color World::shade_hit(const Computations& comps, int remaining) {
    if (lighting.empty()) {
        throw std::runtime_error("World has no lighting configured."); // Commented out, as lighting starts of null to get configured 
     }

    LightShadeVector lsv;
    lsv.E = comps.eyev;
    lsv.N = comps.normalv;


   const Shape& object = resolve(comps.objectId);
   const Material& material = this->material(comps.materialId);
   Color surface{0.0, 0.0, 0.0};
   for (const auto& light : lighting) {
       surface = surface + light->ProcessLighting(
           &object,
           material,
           lsv,
           comps.point,
           is_shadowed(comps.overPt, *light));
   }

    const Material& mat = material;

    Color reflected = reflected_color(comps, remaining); 
    Color refracted = refracted_color(comps, remaining);

    if (mat.reflective > 0 && mat.transparency > 0) {
        double reflectance = schlick(comps); 
        return surface + reflected * reflectance + refracted * (1 - reflectance); 
    }

    // added the refracted 
    return surface + reflected + refracted; 
}





// Find color_at the hit()
Color World::Color_at(const Ray& ray, int remaining) {
    Intersections intersections = intersect_world(ray);

    // Get a pointer to the closest hit
    const Intersection* intersection = intersections.hit(); 

    // If no valid positive intersection exists, return black immediately
    if (intersection == nullptr) {
        return Color{0, 0, 0};
    }

    // the complete intersection list is what allows your nested-object code to calculate the correct n1 and n2
    Computations comp = prepareComputations(*intersection, ray, intersections, *this);

    return shade_hit(comp, remaining);
}
