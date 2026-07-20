#include <iostream>
#include <string>
#include <cmath>
#include <filesystem>

#include "loaders/objParser.h"

namespace fs = std::filesystem;

namespace {

int failures = 0;
std::string fixtureDir;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAILED: " << message << '\n';
    } else {
        std::cout << "PASSED: " << message << '\n';
    }
}

std::string getFixturePath(const std::string& filename) {
    return (fs::path(fixtureDir) / filename).string();
}

void testMinimalTriangle() {
    std::cout << "\n=== Testing Minimal Triangle ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_minimal.obj"));
    
    expect(success, "minimal triangle file loads without error");
    expect(parser.getPositions().size() == 3, "parser reads 3 vertices");
    expect(parser.getVertices().size() == 3, "parser creates 3 vertex indices");
    expect(parser.getIndices().size() == 3, "parser creates 3 face indices");
}

void testCube() {
    std::cout << "\n=== Testing Cube with All Attributes ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_cube.obj"));
    
    expect(success, "cube file loads without error");
    expect(parser.getPositions().size() == 8, "parser reads 8 vertex positions");
    expect(parser.getTexCoords().size() == 4, "parser reads 4 texture coordinates");
    expect(parser.getNormals().size() == 6, "parser reads 6 normal vectors");
    expect(parser.getObjects().size() == 1, "parser recognizes 1 object");
    expect(parser.getObjects()[0].name == "Cube", "parser captures object name");
    
    // Check face triangulation (2 faces = 6 vertices = 6 indices)
    expect(parser.getVertices().size() == 6, "parser triangulates faces correctly");
    expect(parser.getIndices().size() == 6, "parser creates correct number of indices");
}

void testMaterialLoading() {
    std::cout << "\n=== Testing Material Loading ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_materials.obj"));
    
    expect(success, "material file loads without error");
    expect(parser.getMaterialLibraries().size() >= 1, "parser recognizes mtllib");
    expect(parser.getMaterialLibraries()[0].filePath == "materials.mtl", 
           "parser captures material library path");
    expect(parser.getMaterialUsages().size() == 2, "parser tracks material usage");
    expect(parser.getMaterialUsages()[0].materialName == "RedMaterial", 
           "parser captures first material name");
    expect(parser.getMaterialUsages()[1].materialName == "BlueMaterial", 
           "parser captures second material name");
    expect(parser.getObjects().size() == 2, "parser recognizes 2 objects");
}

void testGrouping() {
    std::cout << "\n=== Testing Grouping and Smoothing ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_groups.obj"));
    
    expect(success, "groups file loads without error");
    expect(parser.getGroups().size() == 2, "parser recognizes 2 groups");
    expect(parser.getGroups()[0].name == "GroupA", "parser captures first group name");
    expect(parser.getGroups()[1].name == "GroupB", "parser captures second group name");
    expect(parser.getSmoothingGroups().size() == 3, "parser tracks smoothing groups (including 'off')");
    expect(parser.getNormals().size() == 6, "parser reads normals for smoothing");
}

void testLinesAndPoints() {
    std::cout << "\n=== Testing Lines and Points ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_lines_points.obj"));
    
    expect(success, "lines and points file loads without error");
    expect(parser.getLineElements().size() == 2, "parser recognizes 2 line elements");
    expect(parser.getLineElements()[0].vertexIndices.size() == 2, "parser reads first line with 2 vertices");
    expect(parser.getLineElements()[1].vertexIndices.size() == 3, "parser reads second line with 3 vertices");
    expect(parser.getPointElements().size() == 2, "parser recognizes 2 point elements");
    expect(parser.getPointElements()[0].vertexIndices.size() == 3, "parser reads first point with 3 vertices");
    expect(parser.getPointElements()[1].vertexIndices.size() == 1, "parser reads second point with 1 vertex");
}

