#include "Matrix4x4.hpp"

Matrix4x4::Matrix4x4()
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }
}

Matrix4x4 Matrix4x4::identity() {
    return Matrix4x4();
}

Matrix4x4 Matrix4x4::translation(const Vector3D& t) {
    Matrix4x4 trs;

    trs.m[0][3] = t.X;
    trs.m[1][3] = t.Y;
    trs.m[2][3] = t.Z;
    return trs;
}

Matrix4x4 Matrix4x4::rotationX(double deg) {
    Matrix4x4 rot;
    double radians = deg * M_PI / 180.0;
    double c = cos(radians);
    double s = sin(radians);

    rot.m[1][1] = c;
    rot.m[1][2] = -s;
    rot.m[2][1] = s;
    rot.m[2][2] = c;
    return rot;
}

Matrix4x4 Matrix4x4::rotationY(double deg) {
    Matrix4x4 rot;
    double radians = deg * M_PI / 180.0;
    double c = cos(radians);
    double s = sin(radians);

    rot.m[0][0] = c;
    rot.m[0][2] = s;
    rot.m[2][0] = -s;
    rot.m[2][2] = c;
    return rot;
}

Matrix4x4 Matrix4x4::rotationZ(double deg) {
    Matrix4x4 rot;
    double radians = deg * M_PI / 180.0;
    double c = cos(radians);
    double s = sin(radians);

    rot.m[0][0] = c;
    rot.m[0][1] = -s;
    rot.m[1][0] = s;
    rot.m[1][1] = c;
    return rot;
}

Matrix4x4 Matrix4x4::scale(double sx, double sy, double sz) {
    Matrix4x4 scl;

    scl.m[0][0] = sx;
    scl.m[1][1] = sy;
    scl.m[2][2] = sz;
    return scl;
}

Matrix4x4 Matrix4x4::shear(double xy, double xz, double yx, double yz, double zx, double zy) {
    Matrix4x4 shr;

    shr.m[0][1] = xy;
    shr.m[0][2] = xz;
    shr.m[1][0] = yx;
    shr.m[1][2] = yz;
    shr.m[2][0] = zx;
    shr.m[2][1] = zy;
    return shr;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 res;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double sum = 0.0;
            for (int k = 0; k < 4; k++) {
                sum += this->m[i][k] * other.m[k][j];
            }
            res.m[i][j] = sum;
        }
    }
    return res;
}

Matrix4x4 Matrix4x4::inverse() const {
    Matrix4x4 res = identity();
    double temp[4][4];

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            temp[i][j] = m[i][j];

    for (int i = 0; i < 4; i++) {
        int pivotRow = i;
        for (int k = i + 1; k < 4; k++) {
            if (std::abs(temp[k][i]) > std::abs(temp[pivotRow][i]))
                pivotRow = k;
        }

        if (pivotRow != i) {
            for (int j = 0; j < 4; j++) {
                std::swap(temp[i][j], temp[pivotRow][j]);
                std::swap(res.m[i][j], res.m[pivotRow][j]);
            }
        }
        double pivot = temp[i][i];
        if (pivot == 0)
            return identity();

        for (int j = 0; j < 4; j++) {
            temp[i][j] /= pivot;
            res.m[i][j] /= pivot;
        }
        for (int k = 0; k < 4; k++) {
            if (k != i) {
                double factor = temp[k][i];
                for (int j = 0; j < 4; j++) {
                    temp[k][j] -= factor * temp[i][j];
                    res.m[k][j] -= factor * res.m[i][j];
                }
            }
        }
    }
    return res;
}

Matrix4x4 Matrix4x4::transpose() const {
    Matrix4x4 res;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res.m[i][j] = m[j][i];
        }
    }
    return res;
}

Point3D Matrix4x4::transformPoint(const Point3D& p) const {
    Point3D new_point;

    new_point.X = m[0][0] * p.X +  m[0][1] * p.Y +  m[0][2] * p.Z + m[0][3];
    new_point.Y = m[1][0] * p.X +  m[1][1] * p.Y +  m[1][2] * p.Z + m[1][3];
    new_point.Z = m[2][0] * p.X +  m[2][1] * p.Y +  m[2][2] * p.Z + m[2][3];
    return new_point;
}

Vector3D Matrix4x4::transformVector(const Vector3D& v) const {
    Vector3D new_vec;

    new_vec.X = m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z;
    new_vec.Y = m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z;
    new_vec.Z = m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z;

    return new_vec;
}

Ray Matrix4x4::transformRay(const Ray& ray) const {
    Ray r;

    r.origin = transformPoint(ray.origin);
    r.direction = transformVector(ray.direction).normalize();
    return r;
}