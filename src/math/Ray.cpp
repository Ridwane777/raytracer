#include "Ray.hpp"

Ray::Ray(const Point3D& origin, const Vector3D& direction) : origin(origin), direction(direction) {}

Point3D Ray::at(double t) const {
    
    return origin + (direction * t);
}