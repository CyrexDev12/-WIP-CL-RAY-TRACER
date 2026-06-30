#include "Tests.h"
#define M_PI       3.14159265358979323846   // pi
#include <cmath>
#include <fstream>
#include <memory>
#include <utility>
#include "geometry/Intersection.h"
#include "geometry/Ray.h"
#include "geometry/Sphere.h"
#include <cassert>
#include "scene/LightShadeVector.h"
#include "scene/Lighting.h"
#include "scene/PointLight.h"
#include "scene/World.h"
#include "geometry/Computations.h"
#include "scene/Camera.h"
#include "geometry/Plane.h"
#include "scene/Pattern.h"

using namespace std; 


// helper function to compare two floating-point numbers for approximate equality
bool almostEqual(double a, double b) {
    return fabs(a - b) < 0.00001;
}

// Helper Function 
bool tupleEqual(const vector<double>& a, const vector<double>& b) {
    if (a.size() != b.size()) return false;

    for (int i = 0; i < a.size(); i++) {
        if (!almostEqual(a[i], b[i])) return false;
    }

    return true;
}

bool colorEqual(
    const Color& first,
    const Color& second
) {
    return almostEqual(first.r, second.r) &&
           almostEqual(first.g, second.g) &&
           almostEqual(first.b, second.b);
}

