#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <string>
#include "scene/World.h"
#include "scene/Camera.h"

// Transactionally loads a scene into provided Camera and World instances. World
// owns all loaded lights and shapes; outputs remain unchanged on failure.
// Returns true on success and sets outImageFile to the target output filename (if present).
// outMultiThreaded will be set true if the JSON requests a multithreaded render
// When outError is provided, failures include a diagnostic suitable for display.
bool LoadSceneFromJson(const std::string& path, Camera& outCam, World& outWorld,
                       std::string& outImageFile, bool& outMultiThreaded,
                       std::string* outError = nullptr);

#endif
