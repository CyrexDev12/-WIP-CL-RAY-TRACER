#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include <vector>
#include "geometry/Shape.h"
#include "scene/Lighting.h"
#include "scene/ObjectResolver.h"
#include "geometry/Computations.h"

class World : public clrt::scene::ObjectResolver {
private: 
    std::vector<std::unique_ptr<Shape>> shapesList;
    std::vector<const Shape*> objectsById;
    std::vector<const Material*> materialsById;
    std::vector<std::unique_ptr<Light>> sceneLights;
    std::vector<std::unique_ptr<Lighting>> lighting;

    void registerShapeTree(Shape& shape);

public: 
    World();
    explicit World(std::unique_ptr<Light> light);
    ~World() = default;

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) noexcept = default;
    World& operator=(World&&) noexcept = default;

    Shape& AddShape(std::unique_ptr<Shape> shape);
    void setLight(std::unique_ptr<Light> light);
    Light& addLight(std::unique_ptr<Light> light);
    void clearLights() noexcept;

    [[nodiscard]] const std::vector<std::unique_ptr<Shape>>& shapes() const noexcept {
        return shapesList;
    }

    [[nodiscard]] const Light& getLight() const;
    [[nodiscard]] const std::vector<std::unique_ptr<Light>>& lights() const noexcept {
        return sceneLights;
    }

    [[nodiscard]] const Shape& resolve(clrt::scene::ObjectId id) const override;
    [[nodiscard]] const Material& material(clrt::scene::MaterialId id) const;
    [[nodiscard]] std::size_t objectCount() const noexcept { return objectsById.size(); }
    [[nodiscard]] std::size_t materialCount() const noexcept { return materialsById.size(); }

    Intersections intersect_world(const Ray& ray); 

    bool is_shadowed(const clrt::math::Point3& point);
    bool is_shadowed(
        const clrt::math::Point3& point,
        const Lighting& light);

    Color reflected_color(const Computations& comps, int remaining);
    
    Color refracted_color(const Computations& comps, int remaining);

    Color shade_hit(const Computations& comps, int remaining);


    Color Color_at(const Ray& ray, int remaining = 5);
};

#endif
