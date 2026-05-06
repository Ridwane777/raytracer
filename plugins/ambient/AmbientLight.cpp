#include "AmbientLight.hpp"

AmbientLight::AmbientLight(double intensity)
    : _intensity(intensity)
{}

Color AmbientLight::computeContribution(const HitInfo& hit,
                                        const IPrimitive& scene) const
{
    (void)hit;
    (void)scene;
    return Color(_intensity, _intensity, _intensity);
}

extern "C" ILight* createLight(const std::map<std::string, double>& params)
{
    double intensity = params.count("intensity") ? params.at("intensity") : 0.4;
    return new AmbientLight(intensity);
}

extern "C" void destroyLight(ILight* light)
{
    delete light;
}