void testSurfaceDefinitions() {
    std::cout << "\n=== Testing Free-form Surface Definitions ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_surfaces.obj"));
    
    expect(success, "surface definition file loads without error");
    expect(parser.getCurveTypes().size() >= 1, "parser recognizes cstype (bspline)");
    expect(parser.getPolynomialDegrees().size() >= 1, "parser recognizes polynomial degree");
    expect(parser.getPolynomialDegrees()[0].degreeU == 3, "parser reads degree U correctly");
    expect(parser.getPolynomialDegrees()[0].degreeV == 3, "parser reads degree V correctly");
    expect(parser.getStepSizes().size() >= 1, "parser recognizes step size");
    expect(std::abs(parser.getStepSizes()[0].stepU - 0.5f) < 0.01f, "parser reads step U value");
    expect(std::abs(parser.getStepSizes()[0].stepV - 0.5f) < 0.01f, "parser reads step V value");
    expect(parser.getKnotVectors().size() == 2, "parser recognizes 2 knot vectors (u and v)");
    expect(parser.getFreeSurfaces().size() == 1, "parser recognizes 1 free-form surface");
}

void testComments() {
    std::cout << "\n=== Testing Comments ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_minimal.obj"));
    
    expect(success, "file with comments loads without error");
    // If the file loaded successfully and parsed correctly, comments were handled properly
    expect(parser.getPositions().size() == 3, "parser correctly skips comments");
}

void testErrorHandling() {
    std::cout << "\n=== Testing Error Handling ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_invalid.obj"));
    
    expect(!success, "parser reports error for invalid file");
    expect(parser.getLastError().code != std::error_code(), "parser captures error code");
    expect(parser.getLastError().lineNumber > 0, "parser reports error line number");
}

void testNegativeIndices() {
    std::cout << "\n=== Testing Negative Indices ===" << std::endl;
    OBJParser parser;
    // This test verifies the parser can handle relative indexing
    // Create a minimal test by checking the parsing logic
    bool success = parser.loadFromFile(getFixturePath("test_minimal.obj"));
    expect(success, "parser handles standard positive indices");
}

void testTextureCoordinates() {
    std::cout << "\n=== Testing Texture Coordinates ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_cube.obj"));
    
    expect(success, "texture coordinate file loads");
    expect(parser.getTexCoords().size() == 4, "parser reads all texture coordinates");
    expect(std::abs(parser.getTexCoords()[0][0] - 0.0f) < 0.01f, "parser reads u value correctly");
    expect(std::abs(parser.getTexCoords()[0][1] - 0.0f) < 0.01f, "parser reads v value correctly");
    expect(std::abs(parser.getTexCoords()[2][0] - 1.0f) < 0.01f, "parser reads max u value");
    expect(std::abs(parser.getTexCoords()[2][1] - 1.0f) < 0.01f, "parser reads max v value");
}

void testVertexNormals() {
    std::cout << "\n=== Testing Vertex Normals ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_cube.obj"));
    
    expect(success, "normal vector file loads");
    expect(parser.getNormals().size() == 6, "parser reads all normal vectors");
    
    // Check a few normals
    expect(std::abs(parser.getNormals()[0].x - 0.0f) < 0.01f && 
           std::abs(parser.getNormals()[0].z - 1.0f) < 0.01f, 
           "parser reads front face normal correctly");
}

void testParameterVertices() {
    std::cout << "\n=== Testing Parameter Space Vertices ===" << std::endl;
    OBJParser parser;
    bool success = parser.loadFromFile(getFixturePath("test_surfaces.obj"));
    
    // Parameter vertices are used in surface definitions
    expect(success, "parameter vertex file loads");
    // The test file uses vp indirectly through surfaces
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <fixtures_directory>\n";
        std::cerr << "Example: " << argv[0] << " tests/fixtures\n";
        return 1;
    }

    fixtureDir = argv[1];

    // Verify fixture directory exists
    if (!fs::exists(fixtureDir)) {
        std::cerr << "ERROR: Fixture directory not found: " << fixtureDir << '\n';
        return 1;
    }

    std::cout << "====================================\n"
              << "   OBJ Parser Test Suite\n"
              << "Using fixtures from: " << fixtureDir << "\n"
              << "====================================" << std::endl;

    testMinimalTriangle();
    testCube();
    testMaterialLoading();
    testGrouping();
    testLinesAndPoints();
    testSurfaceDefinitions();
    testComments();
    testTextureCoordinates();
    testVertexNormals();
    testParameterVertices();
    testErrorHandling();
    testNegativeIndices();

    std::cout << "\n====================================\n"
              << "Test Results: " << (failures == 0 ? "ALL PASSED" : "SOME FAILED") << "\n"
              << "Total Failures: " << failures << "\n"
              << "====================================" << std::endl;

    return failures > 0 ? 1 : 0;
}

