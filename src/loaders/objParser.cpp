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

    if (prefix == "v") {
        Position pos;
        iss >> pos.x >> pos.y >> pos.z;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid vertex position");
            return;
        }
        m_positions.push_back(pos);
    } else if (prefix == "vt") {
        TexCoord tex;
        iss >> tex[0] >> tex[1];
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid texture coordinate");
            return;
        }
        m_texCoords.push_back(tex);
    } else if (prefix == "vn") {
        Normal norm;
        iss >> norm.x >> norm.y >> norm.z;
        if (iss.fail()) {
            logError(std::make_error_code(std::errc::invalid_argument), lineNum, "Invalid normal vector");
            return;
        }
        m_normals.push_back(norm);
    } else if (prefix == "f") {
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
    } else {
        // Ignore other prefixes for now
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


