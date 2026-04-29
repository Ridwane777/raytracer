#ifndef POINT3D_HPP_
#define POINT3D_HPP_
#include "Vector3D.hpp"

class Point3D {
    public:
        double X;
        double Y;
        double Z;

        Point3D() : X(0.0), Y(0.0), Z(0.0)  {};
        Point3D(double x, double y, double z) : X(x), Y(y), Z(z) {};
        ~Point3D() = default;

        Point3D operator+(const Vector3D& v) const;
        Vector3D operator-(const Point3D& other) const;
        double distanceTo(const Point3D& other) const;
};

#endif /* !POINT3D_HPP_ */
