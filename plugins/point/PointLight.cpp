#include "PointLight.hpp"

PointLight::PointLight(const Point3D& position, double intensity, const Color& color)
    : _position(position), _intensity(intensity), _color(color)
{}

Color PointLight::computeContribution(const HitInfo& hit, const IPrimitive& scene) const
{
    Vector3D toLight = (_position - hit.point).normalize();
    double distanceToLight = (_position - hit.point).length();

    Point3D shadowOrigin = hit.point + hit.normal * 1e-4;
    Ray shadowRay(shadowOrigin, toLight);

    auto hitDist = scene.hits(shadowRay);
    if (hitDist.has_value() && hitDist.value() < distanceToLight) {
        return Color(0.0, 0.0, 0.0);
    }

    double diff = hit.normal.dot(toLight);
    if (diff < 0.0) diff = 0.0;
    
    double contribution = _intensity * diff;
    return Color(_color.r * contribution, _color.g * contribution, _color.b * contribution);
}

extern "C" ILight* createLight(const std::map<std::string, double>& params)
{
    double x = params.count("x") ? params.at("x") : 0.0;
    double y = params.count("y") ? params.at("y") : 0.0;
    double z = params.count("z") ? params.at("z") : 0.0;
    double intensity = params.count("intensity") ? params.at("intensity") : 0.6;
    double r = params.count("color_r") ? params.at("color_r") : 1.0;
    double g = params.count("color_g") ? params.at("color_g") : 1.0;
    double b = params.count("color_b") ? params.at("color_b") : 1.0;

    return new PointLight(Point3D(x, y, z), intensity, Color(r, g, b));
}

extern "C" void destroyLight(ILight* light)
{
    delete light;
}
