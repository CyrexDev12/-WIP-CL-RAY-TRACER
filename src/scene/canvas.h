#ifndef CANVAS_H
#define CANVAS_H

#include <vector>
#include <cstdint>
#include <string>
#include "math/Operations.h"

struct Canvas {
    int width; 
    int height; 
    std::vector<Color> pixels;

    // Bloom settings
    bool bloomEnabled;
    double bloomIntensity;
    double bloomThreshold;
    int bloomRadius;

    // HDR output settings
    bool toneMappingEnabled;
    double exposure;
    double gamma;

    // Normal constructor: bloom off
    Canvas(int w, int h)
        : width(w),
          height(h),
          pixels(w * h, Color{0, 0, 0}),
          bloomEnabled(false),
          bloomIntensity(0.0),
          bloomThreshold(1.0),
          bloomRadius(6),
          toneMappingEnabled(true),
          exposure(1.0),
          gamma(2.2)
    {}

    // Bloom constructor
    Canvas(
        int w,
        int h,
        bool bloom,
        double intensity,
        double threshold = 1.0,
        int radius = 6,
        bool toneMapping = true,
        double outputExposure = 1.0,
        double outputGamma = 2.2
    )
        : width(w),
          height(h),
          pixels(w * h, Color{0, 0, 0}),
          bloomEnabled(bloom),
          bloomIntensity(intensity),
          bloomThreshold(threshold),
          bloomRadius(radius),
          toneMappingEnabled(toneMapping),
          exposure(outputExposure),
          gamma(outputGamma)
    {}

    Color& at(int x, int y) {
        return pixels[y * width + x];
    }

    const Color& at(int x, int y) const {
        return pixels[y * width + x];
    }

    void writePixel(int x, int y, const Color& C);

    void canvasOut(); 

    std::string convertToPpm();
    std::string constructPixelData();

    int getMaxColorVal();

private:
    Canvas applyBloom() const;
    Canvas extractBrightPixels(double threshold) const;
    Canvas horizontalBlur(int radius) const;
    Canvas verticalBlur(int radius) const;

    static double brightness(const Color& color);
};

#endif
