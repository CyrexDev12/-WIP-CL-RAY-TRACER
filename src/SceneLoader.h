#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <string>
#include "scene/World.h"
#include "scene/Camera.h"

struct SceneRenderSettings {
    std::string imageFile = "out.ppm";
    bool multithreaded = false;
    bool bloom = false;
    double bloomIntensity = 0.35;
    double bloomThreshold = 1.0;
    int bloomRadius = 6;
    bool toneMapping = true;
    double exposure = 1.0;
    double gamma = 2.2;
};

// Loads a scene from a JSON file into provided Camera and World instances.
// Returns true on success and fills the image/render settings when present.
bool LoadSceneFromJson(
    const std::string& path,
    Camera& outCam,
    World& outWorld,
    SceneRenderSettings& outSettings
);

#endif
