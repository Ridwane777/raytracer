#ifndef MATRIX4X4_HPP_
#define MATRIX4X4_HPP_
#include "Ray.hpp"
#include <cmath>

class Matrix4x4 {
public:
    double m[4][4];

    Matrix4x4();                          // matrice identité
    static Matrix4x4 identity();
    static Matrix4x4 translation(const Vector3D& t);
    static Matrix4x4 rotationX(double deg);
    static Matrix4x4 rotationY(double deg);
    static Matrix4x4 rotationZ(double deg);
    static Matrix4x4 scale(double sx, double sy, double sz);

    Matrix4x4 operator*(const Matrix4x4& other) const;
    Matrix4x4 inverse() const;
    Matrix4x4 transpose() const;

    Point3D  transformPoint(const Point3D& p) const;
    Vector3D transformVector(const Vector3D& v) const;
    // Transforme un rayon dans le repère local de l'objet.
    Ray      transformRay(const Ray& ray) const;
};

#endif /* !MATRIX4X4_HPP_ */
