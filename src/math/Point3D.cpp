#include "Point3D.hpp"

Point3D::Point3D(): X(0.0), Y(0.0), Z(0.0)  {}

Point3D::Point3D(double x, double y, double z) : X(x), Y(y), Z(z) {}

Point3D Point3D::operator+(const Vector3D& v) const {
    return Point3D(X + v.X, Y + v.Y, Z + v.Z);
}

Vector3D Point3D::operator-(const Point3D& other) const {
    return Vector3D(X - other.X, Y - other.Y, Z - other.Z);
}

double Point3D::distanceTo(const Point3D& other) const {
    return (*this - other).length();
}