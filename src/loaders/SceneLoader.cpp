#include "loaders/SceneLoader.h"
#include "geometry/Cube.h"
#include "geometry/Cylinder.h"
#include "geometry/Group.h"
#include "geometry/Plane.h"
#include "geometry/Sphere.h"
#include "geometry/Triangle.h"
#include "scene/Pattern.h"
#include "scene/PointLight.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Return a 3-element array for transforms (scale/translate)
static std::array<double,3> asTriple(const json& a) {
    return { a.at(0).get<double>(), a.at(1).get<double>(), a.at(2).get<double>() };
}

static clrt::math::Point3 asPoint(const json& a) {
    return { a.at(0).get<double>(), a.at(1).get<double>(), a.at(2).get<double>() };
}

static clrt::math::Vec3 asVector(const json& a) {
    return { a.at(0).get<double>(), a.at(1).get<double>(), a.at(2).get<double>() };
}

static Color jsonToColor(const json& a) {
    auto v = asTriple(a);
    return Color(v[0], v[1], v[2]);
}

static std::unique_ptr<Shape> jsonToShape(const json& object);

bool LoadSceneFromJson(const std::string& path, Camera& outCam, World& outWorld,
                       std::string& outImageFile, bool& outMultiThreaded,
                       std::string* outError) {
    const auto fail = [outError](const std::string& message) {
        if (outError != nullptr) {
            *outError = message;
        }
        return false;
    };

    std::ifstream ifs(path);
    if (!ifs) return fail("could not open file");
    json root;
    try {
        ifs >> root;
    } catch (const std::exception& error) {
        return fail(std::string("invalid JSON: ") + error.what());
    }

    Camera loadedCamera = outCam;
    World loadedWorld;
    std::string loadedImageFile = outImageFile;
    bool loadedMultiThreaded = false;

    try {
        // Image
        if (root.contains("image")) {
            auto img = root["image"];
            if (img.contains("file")) {
                loadedImageFile = img["file"].get<std::string>();
            }
            if (img.contains("multithreaded")) {
                loadedMultiThreaded = img["multithreaded"].get<bool>();
            }
        }

        // Camera
        if (root.contains("camera")) {
            auto cam = root["camera"];
            int h = cam.value("hsize", 100);
            int v = cam.value("vsize", 50);
            double fov = cam.value("fov", 1.0471975512);
            loadedCamera = Camera(h, v, fov);
            if (cam.contains("from") && cam.contains("to") && cam.contains("up")) {
                const clrt::math::Point3 from = asPoint(cam["from"]);
                const clrt::math::Point3 to = asPoint(cam["to"]);
                const clrt::math::Vec3 up = asVector(cam["up"]);
                loadedCamera.setTransform(
                    clrt::math::Mat4::viewTransform(from, to, up));
            }
        }

        // Lights
        if (root.contains("lights")) {
            auto lights = root["lights"];
            if (lights.empty() || lights.size() > 4) {
                throw std::invalid_argument(
                    "Scene lights must contain between one and four entries");
            }
            loadedWorld.clearLights();
            for (auto& light : lights) {
                std::string type = light.value("type", "point");
                if (type != "point") {
                    throw std::invalid_argument(
                        "Unsupported light type: " + type);
                }
                auto position = asPoint(light["position"]);
                Color color = light.contains("color")
                    ? jsonToColor(light["color"])
                    : Color(1, 1, 1);
                loadedWorld.addLight(
                    std::make_unique<PointLight>(position, color));
            }
        }

        // Objects
        if (root.contains("objects")) {
            auto objects = root["objects"];
            for (auto& object : objects) {
                loadedWorld.AddShape(jsonToShape(object));
            }
        }
    } catch (const std::exception& error) {
        return fail(error.what());
    } catch (...) {
        return fail("unknown scene loading error");
    }

    outCam = std::move(loadedCamera);
    outWorld = std::move(loadedWorld);
    outImageFile = std::move(loadedImageFile);
    outMultiThreaded = loadedMultiThreaded;

    return true;
}

