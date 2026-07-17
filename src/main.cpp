#include <iostream>
#include <vector>
#include <string> 
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include "math/Matrix.h"
#include "Tests.h"
#include "geometry/Intersection.h"
#include "SceneLoader.h"
#include "scene/Camera.h"
#include "scene/canvas.h"
#include "scene/World.h"

using namespace std;

static void writeCanvasToFile(Canvas& c, const std::string& path) {
   std::ofstream ofs(path);
   if (!ofs) {
      std::cerr << "Failed to open output file: " << path << std::endl;
      return;
   }
   ofs << c.convertToPpm();
   ofs.close();
}


using namespace std; 


int main(int argc, char** argv) {
   // If a scene JSON is provided, load and render it
   if (argc >= 3 && std::string(argv[1]) == "--scene") {
      std::string scenePath = argv[2];
      Camera cam;
      World world;
      SceneRenderSettings settings;
      bool ok = LoadSceneFromJson(scenePath, cam, world, settings);
      if (!ok) {
         std::cerr << "Failed to load scene: " << scenePath << std::endl;
         return 1;
      }
      Canvas cnv = render(cam, world, settings.multithreaded);
      cnv.bloomEnabled = settings.bloom;
      cnv.bloomIntensity = settings.bloomIntensity;
      cnv.bloomThreshold = settings.bloomThreshold;
      cnv.bloomRadius = settings.bloomRadius;
      writeCanvasToFile(cnv, settings.imageFile);
      std::cout << "Rendered scene to " << settings.imageFile << std::endl;
      return 0;
   }

   // Default: run tests
   DarkSideTriangleTest();
   return 0;
}
