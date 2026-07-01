#include "Hexagon.h"
#include "Cylinder.h"
#include "Sphere.h"
#include "Math/Operations.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// ---------------------------------------------------------
// Corner = small sphere
// ---------------------------------------------------------
std::shared_ptr<Shape> create_hexagon_corner() {
    auto corner = std::make_shared<Sphere>();

    Matrix m; // identity

    Matrix t = m.translation(0.0, 0.0, -1.0);
    Matrix s = m.scale(0.25, 0.25, 0.25);

    Matrix transform = t.multiplyMatrix(s);

    corner->setTransform(transform);
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

    Matrix m; // identity

    Matrix t  = m.translation(0.0, 0.0, -1.0);
    Matrix ry = m.rotateY(-M_PI / 6.0);
    Matrix rz = m.rotateZ(-M_PI / 2.0);
    Matrix s  = m.scale(0.25, 1.0, 0.25);

    // Correct order: T * RY * RZ * S
    Matrix transform = t.multiplyMatrix(ry);
    transform = transform.multiplyMatrix(rz);
    transform = transform.multiplyMatrix(s);

    edge->setTransform(transform);
    return edge;
}


// ---------------------------------------------------------
// One side (corner + edge)
// ---------------------------------------------------------
std::shared_ptr<Group> create_hexagon_side() {
    auto side = std::make_shared<Group>();

    side->add_child(create_hexagon_corner());
    side->add_child(create_hexagon_edge());

    return side;
}


// ---------------------------------------------------------
// Full hexagon (6 rotated sides)
// ---------------------------------------------------------
std::shared_ptr<Group> create_hexagon() {
    auto hex = std::make_shared<Group>();

    Matrix m; // identity

    for (int n = 0; n < 6; ++n) {
        auto side = create_hexagon_side();

        Matrix rot = m.rotateY(n * M_PI / 3.0);
        side->setTransform(rot);

        hex->add_child(side);
    }

    return hex;
}