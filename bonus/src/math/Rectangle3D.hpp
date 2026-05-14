#ifndef RECTANGLE3D_HPP_
#define RECTANGLE3D_HPP_
#include "Point3D.hpp"

class Rectangle3D {
public:
    Point3D  origin;
    Vector3D bottom_side;
    Vector3D left_side;

    Rectangle3D() = default;
    ~Rectangle3D() = default;
    Rectangle3D(const Point3D& origin,
                const Vector3D& bottom_side,
                const Vector3D& left_side);

    Point3D pointAt(double u, double v) const;
};

#endif 
