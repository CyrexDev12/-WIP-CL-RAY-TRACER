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
    // Data structures for all OBJ prefixes

    // 1. GEOMETRIC PRIMITIVES
    struct ParameterVertex {
        clrt::math::Scalar u, v;
    };

    // 2. ELEMENTS & TOPOLOGY
    struct LineElement {
        std::vector<int> vertexIndices;
        std::vector<int> texCoordIndices;
    };

    struct PointElement {
        std::vector<int> vertexIndices;
    };

    // 3. GROUPING, HIERARCHY, AND SMOOTHING
    struct ObjectDef {
        std::string name;
        size_t startIndex{0};
        size_t faceCount{0};
    };

    struct GroupDef {
        std::string name;
        size_t startIndex{0};
        size_t faceCount{0};
    };

    struct SmoothingGroup {
        int groupNumber;
        size_t startIndex{0};
        size_t faceCount{0};
    };

    // 4. MATERIALS & SHADING
    struct MaterialLibrary {
        std::string filePath;
    };

    struct MaterialUsage {
        std::string materialName;
        size_t startIndex{0};
        size_t faceCount{0};
    };

    struct ShadowProxy {
        std::string filePath;
    };

    struct TraceProxy {
        std::string filePath;
    };

    struct LODLevel {
        int levelNumber;
    };

    // 5. FREE-FORM CURVES AND SURFACES
    struct CurveType {
        std::string type; // e.g., "bspline", "bezier", "taylor"
    };

    struct SurfaceType {
        std::string type; // e.g., "bspline", "bezier", "taylor"
    };

    struct PolynomialDegree {
        int degreeU{0};
        int degreeV{0};
    };

    struct StepSize {
        clrt::math::Scalar stepU{1.0};
        clrt::math::Scalar stepV{1.0};
    };

    struct KnotVector {
        char direction; // 'u' or 'v'
        std::vector<clrt::math::Scalar> knots;
    };

    struct FreeCurve {
        int startParamU{0};
        int endParamU{0};
        std::vector<int> controlPointIndices;
    };

    struct FreeCurve2D {
        std::vector<int> controlPointIndices;
    };

    struct FreeSurface {
        int startParamU{0};
        int endParamU{0};
        int startParamV{0};
        int endParamV{0};
        std::vector<int> controlPointIndices;
    };

    struct SurfaceConnectivity {
        int surface1{0};
        int curve1{0};
        int surface2{0};
        std::vector<int> curveIndices;
    };

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

    // Getters for additional geometry data
    const std::vector<Position>& getPositions() const noexcept { return m_positions; }
    const std::vector<TexCoord>& getTexCoords() const noexcept { return m_texCoords; }
    const std::vector<Normal>& getNormals() const noexcept { return m_normals; }
    const std::vector<ParameterVertex>& getParameterVertices() const noexcept { return m_paramVertices; }
    
    // Getters for elements
    const std::vector<LineElement>& getLineElements() const noexcept { return m_lineElements; }
    const std::vector<PointElement>& getPointElements() const noexcept { return m_pointElements; }
    
    // Getters for grouping/hierarchy
    const std::vector<ObjectDef>& getObjects() const noexcept { return m_objects; }
    const std::vector<GroupDef>& getGroups() const noexcept { return m_groups; }
    const std::vector<SmoothingGroup>& getSmoothingGroups() const noexcept { return m_smoothingGroups; }
    
    // Getters for materials
    const std::vector<MaterialLibrary>& getMaterialLibraries() const noexcept { return m_materialLibs; }
    const std::vector<MaterialUsage>& getMaterialUsages() const noexcept { return m_materialUsages; }
    const std::vector<ShadowProxy>& getShadowProxies() const noexcept { return m_shadowProxies; }
    const std::vector<TraceProxy>& getTraceProxies() const noexcept { return m_traceProxies; }
    const std::vector<LODLevel>& getLODLevels() const noexcept { return m_lodLevels; }
    
    // Getters for curves and surfaces
    const std::vector<CurveType>& getCurveTypes() const noexcept { return m_curveTypes; }
    const std::vector<SurfaceType>& getSurfaceTypes() const noexcept { return m_surfaceTypes; }
    const std::vector<PolynomialDegree>& getPolynomialDegrees() const noexcept { return m_polyDegrees; }
    const std::vector<StepSize>& getStepSizes() const noexcept { return m_stepSizes; }
    const std::vector<KnotVector>& getKnotVectors() const noexcept { return m_knotVectors; }
    const std::vector<FreeCurve>& getFreeCurves() const noexcept { return m_freeCurves; }
    const std::vector<FreeCurve2D>& getFreeCurves2D() const noexcept { return m_freeCurves2D; }
    const std::vector<FreeSurface>& getFreeSurfaces() const noexcept { return m_freeSurfaces; }
    const std::vector<SurfaceConnectivity>& getSurfaceConnectivities() const noexcept { return m_surfaceConnectivities; }

private:
    // Raw wavemen data caches
    std::vector<Position> m_positions;
    std::vector<TexCoord> m_texCoords;
    std::vector<Normal> m_normals;

    // 1. GEOMETRIC PRIMITIVES
    std::vector<ParameterVertex> m_paramVertices;

    // 2. ELEMENTS & TOPOLOGY
    std::vector<LineElement> m_lineElements;
    std::vector<PointElement> m_pointElements;

    // 3. GROUPING, HIERARCHY, AND SMOOTHING
    std::vector<ObjectDef> m_objects;
    std::vector<GroupDef> m_groups;
    std::vector<SmoothingGroup> m_smoothingGroups;

    // 4. MATERIALS & SHADING
    std::vector<MaterialLibrary> m_materialLibs;
    std::vector<MaterialUsage> m_materialUsages;
    std::vector<ShadowProxy> m_shadowProxies;
    std::vector<TraceProxy> m_traceProxies;
    std::vector<LODLevel> m_lodLevels;

    // 5. FREE-FORM CURVES AND SURFACES
    std::vector<CurveType> m_curveTypes;
    std::vector<SurfaceType> m_surfaceTypes;
    std::vector<PolynomialDegree> m_polyDegrees;
    std::vector<StepSize> m_stepSizes;
    std::vector<KnotVector> m_knotVectors;
    std::vector<FreeCurve> m_freeCurves;
    std::vector<FreeCurve2D> m_freeCurves2D;
    std::vector<FreeSurface> m_freeSurfaces;
    std::vector<SurfaceConnectivity> m_surfaceConnectivities;

    // Final consolidated pipeline-ready arrays
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    
    ParseError m_lastError;

    // Internal line tokenization processing
    void processLine(const std::string& line, size_t lineNum);
    void logError(std::error_code ec, size_t lineNum, const std::string& msg);
};
