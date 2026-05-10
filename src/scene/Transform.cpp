#include "Transform.hpp"

Transform::Transform(std::shared_ptr<IPrimitive> primitive, const Matrix4x4& transform) 
    : _wrapped(primitive), _worldToLocal(transform.inverse()), _localToWorld(transform)
{}

std::optional<double> Transform::hits(const Ray& ray) const {
    Ray localRay = _worldToLocal.transformRay(ray);
    return _wrapped->hits(localRay);
}

Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _worldToLocal.transpose().transformVector(localNormal).normalize();
}

Color Transform::getColor(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    return _wrapped->getColor(localPoint);
}

double Transform::getOpacity() const {
    return _wrapped->getOpacity();
}

double Transform::getRefractiveIndex() const {
    return _wrapped->getRefractiveIndex();
}

double Transform::getReflectivity() const {
    return _wrapped->getReflectivity();
}
