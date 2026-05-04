#include "AmbientLight.hpp"

AmbientLight::AmbientLight(double intensity)
    : _intensity(intensity) {}

Color AmbientLight::computeContribution(const HitInfo& hit,
                                        const IPrimitive& scene) const
{
    (void)hit;
    (void)scene;
    return Color(_intensity, _intensity, _intensity);
}

extern "C" ILight *createLight()
{
    return new AmbientLight();
}