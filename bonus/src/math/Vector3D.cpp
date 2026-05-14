#include "Vector3D.hpp"
#include <cmath>

Vector3D::Vector3D() : X(0.0), Y(0.0), Z(0.0)  {};

Vector3D::Vector3D(double x, double y, double z) : X(x), Y(y), Z(z) {};

double Vector3D::length() const {
    auto squarex = X * X;
    auto squarey = Y * Y;
    auto squarez = Z * Z;

    return std::sqrt((squarex + squarey + squarez));
};

double Vector3D::dot(const Vector3D& other) const {
    return (X * other.X) + (Y * other.Y) + (Z * other.Z);
}

Vector3D& Vector3D::operator+=(Vector3D other) {
    X += other.X;
    Y += other.Y;
    Z += other.Z;
    return *this;
};

Vector3D& Vector3D::operator-=(Vector3D other) {
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;
    return *this;
};

Vector3D& Vector3D::operator*=(Vector3D other) {
    X *= other.X;
    Y *= other.Y;
    Z *= other.Z;
    return *this;
};

Vector3D& Vector3D::operator/=(Vector3D other) {
    X /= other.X;
    Y /= other.Y;
    Z /= other.Z;
    return *this;
};

Vector3D Vector3D::operator+(const Vector3D& v) const { 
    return Vector3D(*this) += v;
};

Vector3D Vector3D::operator-(const Vector3D& v) const {
    return Vector3D(*this) -= v;
};

Vector3D Vector3D::operator*(const Vector3D& v) const {
    return Vector3D(*this) *= v;
};

Vector3D Vector3D::operator/(const Vector3D& v) const {
    return Vector3D(*this) /= v;
};

Vector3D& Vector3D::operator*=(double scale) {
    X *= scale;
    Y *= scale;
    Z *= scale;
    return *this;
};

Vector3D& Vector3D::operator/=(double scale) {
    X /= scale;
    Y /= scale;
    Z /= scale;
    return *this;
};

bool Vector3D::operator==(const Vector3D& other) const {
    return (X == other.X && Y == other.Y && Z == other.Z);
};

bool Vector3D::operator!=(const Vector3D& other) const {
    return !(*this == other);
}

Vector3D Vector3D::operator*(double scale) const {
    return Vector3D(*this) *= scale;
};

Vector3D Vector3D::operator/(double scale) const {
    return Vector3D(*this) /= scale;
};

Vector3D Vector3D::cross(const Vector3D& other) const {
    return Vector3D(
        Y * other.Z - Z * other.Y,
        Z * other.X - X * other.Z,
        X * other.Y - Y * other.X
    );
};

Vector3D Vector3D::normalize() const {
    double len = length();
    if (len > 0) {
        return *this / len;
    }
    return Vector3D(0, 0, 0);
};
