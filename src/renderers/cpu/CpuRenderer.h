#ifndef RENDERERS_CPU_CPURENDERER_H
#define RENDERERS_CPU_CPURENDERER_H

#include "scene/Camera.h"
#include "scene/World.h"
#include "scene/canvas.h"

// Boundary for the existing CPU implementation while renderer backends are split.
Canvas renderCpu(const Camera& camera, World& world, bool multithreaded = false);

#endif
