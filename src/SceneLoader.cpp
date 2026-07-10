#include "SceneLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Return a 3-element array for transforms (scale/translate)
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

bool LoadSceneFromJson(const std::string& path, Camera& outCam, World& outWorld, std::string& outImageFile, bool& outMultiThreaded) {
    std::ifstream ifs(path);
    if (!ifs) return false;
    json root;
    try { ifs >> root; } catch (...) { return false; }

    // Image
    outMultiThreaded = false;
    if (root.contains("image")) {
        auto img = root["image"];
        if (img.contains("file")) outImageFile = img["file"].get<std::string>();
        if (img.contains("multithreaded")) outMultiThreaded = img["multithreaded"].get<bool>();
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
        auto objs = root["objects"];
        for (auto &o : objs) {
            std::string type = o.value("type", "sphere");
            if (type == "sphere") {
                Sphere* s = new Sphere();

                // Transform
                if (o.contains("transform")) {
                    auto t = o["transform"];
                    Matrix m;
                    Matrix tr = Matrix();
                    // apply scale then translate if present
                    if (t.contains("scale")) {
                        auto sc = asTriple(t["scale"]);
                        tr = m.scale(sc[0], sc[1], sc[2]);
                    }
                    if (t.contains("translate")) {
                        auto tv = asTriple(t["translate"]);
                        Matrix tt = m.translation(tv[0], tv[1], tv[2]);
                        tr = tt.multiplyMatrix(tr);
                    }
                    s->setTransform(tr);
                }

                // Material
                if (o.contains("material")) {
                    auto mat = o["material"];
                    if (mat.contains("color")) s->setMaterialColor(jsonToColor(mat["color"]));
                    if (mat.contains("ambient")) s->setAmbient(mat["ambient"].get<double>());
                    if (mat.contains("diffuse")) s->setDiffuse(mat["diffuse"].get<double>());
                    if (mat.contains("specular")) s->setSpecular(mat["specular"].get<double>());
                    if (mat.contains("shininess")) s->setShininess(mat["shininess"].get<double>());
                    if (mat.contains("reflective")) s->setReflective(mat["reflective"].get<double>());
                    if (mat.contains("transparency")) s->setTransparency(mat["transparency"].get<double>());
                    if (mat.contains("refractiveIndex")) s->setRefractiveIndex(mat["refractiveIndex"].get<double>());
                }

                outWorld.AddShape(s);
            }
        }
    }

    return true;
}
