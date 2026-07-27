#ifndef CAMERA_H
#define CAMERA_H
#include <functional>
#include "math/Matrix.h"
#include "geometry/Ray.h"
#include "scene/World.h"
#include "scene/canvas.h"
#define M_PI       3.14159265358979323846   // pi


class Camera {
    private:

    double hSize; // Horizontal Size of the canvas that the picture will be rendered too
    double vSize; // Is the canvas vertical size
    double fov; // Field of View, an angle that describes how much the camera can see (IN RADIANS)
    Matrix transform; // Matrix describing how the world should be oriented relative to the camera
    double pixelSize;
    double aspect;  // Aspect ratio of the horizontal size of the canvas to its vertical size
    double halfView;  // The width of half the canvas 
    double halfWidth; // Half width 
    double halfHeight; // Half height

    // pixelSize, halfWidth, halfHeight values will be used to create rays that can pass through any given pixel on the canvas
    public: 

    // Constructor 
    Camera(double h, double v, double f); 
    Camera() : Camera(100, 50, M_PI / 3) {
    // Leave the body completely empty
}

    double getPixelSize() {
        return pixelSize; 
    }

    double getHalfWidth() {
        return halfWidth;
    }

    double getHalfHeight() {
        return halfHeight; 
    }

    double gethSize() {
        return hSize; 
    }

    double getvSize() {
        return vSize; 
    }

    Matrix getTransformM() {
        return transform; 
    }

    void setTransformM(Matrix m) {
        transform = m; 
    }

    // Debug Functions

    void print(); 
};

// Returns new ray that starts at the camera and passes through the indicated (x, y) pixels on the canvas
Ray ray_for_pixel(Camera cam, double x, double y); 

// Returns rendered canvas 
using RenderProgressCallback = std::function<void(int)>;
Canvas render(
    Camera cam,
    World& world,
    bool multiThreaded = false,
    RenderProgressCallback progressCallback = {}
);








#endif
