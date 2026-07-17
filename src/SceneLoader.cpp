#include "SceneLoader.h"
#include "geometry/Cube.h"
#include "geometry/Cylinder.h"
#include "geometry/Group.h"
#include "geometry/Plane.h"
#include "geometry/Sphere.h"
#include "geometry/Triangle.h"
#include "scene/Pattern.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

// Return a 3-element array for transforms and colors.
static std::array<double,3> asTriple(const json& a) {
    return { a.at(0).get<double>(), a.at(1).get<double>(), a.at(2).get<double>() };
}

// Return a 4D tuple representing a point (w=1)
static std::vector<double> asPoint(const json& a) {
    return { a.at(0).get<double>(), a.at(1).get<double>(), a.at(2).get<double>(), 1.0 };
}

// Return a 4D tuple representing a vector (w=0)
static std::vector<double> asVector(const json& a) {
    return { a.at(0).get<double>(), a.at(1).get<double>(), a.at(2).get<double>(), 0.0 };
}

static Color jsonToColor(const json& a) {
    auto v = asTriple(a);
    return Color(v[0], v[1], v[2]);
}

// Pattern and object transforms use scale, X/Y/Z rotation, then translation.
static Matrix jsonToTransform(const json& transform) {
    Matrix matrix;
    Matrix result;

    if (transform.contains("scale")) {
        auto scale = asTriple(transform["scale"]);
        result = matrix.scale(scale[0], scale[1], scale[2]);
    }
    if (transform.contains("rotate")) {
        auto rotation = asTriple(transform["rotate"]);
        Matrix rotateX = matrix.rotateX(rotation[0]);
        result = rotateX.multiplyMatrix(result);
        Matrix rotateY = matrix.rotateY(rotation[1]);
        result = rotateY.multiplyMatrix(result);
        Matrix rotateZ = matrix.rotateZ(rotation[2]);
        result = rotateZ.multiplyMatrix(result);
    }
    if (transform.contains("translate")) {
        auto translation = asTriple(transform["translate"]);
        Matrix translationMatrix = matrix.translation(
            translation[0], translation[1], translation[2]
        );
        result = translationMatrix.multiplyMatrix(result);
    }

    return result;
}

