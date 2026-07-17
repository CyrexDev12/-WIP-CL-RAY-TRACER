#include "scene/canvas.h"

#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <fstream>
#include <algorithm>

using namespace std;

// ---------------------------------------------------------
// Pixel Writing
// ---------------------------------------------------------
void Canvas::writePixel(int x, int y, const Color& C) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    int index = y * width + x;
    pixels[index] = C;
}

// ---------------------------------------------------------
// Bloom Helpers
// ---------------------------------------------------------
double Canvas::brightness(const Color& color) {
    return std::max({color.r, color.g, color.b});
}

Canvas Canvas::extractBrightPixels(double threshold) const {
    Canvas bright(width, height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color c = at(x, y);

            if (brightness(c) > threshold) {
                bright.writePixel(x, y, c);
            } else {
                bright.writePixel(x, y, Color{0.0, 0.0, 0.0});
            }
        }
    }

    return bright;
}

Canvas Canvas::horizontalBlur(int radius) const {
    Canvas result(width, height);

    if (radius <= 0) {
        return *this;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color sum{0.0, 0.0, 0.0};
            int count = 0;

            for (int dx = -radius; dx <= radius; ++dx) {
                int sx = x + dx;

                if (sx < 0 || sx >= width) {
                    continue;
                }

                sum = sum + at(sx, y);
                ++count;
            }

            result.writePixel(x, y, sum * (1.0 / count));
        }
    }

    return result;
}

Canvas Canvas::verticalBlur(int radius) const {
    Canvas result(width, height);

    if (radius <= 0) {
        return *this;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color sum{0.0, 0.0, 0.0};
            int count = 0;

            for (int dy = -radius; dy <= radius; ++dy) {
                int sy = y + dy;

                if (sy < 0 || sy >= height) {
                    continue;
                }

                sum = sum + at(x, sy);
                ++count;
            }

            result.writePixel(x, y, sum * (1.0 / count));
        }
    }

    return result;
}

Canvas Canvas::applyBloom() const {
    Canvas bright = extractBrightPixels(bloomThreshold);

    Canvas blurredHorizontal = bright.horizontalBlur(bloomRadius);
    Canvas blurred = blurredHorizontal.verticalBlur(bloomRadius);

    Canvas result(width, height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color original = at(x, y);
            Color glow = blurred.at(x, y) * bloomIntensity;

            result.writePixel(x, y, original + glow);
        }
    }

    return result;
}

// ---------------------------------------------------------
// PPM Conversion
// ---------------------------------------------------------
string Canvas::convertToPpm() {
    if (bloomEnabled) {
        Canvas outputCanvas = applyBloom();
        outputCanvas.bloomEnabled = false;
        return outputCanvas.convertToPpm();
    }

    string identifier = "P3";
    string widthStr = to_string(width);
    string heightStr = to_string(height);

    return identifier + "\n"
        + widthStr + " " + heightStr + "\n"
        + "255" + "\n"
        + constructPixelData() + "\n";
}

std::string Canvas::constructPixelData() {
    std::ostringstream oss;

    for (int y = 0; y < height; ++y) {
        int lineLength = 0;

        for (int x = 0; x < width; ++x) {
            const Color& c = at(x, y);

            // Clamp only at final PPM output.
            // This allows HDR-ish values to exist before bloom.
            auto clampAndScale = [](double v) -> int {
                if (v < 0.0) {
                    v = 0.0;
                }

                if (v > 1.0) {
                    v = 1.0;
                }

                return static_cast<int>(std::round(v * 255.0));
            };

            auto writeComponent = [&](int value) {
                std::string s = std::to_string(value);
                int needed = (lineLength == 0 ? 0 : 1) + static_cast<int>(s.size());

                if (lineLength + needed > 70) {
                    oss << "\n";
                    lineLength = 0;
                }

                if (lineLength > 0) {
                    oss << " ";
                    lineLength += 1;
                }

                oss << s;
                lineLength += static_cast<int>(s.size());
            };

            writeComponent(clampAndScale(c.r));
            writeComponent(clampAndScale(c.g));
            writeComponent(clampAndScale(c.b));
        }

        oss << "\n";
    }

    return oss.str();
}

int Canvas::getMaxColorVal() {
    return 255;
}

// ---------------------------------------------------------
// Output PPM File
// ---------------------------------------------------------
void Canvas::canvasOut() {
  

    Canvas outputCanvas = *this;

    if (bloomEnabled) {
        cout << "[DEBUG] Applying bloom post-processing..." << endl;

        outputCanvas = applyBloom();

        // Preserve bloom settings in case outputCanvas is inspected later.
        outputCanvas.bloomEnabled = false;
    }

    ofstream out("raySphereCanvas.ppm");

    if (!out) {
        cerr << "Could not create raySphereCanvas.ppm" << endl;
        return;
    }

    string ppm = outputCanvas.convertToPpm();

    out << ppm;
    out.close();


    cout << "Render complete! raySphereCanvas.ppm\n" << endl;
}
