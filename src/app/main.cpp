#include <fstream>
#include <iostream>
#include <string>

#include "loaders/SceneLoader.h"
#include "renderers/cpu/CpuRenderer.h"
#include "scene/Camera.h"
#include "scene/canvas.h"
#include "scene/World.h"

namespace {

void writeCanvasToFile(Canvas& canvas, const std::string& path) {
   std::ofstream ofs(path);
   if (!ofs) {
      std::cerr << "Failed to open output file: " << path << std::endl;
      return;
   }
   ofs << canvas.convertToPpm();
}

void printUsage(const char* executable) {
   std::cout << "Usage:\n"
             << "  " << executable << " --scene <scene.json>\n";
}

} // namespace


int main(int argc, char** argv) {
   if (argc >= 3 && std::string(argv[1]) == "--scene") {
      std::string scenePath = argv[2];
      Camera cam;
      World world;
      std::string outFile = "out.ppm";
      std::string loadError;
      bool multiThreaded = false;
      bool ok = LoadSceneFromJson(
         scenePath, cam, world, outFile, multiThreaded, &loadError);
      if (!ok) {
         std::cerr << "Failed to load scene: " << scenePath;
         if (!loadError.empty()) {
            std::cerr << " (" << loadError << ")";
         }
         std::cerr << std::endl;
         return 1;
      }
      Canvas cnv = renderCpu(cam, world, multiThreaded);
      writeCanvasToFile(cnv, outFile);
      std::cout << "Rendered scene to " << outFile << std::endl;
      return 0;
   }

   printUsage(argv[0]);
   return argc == 1 ? 0 : 2;
}
