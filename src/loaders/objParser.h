#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <system_error>
#include <vector>

#include "core/math/Point3.h"
#include "core/math/Vec3.h"
#include "scene/MeshAsset.h"

using Position = clrt::math::Point3;
using Normal = clrt::math::Vec3;
using TexCoord = std::array<clrt::math::Scalar, 2>;
using Vertex = clrt::scene::MeshVertex;

class OBJParser {
public:
    

    // Tracks exactly where a parsing failure occurs
    struct ParseError {
        std::error_code code;
        size_t lineNumber{0};
        std::string message;
    };

    OBJParser() = default;
    ~OBJParser() = default;

    // Core parsing execution
    bool loadFromFile(const std::string& filePath);
    
    // Getters for CPU-side data feeding (Metal, Vulkan, DX12, etc.)
    const std::vector<Vertex>& getVertices() const noexcept { return m_vertices; }
    const std::vector<uint32_t>& getIndices() const noexcept { return m_indices; }
    const ParseError& getLastError() const noexcept { return m_lastError; }

private:
    // Raw wavemen data caches
    std::vector<Position> m_positions;
    std::vector<TexCoord> m_texCoords;
    std::vector<Normal> m_normals;

    // Final consolidated pipeline-ready arrays
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    
    ParseError m_lastError;

    // Internal line tokenization processing
    void processLine(const std::string& line, size_t lineNum);
    void logError(std::error_code ec, size_t lineNum, const std::string& msg);
};
