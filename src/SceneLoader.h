#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <string>
#include "scene/World.h"
#include "scene/Camera.h"

// Loads a scene from a JSON file into provided Camera and World instances.
// Returns true on success and sets outImageFile to the target output filename (if present).
bool LoadSceneFromJson(const std::string& path, Camera& outCam, World& outWorld, std::string& outImageFile);

#endif
