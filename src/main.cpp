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
      std::string outFile = "out.ppm";
      bool multiThreaded = false;
      bool ok = LoadSceneFromJson(scenePath, cam, world, outFile, multiThreaded);
      if (!ok) {
         std::cerr << "Failed to load scene: " << scenePath << std::endl;
         return 1;
      }
      Canvas cnv = render(cam, world, multiThreaded);
      writeCanvasToFile(cnv, outFile);
      std::cout << "Rendered scene to " << outFile << std::endl;
      return 0;
   }

   // Default: run tests
   DarkSideTriangleTest();
   return 0;
}