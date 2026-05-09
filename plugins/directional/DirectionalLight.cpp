#include "DirectionalLight.hpp"

DirectionalLight::DirectionalLight(const Vector3D& dir, double intensity, const Color& color)
    : _toLight(dir.normalize() * -1.0)  // on stocke la direction vers la lumière
    , _intensity(intensity)
    , _color(color)
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
    return Color(_color.r * contribution, _color.g * contribution, _color.b * contribution);
}

extern "C" ILight* createLight(const std::map<std::string, double>& params)
{
    double dx = params.count("dir_x")    ? params.at("dir_x")    : -1.0;
    double dy = params.count("dir_y")    ? params.at("dir_y")    : -1.0;
    double dz = params.count("dir_z")    ? params.at("dir_z")    : -1.0;
    double intensity = params.count("intensity") ? params.at("intensity") : 0.6;
    double r = params.count("color_r") ? params.at("color_r") : 1.0;
    double g = params.count("color_g") ? params.at("color_g") : 1.0;
    double b = params.count("color_b") ? params.at("color_b") : 1.0;

    return new DirectionalLight(Vector3D(dx, dy, dz), intensity, Color(r, g, b));
}

extern "C" void destroyLight(ILight* light)
{
    delete light;
}