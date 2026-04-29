#ifndef VECTOR3D_HPP_
#define VECTOR3D_HPP_
#include <iostream>

class Vector3D {
    public:
        double X;
        double Y;
        double Z;

        Vector3D();
        Vector3D(double x, double y, double z);
        ~Vector3D() = default;

        double length() const;
        double dot(const Vector3D& other) const;
        Vector3D normalize() const;
        Vector3D cross(const Vector3D& other) const;
        Vector3D& operator+=(Vector3D other);
        Vector3D& operator-=(Vector3D other);
        Vector3D& operator*=(Vector3D other);
        Vector3D& operator/=(Vector3D other);
        bool operator==(const Vector3D& other) const;
        bool operator!=(const Vector3D& other) const;
        Vector3D operator+(const Vector3D& v) const;
        Vector3D operator-(const Vector3D& v) const;
        Vector3D operator*(const Vector3D& v) const;
        Vector3D operator/(const Vector3D& v) const;
        Vector3D& operator*=(double scale);
        Vector3D& operator/=(double scale);
        Vector3D operator*(double scale) const;
        Vector3D operator/(double scale) const;
};

#endif /* !VECTOR3D_HPP_ */
