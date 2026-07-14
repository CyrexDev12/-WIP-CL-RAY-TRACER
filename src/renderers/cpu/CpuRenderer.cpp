#include "renderers/cpu/CpuRenderer.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace {

std::mutex progressMutex;

void updateProgress(int completedSlots) {
    constexpr int maxSlots = 20;
    const double percentage =
        (static_cast<double>(completedSlots) / maxSlots) * 100.0;

    const std::string bar = std::string(completedSlots, '#')
        + std::string(maxSlots - completedSlots, '-');
    std::string output = "[" + bar + "] ("
        + std::to_string(static_cast<int>(percentage)) + "%)";

    constexpr std::size_t padWidth = 64;
    if (output.size() < padWidth) {
        output += std::string(padWidth - output.size(), ' ');
    }

    std::lock_guard<std::mutex> lock(progressMutex);
    std::cout << '\r' << output << std::flush;
}

} // namespace

Canvas renderCpu(const Camera& camera, World& world, bool multithreaded) {
    const int width = static_cast<int>(camera.gethSize());
    const int height = static_cast<int>(camera.getvSize());
    Canvas canvas(width, height);

    if (!multithreaded) {
        const int totalPixels = canvas.width * canvas.height;
        int pixelCount = 0;
        constexpr int maxSlots = 20;
        int lastSlots = -1;

        std::cout << "[DEBUG] Rendering single-threaded..." << std::endl;
        const auto start = std::chrono::high_resolution_clock::now();

        for (int y = 0; y < canvas.height; ++y) {
            for (int x = 0; x < canvas.width; ++x) {
                const Ray ray = ray_for_pixel(camera, x, y);
                canvas.writePixel(x, y, world.Color_at(ray));

                const int completedSlots = (++pixelCount * maxSlots) / totalPixels;
                if (completedSlots != lastSlots) {
                    updateProgress(completedSlots);
                    lastSlots = completedSlots;
                }
            }
        }

        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start);
        std::cout << std::endl
                  << "[DEBUG] Single-threaded render time: "
                  << duration.count() << " ms" << std::endl;
        return canvas;
    }

    unsigned int threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0) {
        threadCount = 4;
    }
    threadCount = std::min(threadCount, static_cast<unsigned int>(height));

    std::cout << "[DEBUG] Rendering multithreaded with "
              << threadCount << " threads..." << std::endl;
    const auto start = std::chrono::high_resolution_clock::now();

    std::atomic<int> rowsCompleted{0};
    std::atomic<int> lastSlots{-1};
    constexpr int maxSlots = 20;

    auto renderRows = [&](int yStart, int yEnd) {
        for (int y = yStart; y < yEnd; ++y) {
            for (int x = 0; x < width; ++x) {
                const Ray ray = ray_for_pixel(camera, x, y);
                canvas.writePixel(x, y, world.Color_at(ray));
            }

            const int completedSlots = (++rowsCompleted * maxSlots) / height;
            int previousSlots = lastSlots.load();
            if (completedSlots != previousSlots
                && lastSlots.compare_exchange_strong(previousSlots, completedSlots)) {
                updateProgress(completedSlots);
            }
        }
    };

    std::vector<std::thread> threads;
    const int rowsPerThread = height / static_cast<int>(threadCount);
    const int extraRows = height % static_cast<int>(threadCount);
    int currentY = 0;

    for (unsigned int i = 0; i < threadCount; ++i) {
        const int yStart = currentY;
        int yEnd = yStart + rowsPerThread;
        if (static_cast<int>(i) < extraRows) {
            ++yEnd;
        }
        currentY = yEnd;
        threads.emplace_back(renderRows, yStart, yEnd);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    updateProgress(maxSlots);
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start);
    std::cout << std::endl
              << "[DEBUG] Multithreaded render time: "
              << duration.count() << " ms" << std::endl;
    return canvas;
}
