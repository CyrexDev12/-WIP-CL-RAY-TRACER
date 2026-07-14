#ifndef CAMERA_H
#define CAMERA_H
#include "core/math/Mat4.h"
#include "geometry/Ray.h"
#include "math/Matrix.h"
#define M_PI       3.14159265358979323846   // pi


class Camera {
    private:

    double hSize; // Horizontal Size of the canvas that the picture will be rendered too
    double vSize; // Is the canvas vertical size
    double fov; // Field of View, an angle that describes how much the camera can see (IN RADIANS)
    clrt::math::Mat4 transform; // World-to-camera transform
    clrt::math::Mat4 inverseTransform; // Cached camera-to-world transform
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

    double getPixelSize() const {
        return pixelSize; 
    }

    double getHalfWidth() const {
        return halfWidth;
    }

    double getHalfHeight() const {
        return halfHeight; 
    }

    double gethSize() const {
        return hSize; 
    }

    double getvSize() const {
        return vSize; 
    }

    [[nodiscard]] const clrt::math::Mat4& getTransform() const noexcept;
    [[nodiscard]] const clrt::math::Mat4& getInverseTransform() const noexcept;
    [[nodiscard]] Matrix getTransformM() const;
    void setTransform(const clrt::math::Mat4& matrix);
    void setTransformM(const Matrix& matrix);

    // Debug Functions

    void print(); 
};

// Returns new ray that starts at the camera and passes through the indicated (x, y) pixels on the canvas
Ray ray_for_pixel(const Camera& camera, double x, double y);

#endif
