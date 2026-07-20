#include "objParser.h"
#include <fstream>
#include <sstream>


void OBJParser::logError(std::error_code ec, size_t lineNum, const std::string& msg) {
    m_lastError = {ec, lineNum, msg};
}

// .txt file example located in ../docs/obj_formatEx.txt
// .txt file with all prefixes located in ../docs/obj_format_Prefixes.txt
void OBJParser::processLine(const std::string& line, size_t lineNum) {
    if (line.empty() || line[0] == '#') {
        return; // Skip empty lines and comments
    }

    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    // ===== 1. GEOMETRIC PRIMITIVES =====
    if (prefix == "v") {
        Position pos;
        iss >> pos.x >> pos.y >> pos.z;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid vertex position");
            return;
        }
        // Optional weight 'w' (defaults to 1.0) is ignored
        m_positions.push_back(pos);
    } 
    else if (prefix == "vt") {
        TexCoord tex;
        iss >> tex[0] >> tex[1];
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid texture coordinate");
            return;
        }
        // Optional 'w' component (defaults to 0.0) is ignored
        m_texCoords.push_back(tex);
    } 
    else if (prefix == "vn") {
        Normal norm;
        iss >> norm.x >> norm.y >> norm.z;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid normal vector");
            return;
        }
        m_normals.push_back(norm);
    } 
    else if (prefix == "vp") {
        // Parameter space vertices for curves/surfaces
        ParameterVertex pv;
        iss >> pv.u >> pv.v;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid parameter space vertex");
            return;
        }
        m_paramVertices.push_back(pv);
    }
    // ===== 2. ELEMENTS & TOPOLOGY =====
    else if (prefix == "f") {
        std::string vertexStr;
        std::vector<Vertex> faceVertices;

        while (iss >> vertexStr) {
            std::istringstream viss(vertexStr);
            std::string indexStr;
            std::vector<int> indices;

            // Explode single tokens like "1/2/3" or "1//3"
            while (std::getline(viss, indexStr, '/')) {
                if (!indexStr.empty()) {
                    try {
                        indices.push_back(std::stoi(indexStr));
                    } catch (const std::invalid_argument&) {
                        logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid face index format");
                        return;
                    }
                } else {
                    indices.push_back(0); // Optional attribute omitted
                }
            }

            if (indices.empty()) continue;

            Vertex vertex{};
            
            // 1. Position Mapping (Required)
            int vIdx = indices[0];
            vIdx = (vIdx < 0) ? static_cast<int>(m_positions.size()) + vIdx + 1 : vIdx;
            if (vIdx > 0 && vIdx <= static_cast<int>(m_positions.size())) {
                vertex.position = m_positions[vIdx - 1];
            } else {
                logError(std::make_error_code(std::errc::result_out_of_range), lineNum, "Vertex position index out of bounds");
                return;
            }

            // 2. Texture Mapping (Optional)
            if (indices.size() > 1 && indices[1] != 0) {
                int vtIdx = indices[1];
                vtIdx = (vtIdx < 0) ? static_cast<int>(m_texCoords.size()) + vtIdx + 1 : vtIdx;
                if (vtIdx > 0 && vtIdx <= static_cast<int>(m_texCoords.size())) {
                    vertex.texCoord = m_texCoords[vtIdx - 1];
                    vertex.hasTexCoord = true;
                } else {
                    logError(std::make_error_code(std::errc::result_out_of_range), lineNum, "Texture index out of bounds");
                    return;
                }
            }

            // 3. Normal Mapping (Optional)
            if (indices.size() > 2 && indices[2] != 0) {
                int vnIdx = indices[2];
                vnIdx = (vnIdx < 0) ? static_cast<int>(m_normals.size()) + vnIdx + 1 : vnIdx;
                if (vnIdx > 0 && vnIdx <= static_cast<int>(m_normals.size())) {
                    vertex.normal = m_normals[vnIdx - 1];
                } else {
                    logError(std::make_error_code(std::errc::result_out_of_range), lineNum, "Normal index out of bounds");
                    return;
                }
            }

            faceVertices.push_back(vertex);
        }

        if (faceVertices.size() < 3) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Face must have at least 3 vertices");
            return;
        }

        // Fan triangulation for polygon faces (handles triangles, quads, and N-gons)
        for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
            m_vertices.push_back(faceVertices[0]);
            m_vertices.push_back(faceVertices[i]);
            m_vertices.push_back(faceVertices[i + 1]);

            uint32_t currentSize = static_cast<uint32_t>(m_vertices.size());
            m_indices.push_back(currentSize - 3);
            m_indices.push_back(currentSize - 2);
            m_indices.push_back(currentSize - 1);
        }
    }
    else if (prefix == "l") {
        // Line element: l [v1/vt1] [v2/vt2] ...
        LineElement line;
        std::string vertexStr;
        while (iss >> vertexStr) {
            std::istringstream viss(vertexStr);
            std::string indexStr;
            int vIdx = 0, vtIdx = 0;
            
            std::getline(viss, indexStr, '/');
            if (!indexStr.empty()) {
                try {
                    vIdx = std::stoi(indexStr);
                } catch (const std::invalid_argument&) {
                    logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid line vertex index");
                    return;
                }
            }
            
            std::getline(viss, indexStr, '/');
            if (!indexStr.empty()) {
                try {
                    vtIdx = std::stoi(indexStr);
                } catch (const std::invalid_argument&) {
                    logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid line texture coordinate index");
                    return;
                }
            }
            
            line.vertexIndices.push_back(vIdx);
            line.texCoordIndices.push_back(vtIdx);
        }
        m_lineElements.push_back(line);
    }
    else if (prefix == "p") {
        // Point element: p [v1] [v2] [v3] ...
        PointElement point;
        int vIdx;
        while (iss >> vIdx) {
            point.vertexIndices.push_back(vIdx);
        }
        m_pointElements.push_back(point);
    }
    // ===== 3. GROUPING, HIERARCHY, AND SMOOTHING =====
    else if (prefix == "o") {
        // Object name: o [object_name]
        std::string objName;
        iss >> objName;
        ObjectDef objDef;
        objDef.name = objName;
        objDef.startIndex = m_vertices.size();
        m_objects.push_back(objDef);
    }
    else if (prefix == "g") {
        // Group name: g [group_name] ...
        std::string groupName;
        iss >> groupName;
        GroupDef groupDef;
        groupDef.name = groupName;
        groupDef.startIndex = m_vertices.size();
        m_groups.push_back(groupDef);
    }
    else if (prefix == "s") {
        // Smoothing group: s [group_number/off]
        std::string smoothingStr;
        iss >> smoothingStr;
        SmoothingGroup smoothGroup;
        if (smoothingStr == "off") {
            smoothGroup.groupNumber = 0;
        } else {
            try {
                smoothGroup.groupNumber = std::stoi(smoothingStr);
            } catch (const std::invalid_argument&) {
                logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid smoothing group number");
                return;
            }
        }
        smoothGroup.startIndex = m_vertices.size();
        m_smoothingGroups.push_back(smoothGroup);
    }
    else if (prefix == "mg") {
        // Merging group: mg [group_number]
        int mgNum;
        iss >> mgNum;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid merging group number");
            return;
        }
        // Merging groups are informational; stored implicitly with smoothing groups
    }
    // ===== 4. MATERIALS & SHADING PIPELINES =====
    else if (prefix == "mtllib") {
        // Material library: mtllib [file_name.mtl]
        std::string filePath;
        iss >> filePath;
        MaterialLibrary matLib;
        matLib.filePath = filePath;
        m_materialLibs.push_back(matLib);
    }
    else if (prefix == "usemtl") {
        // Use material: usemtl [material_name]
        std::string materialName;
        iss >> materialName;
        MaterialUsage matUsage;
        matUsage.materialName = materialName;
        matUsage.startIndex = m_vertices.size();
        m_materialUsages.push_back(matUsage);
    }
    else if (prefix == "shadow_obj") {
        // Shadow proxy: shadow_obj [file_name]
        std::string filePath;
        iss >> filePath;
        ShadowProxy shadowProxy;
        shadowProxy.filePath = filePath;
        m_shadowProxies.push_back(shadowProxy);
    }
    else if (prefix == "trace_obj") {
        // Trace proxy: trace_obj [file_name]
        std::string filePath;
        iss >> filePath;
        TraceProxy traceProxy;
        traceProxy.filePath = filePath;
        m_traceProxies.push_back(traceProxy);
    }
    else if (prefix == "lod") {
        // Level of Detail: lod [level_number]
        int lodLevel;
        iss >> lodLevel;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid LOD level number");
            return;
        }
        LODLevel lod;
        lod.levelNumber = lodLevel;
        m_lodLevels.push_back(lod);
    }
    // ===== 5. FREE-FORM CURVES AND SURFACES (ADVANCED TOPOLOGY) =====
    else if (prefix == "cstype") {
        // Curve/surface type: cstype [type]
        std::string csType;
        iss >> csType;
        
        // Determine if it's a curve or surface type
        if (csType == "bspline" || csType == "bezier" || csType == "taylor" || csType == "rational") {
            // Could be either; convention: if next token is "curv" it's a curve type
            // For now, we store both as potential curve/surface types
            CurveType curveType;
            curveType.type = csType;
            m_curveTypes.push_back(curveType);
            
            SurfaceType surfaceType;
            surfaceType.type = csType;
            m_surfaceTypes.push_back(surfaceType);
        }
    }
    else if (prefix == "deg") {
        // Polynomial degree: deg [deg_u] [deg_v]
        int degU, degV;
        iss >> degU >> degV;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid polynomial degree");
            return;
        }
        PolynomialDegree polyDeg;
        polyDeg.degreeU = degU;
        polyDeg.degreeV = degV;
        m_polyDegrees.push_back(polyDeg);
    }
    else if (prefix == "step") {
        // Step size: step [step_u] [step_v]
        clrt::math::Scalar stepU, stepV;
        iss >> stepU >> stepV;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid step size");
            return;
        }
        StepSize stepSize;
        stepSize.stepU = stepU;
        stepSize.stepV = stepV;
        m_stepSizes.push_back(stepSize);
    }
    else if (prefix == "parm") {
        // Parameter/knot vector: parm [u/v] [knots...]
        char direction;
        iss >> direction;
        if (direction != 'u' && direction != 'v') {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid knot vector direction (must be 'u' or 'v')");
            return;
        }
        KnotVector knotVec;
        knotVec.direction = direction;
        clrt::math::Scalar knot;
        while (iss >> knot) {
            knotVec.knots.push_back(knot);
        }
        m_knotVectors.push_back(knotVec);
    }
    else if (prefix == "curv") {
        // Free-form curve: curv [u1] [u2] [v...]
        int u1, u2;
        iss >> u1 >> u2;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid curve parameters");
            return;
        }
        FreeCurve curve;
        curve.startParamU = u1;
        curve.endParamU = u2;
        int vIdx;
        while (iss >> vIdx) {
            curve.controlPointIndices.push_back(vIdx);
        }
        m_freeCurves.push_back(curve);
    }
    else if (prefix == "curv2") {
        // 2D free-form curve: curv2 [v...]
        FreeCurve2D curve2D;
        int vIdx;
        while (iss >> vIdx) {
            curve2D.controlPointIndices.push_back(vIdx);
        }
        m_freeCurves2D.push_back(curve2D);
    }
    else if (prefix == "surf") {
        // Free-form surface: surf [s1] [s2] [u...] [v...]
        int s1, s2;
        iss >> s1 >> s2;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid surface parameters");
            return;
        }
        FreeSurface surface;
        surface.startParamU = s1;
        surface.endParamU = s2;
        // Parse remaining control point indices
        int vIdx;
        while (iss >> vIdx) {
            surface.controlPointIndices.push_back(vIdx);
        }
        m_freeSurfaces.push_back(surface);
    }
    else if (prefix == "con") {
        // Surface connectivity: con [s1] [c1] [s2] [c2] ...
        SurfaceConnectivity connectivity;
        int s1, c1, s2;
        iss >> s1 >> c1 >> s2;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid surface connectivity");
            return;
        }
        connectivity.surface1 = s1;
        connectivity.curve1 = c1;
        connectivity.surface2 = s2;
        int cIdx;
        while (iss >> cIdx) {
            connectivity.curveIndices.push_back(cIdx);
        }
        m_surfaceConnectivities.push_back(connectivity);
    }
    else {
        // Unknown prefix - silently ignore (allows forward compatibility)
        // Could log a warning here if desired
    }
}



// Core parsing execution
bool OBJParser::loadFromFile(const std::string& filePath) {


    auto file = std::ifstream(filePath);
    if (!file.is_open()) {
        logError(std::make_error_code(std::errc::no_such_file_or_directory), 0, "Failed to open file");
        return false;
    }

    std::string line;
    size_t lineNum = 0;

    while (std::getline(file, line)) {
        ++lineNum;
        processLine(line, lineNum);
        if (m_lastError.code) {
            return false;
        }
    }


    return true;
}


