#include "Hexagon.h"
#include "Cylinder.h"
#include "Sphere.h"
#include "math/Operations.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static constexpr double HEX_RADIUS = 0.12;

// ---------------------------------------------------------
// Corner = small sphere
// ---------------------------------------------------------
std::shared_ptr<Shape> create_hexagon_corner() {
    auto corner = std::make_shared<Sphere>();

    Matrix m;

    Matrix t = m.translation(0.0, 0.0, -1.0);
    Matrix s = m.scale(HEX_RADIUS, HEX_RADIUS, HEX_RADIUS);

    Matrix transform = t.multiplyMatrix(s);

    corner->setTransform(transform);

    corner->setMaterialColor(Color{0.2, 0.6, 1.0});
    corner->setAmbient(0.1);
    corner->setDiffuse(0.7);
    corner->setSpecular(0.15);
    corner->setShininess(50);

    return corner;
}

// ---------------------------------------------------------
// Edge = cylinder between corners
// ---------------------------------------------------------
std::shared_ptr<Shape> create_hexagon_edge() {
    auto edge = std::make_shared<Cylinder>();

    edge->setMin(0.0);
    edge->setMax(1.0);
    edge->setClosed(false);

    Matrix m;

    Matrix t  = m.translation(0.0, 0.0, -1.0);
    Matrix ry = m.rotateY(-M_PI / 6.0);
    Matrix rz = m.rotateZ(-M_PI / 2.0);
    Matrix s  = m.scale(HEX_RADIUS, 1.0, HEX_RADIUS);

    Matrix transform = t.multiplyMatrix(ry);
    transform = transform.multiplyMatrix(rz);
    transform = transform.multiplyMatrix(s);

    edge->setTransform(transform);

    edge->setMaterialColor(Color{0.2, 0.6, 1.0});
    edge->setAmbient(0.1);
    edge->setDiffuse(0.7);
    edge->setSpecular(0.15);
    edge->setShininess(50);

    return edge;
}

// ---------------------------------------------------------
// One side = corner + edge
// ---------------------------------------------------------
std::shared_ptr<Group> create_hexagon_side() {
    auto side = std::make_shared<Group>();

    side->add_child(create_hexagon_corner());
    side->add_child(create_hexagon_edge());

    return side;
}

// ---------------------------------------------------------
// Full hexagon = 6 rotated sides
// ---------------------------------------------------------
std::shared_ptr<Group> create_hexagon() {
    auto hex = std::make_shared<Group>();

    Matrix m;

    for (int n = 0; n < 6; ++n) {
        auto side = create_hexagon_side();

        Matrix rot = m.rotateY(n * M_PI / 3.0);
        side->setTransform(rot);

        hex->add_child(side);
    }

    return hex;
}
