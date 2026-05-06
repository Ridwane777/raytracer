#include "DirectionalLight.hpp"

DirectionalLight::DirectionalLight(const Vector3D& dir, double intensity)
    : _toLight(dir.normalize() * -1.0)  // on stocke la direction vers la lumière
    , _intensity(intensity)
{}

Color DirectionalLight::computeContribution(const HitInfo& hit,
                                            const IPrimitive& scene) const
{
    Point3D shadowOrigin = hit.point + hit.normal * 1e-4;
    Ray shadowRay(shadowOrigin, _toLight);

    if (scene.hits(shadowRay).has_value())
        return Color(0.0, 0.0, 0.0);   // dans l'ombre

    double diff = hit.normal.dot(_toLight);
    if (diff < 0.0)
        diff = 0.0;

    double contribution = _intensity * diff;
    return Color(contribution, contribution, contribution);
}

extern "C" ILight* createLight(const std::map<std::string, double>& params)
{
    double dx = params.count("dir_x")    ? params.at("dir_x")    : -1.0;
    double dy = params.count("dir_y")    ? params.at("dir_y")    : -1.0;
    double dz = params.count("dir_z")    ? params.at("dir_z")    : -1.0;
    double intensity = params.count("intensity") ? params.at("intensity") : 0.6;

    return new DirectionalLight(Vector3D(dx, dy, dz), intensity);
}

extern "C" void destroyLight(ILight* light)
{
    delete light;
}