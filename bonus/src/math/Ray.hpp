#ifndef RAY_HPP_
#define RAY_HPP_
#include "Point3D.hpp"

class Ray {
    public:
        Point3D  origin;
        Vector3D direction;

        Ray() = default;
        Ray(const Point3D& origin, const Vector3D& direction);
        ~Ray() = default;

        Point3D at(double t) const;
};

#endif 
