#include "DirectionalLight.hpp"

DirectionalLight::DirectionalLight(const Vector3D& dir, double intensity) : _toLight(dir.normalize() * -1.0), _intensity(intensity)
{
}

Color DirectionalLight::computeContribution(const HitInfo& hit, const IPrimitive& scene) const
{
    Point3D shadowOrigin = hit.point + hit.normal * 1e-4;
    Ray shadowRay(shadowOrigin, _toLight);

    if (scene.hits(shadowRay).has_value())
        return Color(0.0, 0.0, 0.0);

    double diff = hit.normal.dot(_toLight);
    if (diff < 0.0)
        diff = 0.0;

    double contribution = _intensity * diff;
    return Color(contribution, contribution, contribution);
}

extern "C" ILight *createLight()
{
    return new DirectionalLight(Vector3D(-1.0, -1.0, -1.0));
}