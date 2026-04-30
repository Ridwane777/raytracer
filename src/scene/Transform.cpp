#include "Transform.hpp"

Transform::Transform(IPrimitive* primitive, const Matrix4x4& transform) 
    : _wrapped(primitive), _localToWorld(transform), _worldToLocal(transform.inverse())
{}

std::optional<double> Transform::hits(const Ray& ray) const {
    Ray localRay = _worldToLocal.transformRay(ray);
    return _wrapped->hits(localRay);
}

Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _localToWorld.transformVector(localNormal).normalize();
}

Color Transform::getColor(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    return _wrapped->getColor(localPoint);
}