/*

// SHEARING TRANSFORMATION TEST 
// Status: Test passeed 
void runShearingTest(string testName, Matrix transform, vector<double> p, vector<double> expected) {
    vector<double> result = transform.multiplyTuple(p);

    cout << testName << ": ";

    if (tupleEqual(result, expected)) {
        cout << "PASS";
    } else {
        cout << "FAIL - got ";
        for (double value : result) {
            cout << value << " ";
        }
    }

    cout << endl;
}

// Write projectile Motion Test 
// Status: Passed 
/* void ppmTest() {
    vector<double> initPos = {0.0, 0.0};    // meters (or units)
    vector<double> velocity = {8.0, 15.0};  // initial v (vx, vy)

    double gravity = -9.8;   // downward
    double wind    = 0.0;    // no horizontal accel
    double dt      = 0.05;   // smaller dt → smoother arc

    Projectile myProj(initPos, velocity, gravity, wind, dt);

    Canvas myCanvas(800, 300);   // wider than tall

    // red color
    vector<double> redColVec = {255, 100, 100};
    Color Red = makeColor(redColVec);

    // map physics units → pixels
    const double scaleX = 10.0;  // 1 unit = 10 pixels horizontally
    const double scaleY = 10.0;  // 1 unit = 10 pixels vertically

    vector<double> currPos = initPos;

    while (currPos[1] >= 0.0) {  // stop when projectile hits the "ground"
        // world → canvas coords
        int canvasX = static_cast<int>(currPos[0] * scaleX);
        int canvasY = myCanvas.height - 1 - static_cast<int>(currPos[1] * scaleY);

        if (canvasX >= 0 && canvasX < myCanvas.width &&
            canvasY >= 0 && canvasY < myCanvas.height) {
            myCanvas.writePixel(canvasX, canvasY, Red);
        }

        currPos = myProj.Tick();
    }

    // write out PPM
    std::string ppm = myCanvas.convertToPpm();
    std::ofstream out("projectile.ppm");
    out << ppm;
    out.close();
} 


// Chaining Matrix Translation Test 
// Status Passed 
void chainingMatrixTransTest() {
    Matrix transform;

Matrix A = transform.rotateX(M_PI / 2);
Matrix B = transform.scale(5, 5, 5);
Matrix C = transform.translation(10, 5, 7);

// Chain: C * B * A
Matrix chain = C.multiplyMatrix(B).multiplyMatrix(A);

vector<double> p = {1, 0, 1, 1};

vector<double> result = chain.multiplyTuple(p);

for (double val : result) {
    cout << round(val * 100000) / 100000 << " ";
}

}


// Represent points as homogeneous coordinates: (x, y, z, w)
// The clock lies flat in the XY plane, so z = 0.
// To spin points around the center of the clock, rotate around the Z axis.
// For Every Hour 
// Matrix rotation = Matrix::rotateZ(angle * hour);
// vector<double> rotated = rotation.multiplyTuple(point);
// Then we convert from math coordinates to canvas pixels 
// int x = canvasWidth / 2 + rotated[0] * radius;
// int y = canvasHeight / 2 + rotated[1] * radius;

// White rgb(255, 255, 255)
void AnalogClockPPM() {
    Canvas canvas(500, 500);

    vector<double> whiteColorVec = {255, 255, 255};
    Color white = makeColor(whiteColorVec);

    double radius = 200.0;
    double angleStep = M_PI / 6.0;

    vector<double> pt = {0, -1, 0, 1};

    for (int hour = 0; hour < 12; hour++) {
        Matrix transform;
        Matrix rm = transform.rotateZ(angleStep * hour);

        vector<double> rotated = rm.multiplyTuple(pt);

        int x = static_cast<int>(canvas.width / 2 + rotated[0] * radius);
        int y = static_cast<int>(canvas.height / 2 + rotated[1] * radius);

        canvas.writePixel(x, y, white);
    }

    ofstream out("analogClock.ppm");

    if (!out) {
        cerr << "Could not create analogClock.ppm" << endl;
        return;
    }

    out << canvas.convertToPpm();
}



// Ray Transform Tests

void runRayTransformTests() {
    Matrix transform;

    Ray r = {
        {1, 2, 3, 1},
        {0, 1, 0, 0}
    };

    Matrix translationMatrix = transform.translation(3, 4, 5);
    Ray translatedRay = r.transform(translationMatrix);

    cout << "Translating a ray: ";

    if (
        tupleEqual(translatedRay.origin, {4, 6, 8, 1}) &&
        tupleEqual(translatedRay.direction, {0, 1, 0, 0})
    ) {
        cout << "PASS";
    } else {
        cout << "FAIL";
    }

    cout << endl;

    Matrix scaleMatrix = transform.scale(2, 3, 4);
    Ray scaledRay = r.transform(scaleMatrix);

    cout << "Scaling a ray: ";

    if (
        tupleEqual(scaledRay.origin, {2, 6, 12, 1}) &&
        tupleEqual(scaledRay.direction, {0, 3, 0, 0})
    ) {
        cout << "PASS";
    } else {
        cout << "FAIL";
    }

    cout << endl;
}

void SphereIntersectionTest() {
    Sphere s;
    Ray r = {
        {0, 0, 5, 1},
        {0, 0, 1, 0}
    };

    vector<double> intersections = s.intersect(r);

    cout << "Intersecting a ray with a sphere: ";

    if (intersections.size() == 2 &&
        almostEqual(intersections[0], -6.0) &&
        almostEqual(intersections[1], -4.0)) {
        cout << "PASS";
    } else {
        cout << "FAIL";
    }

    cout << endl;
}


 void hitTest() {
    Intersections intersections;
    intersections.addIntersection(Intersection(1.0, nullptr));
    intersections.addIntersection(Intersection(2.0, nullptr));
    intersections.addIntersection(Intersection(-1.0, nullptr));

    double hitT = intersections.hit();

    cout << "Testing hit function: ";

    if (almostEqual(hitT, 1.0)) {
        cout << "PASS";
    } else {
        cout << "FAIL - got " << hitT;
    }

    cout << endl;
} 



void TranslateRay() {
        Ray r = {
            {1, 2, 3, 1},
            {0, 1, 0, 0}
        };
    
        Matrix transform;
    
        Matrix translationMatrix = transform.translation(3, 4, 5);
        Ray translatedRay = r.transform(translationMatrix);
    
        cout << "Translating a ray: ";
    
        if (
            tupleEqual(translatedRay.origin, {4, 6, 8, 1}) &&
            tupleEqual(translatedRay.direction, {0, 1, 0, 0})
        ) {
            cout << "PASS";
        } else {
            cout << "FAIL";
        }
    
        cout << endl;
}
 

void IntersectScaledSphereWithRay() {
    Sphere s;
    
    Matrix m; 
    Matrix Scale = m.scale(2, 2, 2);
    s.settransform(Scale); 


    Ray r = {
        {0, 0, -5, 1},
        {0, 0, 1, 0}
    };

    
    vector<double> intersections = s.intersect(r);
    cout << "Intersections with a scaled sphere: ";
    for (double t : intersections) {
        cout << t << " ";
    }

    cout << "Intersecting a scaled sphere with a ray: ";

    if (intersections.size() == 2 &&
        almostEqual(intersections[0], 3.0) &&
        almostEqual(intersections[1], 7.0)) {
        cout << "PASS";
    } else {
        cout << "FAIL";
    }

    cout << endl;
}


// Cast a ray to a sphere and draw a pictures to a canvas 
// Any ray that hits the sphere should result in a red pixel, and any miss shall be drawn as black 



void RaySphereCanvas() {
    const int canvasSize = 200;
    Canvas canvas(canvasSize, canvasSize);

    Sphere s;
    Matrix m; 
    Matrix Scale = m.scale(50, 50, 50);

    s.settransform(Scale);

    // White rgb(255, 255, 255)
    // Red rgb(255, 0, 0)
    vector<double> redColorVec = {255, 0, 0};
    Color red = makeColor(redColorVec);

    for (int x = 0; x < canvas.width; x++) {
        for (int y = 0; y < canvas.height; y++) {
            double rayX = x - canvas.width / 2;
            double rayY = y - canvas.height / 2;
            Ray r = {
                {rayX, rayY, -100, 1},
                {0, 0, 1, 0}
            };

            vector<double> intersections = s.intersect(r);
            if (!intersections.empty()) {
                canvas.writePixel(x, y, red);
            }
        }
    }

    ofstream out("raySphereCanvas.ppm");

    if (!out) {
        cerr << "Could not create raySphereCanvas.ppm" << endl;
        return;
    }

        string ppm = canvas.convertToPpm();

        out << ppm;
        out.close();
}



    // Calculate all the vectors 
void LightShadeVectorTest() {
    cout << "--- Running LightShadeVector Tuple (w) Tests ---\n" << endl;

    // 1. Test CalculateEyeVector
    {
        LightShadeVector lsv;
        vector<double> rayOrigin = {1.0, -2.0, 3.0, 1.0}; // Point
        lsv.CalculateEyeVector(rayOrigin);
        
        assert(lsv.E.size() == 4);
        assert(abs(lsv.E[0] - (-1.0)) < 1e-6);
        assert(abs(lsv.E[1] - 2.0) < 1e-6);
        assert(abs(lsv.E[2] - (-3.0)) < 1e-6);
        // Note: Decide if your NegateTuple flips w. Usually, an eye vector should have w = 0.
        cout << "[PASS] CalculateEyeVector executed successfully." << endl;
    }

    // 2. Test CalculateLightVector
    {
        LightShadeVector lsv;
        vector<double> lightPosition = {0.0, 10.0, 0.0, 1.0}; // Point
        vector<double> pointP        = {0.0, 2.0, 0.0, 1.0};  // Point
        lsv.CalculateLightVector(lightPosition, pointP);
        
        // Point - Point = Vector (w = 0)
        assert(lsv.L.size() == 4);
        assert(abs(lsv.L[0] - 0.0) < 1e-6);
        assert(abs(lsv.L[1] - 8.0) < 1e-6);
        assert(abs(lsv.L[2] - 0.0) < 1e-6);
        assert(abs(lsv.L[3] - 0.0) < 1e-6); // Verifying w component conversion
        cout << "[PASS] CalculateLightVector creates a clean vector (w=0)." << endl;
    }

    // 3. Test CalculateNormalVector (Untransformed Sphere)
    {
        LightShadeVector lsv;
        Sphere s; 
        vector<double> pointP = {0.0, 1.0, 0.0, 1.0}; // Point on top of sphere
        lsv.CalculateNormalVector(pointP, s);
        
        assert(lsv.N.size() == 4);
        assert(abs(lsv.N[0] - 0.0) < 1e-6);
        assert(abs(lsv.N[1] - 1.0) < 1e-6);
        assert(abs(lsv.N[2] - 0.0) < 1e-6);
        assert(abs(lsv.N[3] - 0.0) < 1e-6); // Normal must have w = 0
        cout << "[PASS] CalculateNormalVector correctly sanitizes and calculates normal vector." << endl;
    }

    // 4. Test CalculateReflectionVector
    {
        LightShadeVector lsv;
        // Inbound light vector pointing up and right
        vector<double> L = {1.0, 1.0, 0.0, 0.0}; 
        vector<double> N = {0.0, 1.0, 0.0, 0.0};  // Normal straight up
        lsv.CalculateReflectionVector(L, N);
        
        // R = 2*(1)*{0,1,0,0} - {1,1,0,0} = {-1, 1, 0, 0}
        assert(lsv.R.size() == 4);
        assert(abs(lsv.R[0] - (-1.0)) < 1e-6);
        assert(abs(lsv.R[1] - 1.0) < 1e-6);
        assert(abs(lsv.R[2] - 0.0) < 1e-6);
        assert(abs(lsv.R[3] - 0.0) < 1e-6); // Reflection must have w = 0
        cout << "[PASS] CalculateReflectionVector correctly computes reflection trajectory." << endl;
    }

    cout << "\n--- All 4D Tuple Validation Tests Passed! ---" << endl;
}



// IN FRONT
void LightingTestOne() {
    // 1. Set up the local vector block
    LightShadeVector lsv; 
    std::vector<double> pt = {0, 0, 0, 1}; 
    lsv.E = {0, 0, -1, 0}; 
    lsv.N = {0, 0, -1, 0}; 

    // 2. Instantiate objects
    Color someColor(1, 1, 1);
    PointLight ptLight({0, 0, -10, 1}, someColor);
    Material mat; 

    // 3. Process passing the light reference and localized vectors
    Lighting lighting(ptLight); 
    Color result = lighting.ProcessLighting(mat, lsv, pt); 

    string testString = "Should be {1.9, 1.9, 1.9}"; 
    PrintColor(testString, result); 
}





    // EYE AT 45 DEGREE ANGLE 
    void LightingTestTwo() {
        std::vector<double> pt = {0, 0, 0, 1}; 


        LightShadeVector lsv; 
        lsv.E = {0, sqrt(2)/ 2, -sqrt(2)/2, 0}; 
        lsv.N = {0, 0, -1, 0}; 

        Color someCol{1, 1, 1};
        PointLight ptLight({0, 0, -10, 1}, someCol); 

        Lighting lighting(ptLight); 
        Material mat; 

        Color result = lighting.ProcessLighting(mat, lsv, pt); 

        string testString = "Should be {1.0, 1.0, 1.0}"; 
        PrintColor(testString, result); 

    }


// TEST BEHIND THE SURFACE 
void LightingTestThree() {
     std::vector<double> pt = {0, 0, 0, 1}; 


        LightShadeVector lsv; 
        lsv.E = {0, 0, -1, 0}; 
        lsv.N = {0, 0, -1, 0}; 

        Color someCol{1, 1, 1};
        PointLight ptLight({0, 0, 10, 1}, someCol); 

        Lighting lighting(ptLight); 
        Material mat; 

        Color result = lighting.ProcessLighting(mat, lsv, pt); 

        string testString = "Should be {0.1, 0.1, 0.1}"; 
        PrintColor(testString, result); 

}



void lightingTestPpmRender() {
    const int canvasSize = 200;
    Canvas canvas(canvasSize, canvasSize);

    // 1. Create your Sphere as a generic Shape pointer to test your modularity
    Shape* s = new Sphere(); 
    Matrix m; 
    
    // Position the sphere at the center of the world, scaling it up to size 50
    Matrix Scale = m.scale(50, 50, 50);
    s->setTransform(Scale); // Fixed casing to match your refactored Shape.h

    // Give your sphere an ORANGE material color
    Color orange(1.0, 0.5, 0.0); // 100% Red, 50% Green, 0% Blue makes Orange
    s->setMaterialColor(orange);

    // Setup your light source in the scene
    Color lightColor(1.0, 1.0, 1.0); // Crisp white light
    PointLight ptLight({-60.0, 40.0, -50.0, 1.0}, lightColor); 
    Lighting lightingSystem(ptLight);

    // Ray origin z is at -100, traveling straight forward along +Z axis
    for (int y = 0; y < canvas.height; y++) {
        for (int x = 0; x < canvas.width; x++) {

            // Centering the coordinates on the screen canvas
            double rayX = x - canvas.width / 2.0;
            double rayY = (canvas.height / 2.0) - y; 
            
            Ray r = {
                {rayX, rayY, -100, 1}, // Origin
                {0, 0, 1, 0}           // Direction vector
            };

            // 2. Instantiate your modular Intersections collection container
            Intersections sceneIntersections;

            // 3. Pass the list into your shape to populate it
            s->intersect(r, sceneIntersections);

            // 4. Use your hit() function to find the true visible surface distance
            double closestT = sceneIntersections.hit();

            // hit() returns -1.0 if there are no valid positive intersections
            if (closestT > 0.0) {
                
                // Calculate the exact 3D point in world space where the ray hit
                vector<double> hitPoint = {
                    r.origin[0] + closestT * r.direction[0],
                    r.origin[1] + closestT * r.direction[1],
                    r.origin[2] + closestT * r.direction[2],
                    1.0 
                };

                // Initialize your shading vector block
                LightShadeVector lsv;
                lsv.CalculateEyeVector(r.direction); 
                
                // Pass the object pointer via (*s) to match your calculations
                lsv.CalculateNormalVector(hitPoint, *s); 

                // Run lighting math directly off the base shape's material properties
                Color shadedColor = lightingSystem.ProcessLighting(s->getMaterial(), lsv, hitPoint);

                // Write the final calculated color onto your rendering grid
                canvas.writePixel(x, y, shadedColor);
            } else {
                // Background color (Midnight Dark Blue/Black)
                canvas.writePixel(x, y, Color(0.05, 0.05, 0.1)); 
            }
        }
    }

    // Free memory since we used 'new' for polymorphism
    delete s; 

    ofstream out("raySphereCanvas.ppm");
    if (!out) {
        cerr << "Could not create raySphereCanvas.ppm" << endl;
        return;
    }

    string ppm = canvas.convertToPpm();
    out << ppm;
    out.close();
    cout << "Render complete! raySphereCanvas.ppm generated successfully with an orange sphere.\n";
}



// TODO: Not quite right tests are not passing
// Need Output {4, 4.5, 5.5, 6}
void defaultWorldTest() {
    
    PointLight* ptLight = new PointLight({-10, 10, -10, 1}, Color({1, 1, 1})); 

    Shape* s1 = new Sphere(); 
    // Configure S1 (Sphere 1)
    s1->setMaterialColor(Color{0.8, 1.0, 0.6});  
    s1->setDiffuse(0.7); 
    s1->setSpecular(0.2); 

    Shape *s2 = new Sphere(); 
    // Configure S2 (Sphere 2)
    Matrix m; 
    Matrix scale = m.scale(0.5, 0.5, 0.5);
    s2->setTransform(scale); 

    // Setup default world 
    World* world = new World();

    // Add the shapes
    world->AddShape(s1); 
    world->AddShape(s2); 

    // Now Instanstiate the lighting, and add it to the world 
    // Lighting expects a PointLight object (not a pointer), so dereference
    Lighting* lighting = new Lighting(*ptLight); 
    world->addLighting(*lighting);

    // Create the ray get all intersections from the world
    Ray ray({0, 0, -5, 1}, {0, 0, 1, 0}); 
    Intersections intersectionList; 
    intersectionList = world->intersect_world(ray);
    
    cout << "Printing List: "; 
    intersectionList.print(); 
    
    delete world; 
}




void ComputationsTestOutside() {
    Ray ray({0, 0, -5, 1}, {0, 0, 1, 0}); 

    Shape* sphere = new Sphere(); 

    Intersection intersection(4, sphere);

    Computations comp; 

   comp = prepareComputations(intersection, ray);


   comp.print(); 

}


void ComputationsTestInside() {
    Ray ray({0, 0, 0, 1}, {0, 0, 1, 0}); 

    Shape* sphere = new Sphere(); 

    Intersection intersection(1, sphere);

    Computations comp; 
    comp = prepareComputations(intersection, ray);

    comp.print();
}



void ComputationsTestInside() {
    Ray ray({0, 0, 0, 1}, {0, 0, 1, 0}); 

    Shape* sphere = new Sphere(); 

    Intersection intersection(1, sphere);

    Computations comp = prepareComputations(intersection, ray);

    comp.print();

    delete sphere;
}


bool colorEqual(const Color& a, const Color& b) {
    return almostEqual(a.r, b.r) &&
           almostEqual(a.g, b.g) &&
           almostEqual(a.b, b.b);
}

void printShadeTestResult(const string& testName, const Color& result, const Color& expected) {
    cout << testName << ": ";

    if (colorEqual(result, expected)) {
        cout << "PASS\n";
    } else {
        cout << "FAIL\n";
        cout << "Expected: ";
        PrintColor("", expected);
        cout << "Got: ";
        PrintColor("", result);
    }
}

void ShadeHitTestOutside() {
    PointLight pointLight({-10, 10, -10, 1}, Color(1, 1, 1));
    Lighting lighting(pointLight);

    Shape* s1 = new Sphere();
    s1->setMaterialColor(Color(0.8, 1.0, 0.6));
    s1->setDiffuse(0.7);
    s1->setSpecular(0.2);

    Shape* s2 = new Sphere();
    Matrix m;
    Matrix scale = m.scale(0.5, 0.5, 0.5);
    s2->setTransform(scale);

    World* world = new World();
    world->AddShape(s1);
    world->AddShape(s2);
    world->addLighting(lighting);

    Ray ray({0, 0, -5, 1}, {0, 0, 1, 0});
    Intersection intersection(4, s1);

    Computations comps = prepareComputations(intersection, ray);
    Color result = world->shade_hit(comps);

    Color expected(0.38066, 0.47583, 0.2855);
    printShadeTestResult("ShadeHitTestOutside", result, expected);

    delete world;
}

void ShadeHitTestInside() {
    PointLight pointLight({0, 0.25, 0, 1}, Color(1, 1, 1));
    Lighting lighting(pointLight);

    Shape* s1 = new Sphere();
    s1->setMaterialColor(Color(0.8, 1.0, 0.6));
    s1->setDiffuse(0.7);
    s1->setSpecular(0.2);

    Shape* s2 = new Sphere();
    Matrix m;
    Matrix scale = m.scale(0.5, 0.5, 0.5);
    s2->setTransform(scale);

    World* world = new World();
    world->AddShape(s1);
    world->AddShape(s2);
    world->addLighting(lighting);

    Ray ray({0, 0, 0, 1}, {0, 0, 1, 0});
    Intersection intersection(0.5, s2);

    Computations comps = prepareComputations(intersection, ray);
    Color result = world->shade_hit(comps);

    Color expected(0.90498, 0.90498, 0.90498);
    printShadeTestResult("ShadeHitTestInside", result, expected);

    delete world;
}
    

// The ray fails to intersect anything and should return black
void Color_AtTest1() {
    World* world = new World(); 

    Ray r({0, 0, -5, 1}, {0, 0, 1, 0}); 

    Color c = world->Color_at(r);
    string label = "Should be {0, 0, 0}";
    PrintColor(label, c); 
}


// Expect Color_at() to use hit when computing the color. Ray inside an outer sphere, but outside the inner sphere,
// and poiting at the inner sphere
// We expect the hit to be on the inner sphere, and thus return its color
void Color_atTest2() {
     World* world = new World(); // Should initalize default world 
     Shape* s1 = new Sphere(); 
     s1->setAmbient(1);
     world->AddShape(s1);
     Shape* s2 = new Sphere(); 
     s2->setAmbient(1);
     world->AddShape(s2);

    Ray r({0, 0, 0.75, 1}, {0, 0, -1, 0}); 

    Color c = world->Color_at(r);
    string label = "Output";
    PrintColor(label, c); 
    Color res = s2->getMaterialColor();
    cout << "Should be: " << endl; 
     PrintColor(label, res);


     delete world; 
     delete s1; 
     delete s2; 
}





void viewTransformTest() {
    vector<double> from = {1, 3, 2, 1}; 
    vector<double> to = {4, -2, 8, 1}; 
    vector<double> up = {1, 1, 0, 0}; 

    Matrix m; 
    Matrix viewT = m.viewTransformation(from, to, up); 

    viewT.printMatrix(); 
}


void  rayPixelTest() {
    Camera c(201, 101, M_PI / 2); 
    Matrix m; 
    Matrix rot = m.rotateY(M_PI / 4); 
    Matrix trans = m.translation(0, -2, 5); 
    c.setTransformM(rot.multiplyMatrix(trans));

    Ray r = ray_for_pixel(c, 100, 50); 
   // c.print();
    r.printRay(); 
}



void NormalOnTranslatedSphereTest() {
    Sphere s;
    Matrix m;

    s.setTransform(m.translation(0, 1, 0));

    vector<double> n = s.normal_at({0, 1.70711, -0.70711, 1});

    cout << "NormalOnTranslatedSphereTest: ";
    if (tupleEqual(n, {0, 0.70711, -0.70711, 0})) {
        cout << "PASS\n";
    } else {
        cout << "FAIL - got ";
        PrintTuple(n);
    }
}

void NormalOnTransformedSphereTest() {
    Sphere s;
    Matrix m;

    Matrix scale = m.scale(1, 0.5, 1);
    Matrix rotate = m.rotateZ(M_PI / 5);
    Matrix transform = scale.multiplyMatrix(rotate);

    s.setTransform(transform);

    vector<double> n = s.normal_at({0, sqrt(2) / 2, -sqrt(2) / 2, 1});

    cout << "NormalOnTransformedSphereTest: ";
    if (tupleEqual(n, {0, 0.97014, -0.24254, 0})) {
        cout << "PASS\n";
    } else {
        cout << "FAIL - got ";
        PrintTuple(n);
    }
}

void MultiSpherereRender() {
    Matrix m; // Identity Matrix initialized 

    // Create the world tracking instance
    World* world = new World(); 

    // 2. Flattened sphere on the bottom (FLOOR)
    Shape* floor = new Sphere; 
    floor->setTransform(m.scale(10, 0.01, 10)); 
    floor->setMaterialColor(Color{1, 0.9, 0.9}); 
    floor->setSpecular(0); 
    world->AddShape(floor);

    // Common transformation sub-matrices for the walls
    Matrix trans = m.translation(0, 0, 5); 
    Matrix rotX = m.rotateX(M_PI / 2);  
    Matrix scaling = m.scale(10, 0.01, 10); 

    // 3. Wall on the left 
    // Left-to-right application order: Translate -> Rotate Y -> Rotate X -> Scale
    Shape* lwall = new Sphere; 
    Matrix rotY = m.rotateY(-M_PI / 4); 
    Matrix finalTrans = trans.multiplyMatrix(rotY).multiplyMatrix(rotX).multiplyMatrix(scaling);
    lwall->setTransform(finalTrans);
    lwall->setMaterialColor(Color{1, 0.9, 0.9}); 
    lwall->setSpecular(0);
    world->AddShape(lwall);

    // 4. Wall on the right
    Shape* rwall = new Sphere; 
    Matrix rotYR = m.rotateY(M_PI / 4); 
    Matrix finalTransR = trans.multiplyMatrix(rotYR).multiplyMatrix(rotX).multiplyMatrix(scaling);
    rwall->setTransform(finalTransR);
    rwall->setMaterialColor(Color{1, 0.9, 0.9}); 
    rwall->setSpecular(0);
    world->AddShape(rwall);

    // 5. Large sphere in the middle 
    Shape* middle = new Sphere; 
    Matrix transMid = m.translation(-0.5, 1, 0.5); 
    middle->setTransform(transMid); 
    middle->setMaterialColor(Color{0.1, 1, 0.5}); 
    middle->setDiffuse(0.7); 
    middle->setSpecular(0.3); 
    world->AddShape(middle);

    // 6. Smaller green Sphere on the right 
    Shape* right = new Sphere; 
    Matrix transRight = m.translation(1.5, 0.5, -0.5); 
    Matrix scaleRight = m.scale(0.5, 0.5, 0.5); 
    Matrix finalRight = transRight.multiplyMatrix(scaleRight); // Translate -> Scale
    right->setTransform(finalRight); 
    right->setMaterialColor(Color{0.5, 1, 0.1}); 
    right->setDiffuse(0.7); 
    right->setSpecular(0.3); 
    world->AddShape(right);

    // 7. Smallest sphere on the left
    Shape* left = new Sphere; 
    Matrix transLeft = m.translation(-1.5, 0.33, -0.75);
    Matrix scaleLeft = m.scale(0.33, 0.33, 0.33); 
    Matrix finalleft = transLeft.multiplyMatrix(scaleLeft); // Fixed original typo: Translate -> Scale
    left->setTransform(finalleft); 
    left->setMaterialColor(Color{1, 0.8, 0.1});
    left->setDiffuse(0.7); 
    left->setSpecular(0.3); 
    world->AddShape(left); 

    // 8. Camera Configuration
    Camera cam(100, 50, M_PI / 3); 
    std::vector<double> from = {0.0, 1.5, -5.0, 1.0}; 
    std::vector<double> to   = {0.0, 1.0,  0.0, 1.0};
    std::vector<double> up   = {0.0, 1.0,  0.0, 0.0}; 
    Matrix viewTrans = m.viewTransformation(from, to, up); 
    cam.setTransformM(viewTrans);

    // 9. Execution and Frame Flush
    Canvas canvas = render(cam, *world);
    canvas.canvasOut(); 

    // Clean up memory
    delete world;

}


void PlaneRenderTest() {
    Matrix m; // Identity Matrix initialized 

    // Create the world tracking instance
    World* world = new World(); 

    // 1. Add a Light Source (Assuming your world needs a light set up)
    // PointLight light(Color{1, 1, 1}, {-10, 10, -10, 1});
    // world->SetLight(light);

    // 2. TRUE FLOOR (Using your new Plane class)
    // No flattening scale or rotation needed! A plane is infinitely flat at y = 0.
    Plane* floor = new Plane(); 
    floor->setMaterialColor(Color{0.2, 0.3, 0.5}); // Nice blue-grey floor
    floor->setSpecular(0.1); 
    // You can leave the transform as Identity, or shift it slightly down if wanted
    // floor->setTransform(m.translation(0, 0, 0)); 
    world->AddShape(floor);

    // 3. TRUE BACK WALL (Using your new Plane class)
    // We just take an infinite plane and rotate it 90 degrees around X, 
    // then push it back along Z to make it a vertical wall.
    Plane* backWall = new Plane();
    Matrix wallTrans = m.translation(0, 0, 5);
    Matrix wallRotX = m.rotateX(M_PI / 2); // Rotate flat plane vertical
    Matrix finalWallTransform = wallTrans.multiplyMatrix(wallRotX);
    backWall->setTransform(finalWallTransform);
    backWall->setMaterialColor(Color{0.8, 0.8, 0.8}); // Light grey wall
    backWall->setSpecular(0);
    world->AddShape(backWall);

    // 4. Large test sphere floating slightly above the floor
    Shape* middle = new Sphere(); 
    Matrix transMid = m.translation(0, 1, 1); 
    middle->setTransform(transMid); 
    middle->setMaterialColor(Color{1, 0.2, 0.2}); // Bright red sphere
    middle->setDiffuse(0.7); 
    middle->setSpecular(0.3); 
    world->AddShape(middle);

    // 5. Camera Configuration
    // Low resolution (100x50) for a fast test print
    Camera cam(100, 50, M_PI / 3); 
    std::vector<double> from = {0.0, 1.5, -5.0, 1.0}; // Elevated view
    std::vector<double> to   = {0.0, 1.0,  0.0, 1.0};  // Looking at the center
    std::vector<double> up   = {0.0, 1.0,  0.0, 0.0}; 
    Matrix viewTrans = m.viewTransformation(from, to, up); 
    cam.setTransformM(viewTrans);

    // 6. Execution and Frame Flush
    Canvas canvas = render(cam, *world);
    canvas.canvasOut(); 

    // Clean up memory
    delete world;
}




void PatternRenderTest() {
    std::cout << "[DEBUG] Starting PatternRenderTest..." << std::endl;
    Matrix m; 

    std::cout << "[DEBUG] Initializing World and Lighting..." << std::endl;
    PointLight light({-10.0, 10.0, -10.0, 1.0}, Color{1, 1, 1});
    Lighting lighting(light);
    World* world = new World(lighting);

    // 1. FLOOR WITH CHECKERS
    std::cout << "[DEBUG] Setting up 1. Floor with Checkers..." << std::endl;
    Shape* floor = new Plane();
    auto floorCheckers = std::make_shared<CheckersPattern>(
        Color{0.1, 0.1, 0.1},
        Color{0.9, 0.9, 0.9}
    );
    floorCheckers->transform = m.scale(2.0, 2.0, 2.0);
    floor->setMaterialPattern(floorCheckers);
    floor->setAmbient(0.1);
    floor->setDiffuse(0.7);
    floor->setSpecular(0.1);
    world->AddShape(floor);

    // 2. BACK WALL WITH PERTURBED STRIPES
    std::cout << "[DEBUG] Setting up 2. Back Wall with Perturbed Stripes..." << std::endl;
    Shape* backWall = new Plane();
    Matrix wallTrans = m.translation(0, 0, 5);
    Matrix wallRotX = m.rotateX(M_PI / 2);
    backWall->setTransform(wallTrans.multiplyMatrix(wallRotX));

    auto wallStripesBase = std::make_shared<StripePattern>(
        Color{0.8, 0.1, 0.1},
        Color{0.9, 0.8, 0.1}
    );
    wallStripesBase->transform = m.scale(0.5, 1.0, 1.0);

    auto perturbedWall = std::make_shared<PertubedPattern>(
        wallStripesBase, 
        0.35, 
        1.5   
    );
    backWall->setMaterialPattern(perturbedWall);
    backWall->setAmbient(0.1);
    backWall->setDiffuse(0.8);
    backWall->setSpecular(0.0);
    world->AddShape(backWall);

    // 3. LEFT SPHERE WITH GRADIENT
    std::cout << "[DEBUG] Setting up 3. Left Sphere with Gradient..." << std::endl;
    Shape* leftSphere = new Sphere();
    Matrix leftSphereTrans = m.translation(-1.7, 1.0, 0.5);
    Matrix leftSphereScale = m.scale(1.0, 1.0, 1.0);
    leftSphere->setTransform(leftSphereTrans.multiplyMatrix(leftSphereScale));

    auto sphereGradient = std::make_shared<GradientPattern>(
        Color{1.0, 0.0, 0.0},
        Color{0.0, 0.0, 1.0}
    );
    sphereGradient->transform = m.scale(1.0, 1.0, 1.0);
    leftSphere->setMaterialPattern(sphereGradient);
    leftSphere->setAmbient(0.1);
    leftSphere->setDiffuse(0.8);
    leftSphere->setSpecular(0.2);
    leftSphere->setShininess(100);
    world->AddShape(leftSphere);

    // 4. MIDDLE SPHERE WITH PERTURBED RINGS
    std::cout << "[DEBUG] Setting up 4. Middle Sphere with Perturbed Rings..." << std::endl;
    Shape* middleSphere = new Sphere();
    middleSphere->setTransform(m.translation(0, 1.5, 1));

    auto sphereRingsBase = std::make_shared<RingPattern>(
        Color{0.1, 0.8, 0.2},
        Color{0.1, 0.2, 0.8}
    );
    sphereRingsBase->transform = m.scale(0.35, 0.35, 0.35);

    auto perturbedRings = std::make_shared<PertubedPattern>(
        sphereRingsBase,
        0.25, 
        3.0   
    );
    middleSphere->setMaterialPattern(perturbedRings);
    middleSphere->setAmbient(0.1);
    middleSphere->setDiffuse(0.8);
    middleSphere->setSpecular(0.2);
    middleSphere->setShininess(100);
    world->AddShape(middleSphere);

    // 5. RIGHT SPHERE WITH STRIPES
    std::cout << "[DEBUG] Setting up 5. Right Sphere with Stripes..." << std::endl;
    Shape* rightSphere = new Sphere();
    Matrix rightSphereTrans = m.translation(1.7, 1.0, 0.5);
    Matrix rightSphereScale = m.scale(1.0, 1.0, 1.0);
    rightSphere->setTransform(rightSphereTrans.multiplyMatrix(rightSphereScale));

    auto sphereStripes = std::make_shared<StripePattern>(
        Color{0.9, 0.9, 0.9},
        Color{0.05, 0.05, 0.05}
    );
    sphereStripes->transform = m.scale(0.35, 1.0, 1.0);
    rightSphere->setMaterialPattern(sphereStripes);
    rightSphere->setAmbient(0.1);
    rightSphere->setDiffuse(0.8);
    rightSphere->setSpecular(0.2);
    rightSphere->setShininess(100);
    world->AddShape(rightSphere);

    // 6. CAMERA & RENDERING EXECUTION
    std::cout << "[DEBUG] Configuring Camera view transformation..." << std::endl;
    Camera cam(800, 400, M_PI / 3);
    std::vector<double> from = {0.0, 2.5, -5.0, 1.0};
    std::vector<double> to   = {0.0, 1.0,  0.0, 1.0};
    std::vector<double> up   = {0.0, 1.0,  0.0, 0.0};

    Matrix viewTrans = m.viewTransformation(from, to, up);
    cam.setTransformM(viewTrans);

    std::cout << "[DEBUG] ENTERING RENDER LOOP (Shooting rays)..." << std::endl; // This is where the problem occurs 
    Canvas canvas = render(cam, *world);
    std::cout << "[DEBUG] EXITING RENDER LOOP (Render finished successfully!)." << std::endl;

    std::cout << "[DEBUG] Outputting Canvas image payload..." << std::endl;
    canvas.canvasOut();

    std::cout << "[DEBUG] Deallocating World memory..." << std::endl;
    delete world; 
    std::cout << "[DEBUG] PatternRenderTest completed cleanly with no crashes!" << std::endl;
}



// Pre computing reflection vector to test 
// Reflection vector should printout and be (0, sqrt(2)/2, sqrt(2)/2, 1)
void reflectionVectorPreComputeTest() {
    Plane plane;

    Ray ray(
        {0, 1, -1, 1},
        {0, -sqrt(2) / 2, sqrt(2) / 2, 0}
    );

    Intersection intersect(sqrt(2), &plane);
    Computations comps = prepareComputations(intersect, ray);

    vector<double> expected = {
        0,
        sqrt(2) / 2,
        sqrt(2) / 2,
        0
    };

    cout << "Reflection vector test: "
         << (tupleEqual(comps.reflectv, expected) ? "PASS" : "FAIL")
         << endl;

    comps.print();
}

void transparencyMaterialTest() {
    Material material;

    bool defaultsPass =
        almostEqual(material.transparency, 0.0) &&
        almostEqual(material.refractiveIndex, 1.0);

    Sphere glassSphere;
    glassSphere.setTransparency(1.0);
    glassSphere.setRefractiveIndex(1.5);

    const Material& glassMaterial = glassSphere.getMaterial();

    bool glassPass =
        almostEqual(glassMaterial.transparency, 1.0) &&
        almostEqual(glassMaterial.refractiveIndex, 1.5);

    cout << "Default transparency material test: "
         << (defaultsPass ? "PASS" : "FAIL")
         << endl;

    cout << "Glass material test: "
         << (glassPass ? "PASS" : "FAIL")
         << endl;
}

void refractiveIndicesTest() {
    Matrix matrix;

    // Sphere A: large outer glass sphere
    Sphere sphereA;
    sphereA.setTransform(matrix.scale(2, 2, 2));
    sphereA.setTransparency(1.0);
    sphereA.setRefractiveIndex(1.5);

    // Sphere B: shifted slightly backward
    Sphere sphereB;
    sphereB.setTransform(matrix.translation(0, 0, -0.25));
    sphereB.setTransparency(1.0);
    sphereB.setRefractiveIndex(2.0);

    // Sphere C: shifted slightly forward
    Sphere sphereC;
    sphereC.setTransform(matrix.translation(0, 0, 0.25));
    sphereC.setTransparency(1.0);
    sphereC.setRefractiveIndex(2.5);

    // Ray travels forward through all three spheres
    Ray ray(
        {0, 0, -4, 1},
        {0, 0, 1, 0}
    );

    Intersections intersections;

    intersections.addIntersection(
        Intersection(2.0, &sphereA)
    );

    intersections.addIntersection(
        Intersection(2.75, &sphereB)
    );

    intersections.addIntersection(
        Intersection(3.25, &sphereC)
    );

    intersections.addIntersection(
        Intersection(4.75, &sphereB)
    );

    intersections.addIntersection(
        Intersection(5.25, &sphereC)
    );

    intersections.addIntersection(
        Intersection(6.0, &sphereA)
    );

    intersections.Sort();

    // Expected n1 and n2 for each intersection
    const vector<pair<double, double>> expected = {
        {1.0, 1.5},
        {1.5, 2.0},
        {2.0, 2.5},
        {2.5, 2.5},
        {2.5, 1.5},
        {1.5, 1.0}
    };

    const vector<Intersection>& allIntersections =
        intersections.getIntersections();

    bool passed = true;

    for (size_t i = 0; i < allIntersections.size(); i++) {
        Computations comps = prepareComputations(
            allIntersections[i],
            ray,
            intersections
        );

        bool n1Correct =
            almostEqual(comps.n1, expected[i].first);

        bool n2Correct =
            almostEqual(comps.n2, expected[i].second);

        if (!n1Correct || !n2Correct) {
            passed = false;

            cout << "Intersection " << i
                 << " FAILED: expected n1="
                 << expected[i].first
                 << ", n2="
                 << expected[i].second
                 << " but received n1="
                 << comps.n1
                 << ", n2="
                 << comps.n2
                 << "\n";
        }
    }

    cout << "Nested refractive indices test: "
         << (passed ? "PASS" : "FAIL")
         << "\n";
}


void refractedColorTests() {
    const Color black{0, 0, 0};

    PointLight light(
        {-10, 10, -10, 1},
        Color{1, 1, 1}
    );

    Lighting lighting(light);

    // ---------------------------------------------------------
    // Test 1: An opaque material should not refract any color
    // ---------------------------------------------------------
    {
        World world(lighting);

        Sphere sphere;

        Ray ray(
            {0, 0, -5, 1},
            {0, 0, 1, 0}
        );

        Intersection intersection(4.0, &sphere);

        Computations comps =
            prepareComputations(intersection, ray);

        Color result =
            world.refracted_color(comps, 5);

        cout << "Opaque material refracted color test: "
             << (colorEqual(result, black) ? "PASS" : "FAIL")
             << "\n";
    }

    // ---------------------------------------------------------
    // Test 2: Refraction should stop when recursion reaches zero
    // ---------------------------------------------------------
    {
        World world(lighting);

        Sphere glassSphere;
        glassSphere.setTransparency(1.0);
        glassSphere.setRefractiveIndex(1.5);

        Ray ray(
            {0, 0, -5, 1},
            {0, 0, 1, 0}
        );

        Intersection intersection(4.0, &glassSphere);

        Computations comps =
            prepareComputations(intersection, ray);

        Color result =
            world.refracted_color(comps, 0);

        cout << "Refraction recursion limit test: "
             << (colorEqual(result, black) ? "PASS" : "FAIL")
             << "\n";
    }

    // ---------------------------------------------------------
    // Test 3: Total internal reflection should return black
    // ---------------------------------------------------------
    {
        World world(lighting);

        Sphere glassSphere;
        glassSphere.setTransparency(1.0);
        glassSphere.setRefractiveIndex(1.5);

        Ray ray(
            {0, 0, sqrt(2) / 2, 1},
            {0, 1, 0, 0}
        );

        Intersections intersections;

        intersections.addIntersection(
            Intersection(-sqrt(2) / 2, &glassSphere)
        );

        intersections.addIntersection(
            Intersection(sqrt(2) / 2, &glassSphere)
        );

        intersections.Sort();

        const vector<Intersection>& all =
            intersections.getIntersections();

        Computations comps =
            prepareComputations(
                all[1],
                ray,
                intersections
            );

        Color result =
            world.refracted_color(comps, 5);

        cout << "Total internal reflection test: "
             << (colorEqual(result, black) ? "PASS" : "FAIL")
             << "\n";
    }

    // ---------------------------------------------------------
    // Test 4: A transparent plane should reveal the color behind it
    // ---------------------------------------------------------
    {
        World world(lighting);
        Matrix matrix;

        Plane* glassPlane = new Plane();
        glassPlane->setTransparency(0.5);
        glassPlane->setRefractiveIndex(1.5);

        world.AddShape(glassPlane);

        Sphere* coloredSphere = new Sphere();

        coloredSphere->setTransform(
            matrix.translation(0, -2, 0)
        );

        coloredSphere->setMaterialColor(
            Color{0.2, 0.4, 0.6}
        );

        // Make the sphere's resulting color predictable
        coloredSphere->setAmbient(1.0);
        coloredSphere->setDiffuse(0.0);
        coloredSphere->setSpecular(0.0);

        world.AddShape(coloredSphere);

        Ray ray(
            {0, 1, 0, 1},
            {0, -1, 0, 0}
        );

        Intersection intersection(1.0, glassPlane);

        Intersections intersections;
        intersections.addIntersection(intersection);

        Computations comps =
            prepareComputations(
                intersection,
                ray,
                intersections
            );

        Color result =
            world.refracted_color(comps, 5);

        // Sphere color multiplied by plane transparency:
        // {0.2, 0.4, 0.6} * 0.5
        Color expected{0.1, 0.2, 0.3};

        cout << "Refracted color through surface test: "
             << (colorEqual(result, expected) ? "PASS" : "FAIL")
             << "\n";

        if (!colorEqual(result, expected)) {
            cout << "Expected: "
                 << expected.r << " "
                 << expected.g << " "
                 << expected.b << "\n";

            cout << "Received: "
                 << result.r << " "
                 << result.g << " "
                 << result.b << "\n";
        }
    }
}





void SchlickApproximationTest() {
    Shape* glassSphere = new Sphere;

    glassSphere->setTransparency(1.0);
    glassSphere->setRefractiveIndex(1.5);

    Ray ray(
        {0, 0, sqrt(2) / 2, 1},
        {0, 1, 0, 0}
    );

    Intersection intA(-sqrt(2) / 2, glassSphere);
    Intersection intB(sqrt(2) / 2, glassSphere);

    Intersections ints;
    ints.addIntersection(intA);
    ints.addIntersection(intB);

    Computations comps = prepareComputations(intB, ray, ints);

    double reflectanceResult = schlick(comps);

    cout << "Result: " << reflectanceResult << endl;

    delete glassSphere;
}

*/