static std::shared_ptr<Pattern> jsonToPattern(const json& patternJson) {
    const std::string type = patternJson.at("type").get<std::string>();
    std::shared_ptr<Pattern> pattern;

    if (type == "stripe") {
        pattern = std::make_shared<StripePattern>(
            jsonToColor(patternJson.at("colorA")),
            jsonToColor(patternJson.at("colorB"))
        );
    } else if (type == "checkers") {
        pattern = std::make_shared<CheckersPattern>(
            jsonToColor(patternJson.at("colorA")),
            jsonToColor(patternJson.at("colorB"))
        );
    } else if (type == "gradient") {
        pattern = std::make_shared<GradientPattern>(
            jsonToColor(patternJson.at("colorA")),
            jsonToColor(patternJson.at("colorB"))
        );
    } else if (type == "ring") {
        pattern = std::make_shared<RingPattern>(
            jsonToColor(patternJson.at("colorA")),
            jsonToColor(patternJson.at("colorB"))
        );
    } else if (type == "perturbed" || type == "pertubed") {
        pattern = std::make_shared<PertubedPattern>(
            jsonToPattern(patternJson.at("base")),
            patternJson.value("distortionScale", 0.2),
            patternJson.value("noiseFrequency", 2.0)
        );
    } else {
        throw std::invalid_argument("Unsupported pattern type: " + type);
    }

    if (patternJson.contains("transform")) {
        pattern->transform = jsonToTransform(patternJson["transform"]);
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
    if (material.contains("emissiveColor")) shape.setEmissiveColor(jsonToColor(material["emissiveColor"]));
    if (material.contains("emissiveStrength")) shape.setEmissiveStrength(material["emissiveStrength"].get<double>());
    if (material.contains("pattern")) shape.setMaterialPattern(jsonToPattern(material["pattern"]));
}

static std::unique_ptr<Shape> jsonToShape(const json& objectJson) {
    const std::string type = objectJson.at("type").get<std::string>();
    std::unique_ptr<Shape> shape;

    if (type == "sphere") {
        shape = std::make_unique<Sphere>();
    } else if (type == "plane") {
        shape = std::make_unique<Plane>();
    } else if (type == "cube") {
        shape = std::make_unique<Cube>();
    } else if (type == "cylinder") {
        auto cylinder = std::make_unique<Cylinder>();
        const double minimum = objectJson.value("minimum", -1.0);
        const double maximum = objectJson.value("maximum", 1.0);
        if (minimum >= maximum) {
            throw std::invalid_argument("Cylinder minimum must be less than maximum");
        }
        cylinder->setMin(minimum);
        cylinder->setMax(maximum);
        cylinder->setClosed(objectJson.value("closed", false));
        shape = std::move(cylinder);
    } else if (type == "triangle") {
        shape = std::make_unique<Triangle>(
            asPoint(objectJson.at("p1")),
            asPoint(objectJson.at("p2")),
            asPoint(objectJson.at("p3"))
        );
    } else if (type == "group") {
        auto group = std::make_unique<Group>();
        for (const auto& childJson : objectJson.at("children")) {
            std::unique_ptr<Shape> child = jsonToShape(childJson);
            group->add_child(std::shared_ptr<Shape>(std::move(child)));
        }
        shape = std::move(group);
    } else {
        throw std::invalid_argument("Unsupported object type: " + type);
    }

    if (objectJson.contains("transform")) {
        shape->setTransform(jsonToTransform(objectJson["transform"]));
    }
    if (objectJson.contains("material")) {
        applyMaterial(*shape, objectJson["material"]);
    }
    return shape;
}

bool LoadSceneFromJson(
    const std::string& path,
    Camera& outCam,
    World& outWorld,
    SceneRenderSettings& outSettings
) {
    std::ifstream ifs(path);
    if (!ifs) return false;
    json root;
    try { ifs >> root; } catch (...) { return false; }

    // Image
    outSettings = SceneRenderSettings{};
    if (root.contains("image")) {
        auto img = root["image"];
        if (img.contains("file")) outSettings.imageFile = img["file"].get<std::string>();
        if (img.contains("multithreaded")) outSettings.multithreaded = img["multithreaded"].get<bool>();
        if (img.contains("bloom")) outSettings.bloom = img["bloom"].get<bool>();
        if (img.contains("bloomIntensity")) outSettings.bloomIntensity = img["bloomIntensity"].get<double>();
        if (img.contains("bloomThreshold")) outSettings.bloomThreshold = img["bloomThreshold"].get<double>();
        if (img.contains("bloomRadius")) outSettings.bloomRadius = img["bloomRadius"].get<int>();
    }

    // Camera
    if (root.contains("camera")) {
        auto cam = root["camera"];
        int h = cam.value("hsize", 100);
        int v = cam.value("vsize", 50);
        double fov = cam.value("fov", 1.0471975512);
        outCam = Camera(h, v, fov);
        if (cam.contains("from") && cam.contains("to") && cam.contains("up")) {
            std::vector<double> from = asPoint(cam["from"]);
            std::vector<double> to = asPoint(cam["to"]);
            std::vector<double> up = asVector(cam["up"]);
            Matrix m;
            Matrix view = m.viewTransformation(from, to, up);
            outCam.setTransformM(view);
        }
    }

    // Lights
    if (root.contains("lights")) {
        auto lights = root["lights"];
        for (auto &l : lights) {
            std::string type = l.value("type", "point");
            if (type == "point") {
                auto pos = asPoint(l["position"]);
                Color col = l.contains("color") ? jsonToColor(l["color"]) : Color(1,1,1);
                PointLight* pl = new PointLight(pos, col);
                Lighting* lighting = new Lighting(*pl);
                outWorld.setLighting(*lighting);
            }
        }
    }

    // Objects
    if (root.contains("objects")) {
        try {
            for (const auto& objectJson : root["objects"]) {
                std::unique_ptr<Shape> shape = jsonToShape(objectJson);
                outWorld.AddShape(shape.release());
            }
        } catch (const std::exception& error) {
            std::cerr << "Failed to load scene object: " << error.what() << std::endl;
            return false;
        }
    }

    return true;
}