static clrt::math::Mat4 jsonToTransform(const json& value) {
    clrt::math::Mat4 transform;
    if (value.contains("scale")) {
        const auto scale = asTriple(value["scale"]);
        transform = clrt::math::Mat4::scaling(scale[0], scale[1], scale[2]);
    }
    if (value.contains("rotate")) {
        const auto rotate = asTriple(value["rotate"]);
        transform = clrt::math::Mat4::rotationZ(rotate[2])
            * clrt::math::Mat4::rotationY(rotate[1])
            * clrt::math::Mat4::rotationX(rotate[0])
            * transform;
    }
    if (value.contains("translate")) {
        const auto translate = asTriple(value["translate"]);
        transform = clrt::math::Mat4::translation(
            translate[0], translate[1], translate[2]) * transform;
    }
    return transform;
}

static std::shared_ptr<Pattern> jsonToPattern(const json& value) {
    const std::string type = value.at("type").get<std::string>();
    std::shared_ptr<Pattern> pattern;
    if (type == "stripe") {
        pattern = std::make_shared<StripePattern>(
            jsonToColor(value.at("colorA")), jsonToColor(value.at("colorB")));
    } else if (type == "checkers") {
        pattern = std::make_shared<CheckersPattern>(
            jsonToColor(value.at("colorA")), jsonToColor(value.at("colorB")));
    } else if (type == "gradient") {
        pattern = std::make_shared<GradientPattern>(
            jsonToColor(value.at("colorA")), jsonToColor(value.at("colorB")));
    } else if (type == "ring") {
        pattern = std::make_shared<RingPattern>(
            jsonToColor(value.at("colorA")), jsonToColor(value.at("colorB")));
    } else if (type == "perturbed") {
        pattern = std::make_shared<PertubedPattern>(
            jsonToPattern(value.at("base")),
            value.value("distortionScale", 0.2),
            value.value("noiseFrequency", 2.0));
    } else {
        throw std::invalid_argument("Unsupported pattern type: " + type);
    }

    if (value.contains("transform")) {
        pattern->setTransform(jsonToTransform(value["transform"]));
    }
    return pattern;
}

static void applyMaterial(Shape& shape, const json& material) {
    if (material.contains("color")) shape.setMaterialColor(jsonToColor(material["color"]));
    if (material.contains("ambient")) shape.setAmbient(material["ambient"].get<double>());
    if (material.contains("diffuse")) shape.setDiffuse(material["diffuse"].get<double>());
    if (material.contains("specular")) shape.setSpecular(material["specular"].get<double>());
    if (material.contains("shininess")) shape.setShininess(material["shininess"].get<double>());
    if (material.contains("reflective")) shape.setReflective(material["reflective"].get<double>());
    if (material.contains("transparency")) shape.setTransparency(material["transparency"].get<double>());
    if (material.contains("refractiveIndex")) shape.setRefractiveIndex(material["refractiveIndex"].get<double>());
    if (material.contains("pattern") && !material["pattern"].is_null()) {
        shape.setMaterialPattern(jsonToPattern(material["pattern"]));
    }
}

static std::unique_ptr<Shape> jsonToShape(const json& object) {
    const std::string type = object.at("type").get<std::string>();
    std::unique_ptr<Shape> shape;
    if (type == "sphere") {
        shape = std::make_unique<Sphere>();
    } else if (type == "plane") {
        shape = std::make_unique<Plane>();
    } else if (type == "cube") {
        shape = std::make_unique<Cube>();
    } else if (type == "cylinder") {
        auto cylinder = std::make_unique<Cylinder>();
        const double minimum = object.value("minimum", -1.0);
        const double maximum = object.value("maximum", 1.0);
        if (minimum >= maximum) {
            throw std::invalid_argument(
                "Cylinder minimum must be less than maximum");
        }
        cylinder->setMin(minimum);
        cylinder->setMax(maximum);
        cylinder->setClosed(object.value("closed", false));
        shape = std::move(cylinder);
    } else if (type == "triangle") {
        shape = std::make_unique<Triangle>(
            asPoint(object.at("p1")),
            asPoint(object.at("p2")),
            asPoint(object.at("p3")));
    } else if (type == "group") {
        auto group = std::make_unique<Group>();
        const auto& children = object.at("children");
        if (children.empty()) {
            throw std::invalid_argument("Groups require at least one child");
        }
        for (const auto& child : children) {
            group->add_child(std::shared_ptr<Shape>(jsonToShape(child)));
        }
        shape = std::move(group);
    } else {
        throw std::invalid_argument("Unsupported object type: " + type);
    }

    if (object.contains("transform")) shape->setTransform(jsonToTransform(object["transform"]));
    if (object.contains("material")) applyMaterial(*shape, object["material"]);
    return shape;
}