void ReflectionRefractionSceneTest() {
    std::cout << "[DEBUG] Starting ReflectionRefractionSceneTest..." << std::endl;

    Matrix m;

    PointLight light(
        {-10.0, 10.0, -10.0, 1.0},
        Color{1.0, 1.0, 1.0}
    );

    Lighting lighting(light);
    World* world = new World(lighting);

    // ---------------------------------------------------------
    // 1. Reflective + transparent floor
    // ---------------------------------------------------------
    Shape* floor = new Plane();

    floor->setMaterialColor(Color{1.0, 1.0, 1.0});
    floor->setAmbient(0.05);
    floor->setDiffuse(0.4);
    floor->setSpecular(0.2);
    floor->setShininess(100);

    floor->setReflective(0.5);
    floor->setTransparency(0.5);
    floor->setRefractiveIndex(1.5);

    world->AddShape(floor);

    // ---------------------------------------------------------
    // 2. Red sphere below the floor
    // This should be visible through the transparent floor.
    // ---------------------------------------------------------
    Shape* belowSphere = new Sphere();

    Matrix belowSphereTrans = m.translation(0.0, -1.5, 0.0);
    Matrix belowSphereScale = m.scale(0.6, 0.6, 0.6);
    Matrix belowSphereTransform = belowSphereTrans.multiplyMatrix(belowSphereScale);

    belowSphere->setTransform(belowSphereTransform);

    belowSphere->setMaterialColor(Color{1.0, 0.0, 0.0});
    belowSphere->setAmbient(0.5);
    belowSphere->setDiffuse(0.4);
    belowSphere->setSpecular(0.0);

    world->AddShape(belowSphere);

    // ---------------------------------------------------------
    // 3. Blue glass sphere above the floor
    // This tests curved-surface refraction.
    // ---------------------------------------------------------
    Shape* glassSphere = new Sphere();

    Matrix glassSphereTrans = m.translation(-0.9, 1.0, 0.5);
    Matrix glassSphereScale = m.scale(1.0, 1.0, 1.0);
    Matrix glassSphereTransform = glassSphereTrans.multiplyMatrix(glassSphereScale);

    glassSphere->setTransform(glassSphereTransform);

    glassSphere->setMaterialColor(Color{0.7, 0.9, 1.0});
    glassSphere->setAmbient(0.0);
    glassSphere->setDiffuse(0.2);
    glassSphere->setSpecular(0.9);
    glassSphere->setShininess(200);
    glassSphere->setReflective(0.1);
    glassSphere->setTransparency(0.9);
    glassSphere->setRefractiveIndex(1.5);

    world->AddShape(glassSphere);

    // ---------------------------------------------------------
    // 4. Mirror-like sphere above the floor
    // This should strongly reflect the scene.
    // ---------------------------------------------------------
    Shape* mirrorSphere = new Sphere();

    Matrix mirrorSphereTrans = m.translation(1.1, 0.7, -0.6);
    Matrix mirrorSphereScale = m.scale(0.7, 0.7, 0.7);
    Matrix mirrorSphereTransform = mirrorSphereTrans.multiplyMatrix(mirrorSphereScale);

    mirrorSphere->setTransform(mirrorSphereTransform);

    mirrorSphere->setMaterialColor(Color{0.8, 0.8, 0.8});
    mirrorSphere->setAmbient(0.0);
    mirrorSphere->setDiffuse(0.1);
    mirrorSphere->setSpecular(1.0);
    mirrorSphere->setShininess(200);
    mirrorSphere->setReflective(0.9);

    world->AddShape(mirrorSphere);

    // ---------------------------------------------------------
    // 5. Back wall
    // This gives reflections/refractions something visible.
    // ---------------------------------------------------------
    Shape* backWall = new Plane();

    Matrix wallTrans = m.translation(0.0, 0.0, 5.0);
    Matrix wallRotX = m.rotateX(M_PI / 2);
    Matrix wallTransform = wallTrans.multiplyMatrix(wallRotX);

    backWall->setTransform(wallTransform);

    backWall->setMaterialColor(Color{0.8, 0.8, 0.9});
    backWall->setAmbient(0.1);
    backWall->setDiffuse(0.7);
    backWall->setSpecular(0.0);

    world->AddShape(backWall);

    // ---------------------------------------------------------
    // 6. Small green sphere in the background
    // This makes mirror reflection easier to verify.
    // ---------------------------------------------------------
    Shape* greenSphere = new Sphere();

    Matrix greenSphereTrans = m.translation(0.0, 0.5, 3.0);
    Matrix greenSphereScale = m.scale(0.5, 0.5, 0.5);
    Matrix greenSphereTransform = greenSphereTrans.multiplyMatrix(greenSphereScale);

    greenSphere->setTransform(greenSphereTransform);

    greenSphere->setMaterialColor(Color{0.0, 1.0, 0.2});
    greenSphere->setAmbient(0.1);
    greenSphere->setDiffuse(0.7);
    greenSphere->setSpecular(0.3);
    greenSphere->setShininess(100);

    world->AddShape(greenSphere);

    // ---------------------------------------------------------
    // 7. Camera
    // ---------------------------------------------------------
    Camera cam(800, 400, M_PI / 3);

    std::vector<double> from = {0.0, 2.0, -6.0, 1.0};
    std::vector<double> to   = {0.0, 0.5,  0.0, 1.0};
    std::vector<double> up   = {0.0, 1.0,  0.0, 0.0};

    Matrix viewTrans = m.viewTransformation(from, to, up);
    cam.setTransformM(viewTrans);

    std::cout << "[DEBUG] Rendering reflection/refraction scene..." << std::endl;

    Canvas canvas = render(cam, *world);

    std::cout << "[DEBUG] Render finished. Outputting canvas..." << std::endl;

    canvas.canvasOut();

    delete world;

    std::cout << "[DEBUG] ReflectionRefractionSceneTest completed." << std::endl;
}