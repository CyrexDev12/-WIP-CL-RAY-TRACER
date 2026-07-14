#include "scene/Camera.h"
#include <iostream>
#include <iomanip>
#include <cmath>

#include "math/LegacyMathAdapters.h"

// Constructor 
Camera::Camera(double h, double v, double f) {
    hSize = h; 
    vSize = v;
    fov = f; 

    halfView = tan(fov / 2); // The width of half the canvas 
    aspect = hSize / vSize;  // Aspect ratio of the horizontal size of the canvas to its vertical size
    
    if (aspect >= 1) {
        // Horizontally dominated 
        // half view is half the width of the canvas, and half view / aspect is half of the canvas's height
        halfWidth = halfView; 
        halfHeight = halfView / aspect; 

    } else {
        // Vertically dominated 
        // half view is instead half the height of the canvas, and half the canvas's width is halfview * aspect
        halfWidth = halfView * aspect; 
        halfHeight = halfView; 
    }

    pixelSize = (halfWidth * 2) / hSize; 

}

const clrt::math::Mat4& Camera::getTransform() const noexcept {
    return transform;
}

const clrt::math::Mat4& Camera::getInverseTransform() const noexcept {
    return inverseTransform;
}

Matrix Camera::getTransformM() const {
    return clrt::compat::matrixToLegacy(transform);
}

void Camera::setTransform(const clrt::math::Mat4& matrix) {
    transform = matrix;
    inverseTransform = matrix.inverse();
}

void Camera::setTransformM(const Matrix& matrix) {
    setTransform(clrt::compat::matrixFromLegacy(matrix));
}

void Camera::print() {
     // Set floating-point formatting for clean reading
    std::cout << std::fixed << std::setprecision(4);
    
    std::cout << "========================================\n";
    std::cout << "             CAMERA STATUS              \n";
    std::cout << "========================================\n";
    
    // Canvas Dimensions & Field of View
    std::cout << "  Canvas Width (hSize) : " << hSize << " px\n";
    std::cout << "  Canvas Height (vSize): " << vSize << " px\n";
    std::cout << "  Field of View (fov)  : " << fov << " rad\n";
    std::cout << "  Aspect Ratio (aspect): " << aspect << "\n";
    std::cout << "----------------------------------------\n";
    
    // Internal Ray Generation Variables
    std::cout << "  Pixel Size           : " << pixelSize << "\n";
    std::cout << "  Half View            : " << halfView << "\n";
    std::cout << "  Half Width           : " << halfWidth << "\n";
    std::cout << "  Half Height          : " << halfHeight << "\n";
    std::cout << "----------------------------------------\n";
    
    // Transformation Matrix
    std::cout << "  Transformation Matrix:\n";
    
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            std::cout << transform(row, column) << ' ';
        }
        std::cout << '\n';
    }
    
    std::cout << "========================================\n" << std::endl;
}

// Must compute the world coordinates at the center of a given pixel, and then construct a ray that passes through that point 
Ray ray_for_pixel(const Camera& camera, double x, double y) {
    // The offset from the edge of the canvas to the pixels center 
    const double pixelSize = camera.getPixelSize();
    const double halfWidth = camera.getHalfWidth();
    const double halfHeight = camera.getHalfHeight();

    double xOffset = (x + 0.5) * pixelSize; 
    double yOffset = (y + 0.5) * pixelSize; 

    // The untransformed coordinates of the pixel in world space
    // (recall that the camera looks towards -z), so +x is to the *left)
    double world_x = halfWidth - xOffset; 
    double world_y = halfHeight - yOffset; 

    // Using the camera matrix, transform the canvas point and the origin. 
    // And then compute the rays direction vector
    // (Canvas is at z = -1)
    const clrt::math::Mat4& inverse = camera.getInverseTransform();
    const clrt::math::Point3 pixel =
        inverse * clrt::math::Point3{world_x, world_y, -1.0};
    const clrt::math::Point3 origin =
        inverse * clrt::math::Point3{0.0, 0.0, 0.0};
    const clrt::math::Vec3 direction = (pixel - origin).normalized();

    return Ray{origin, direction};
}
