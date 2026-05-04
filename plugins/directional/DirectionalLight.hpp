#ifndef DIRECTIONALLIGHT_HPP_
#define DIRECTIONALLIGHT_HPP_

#include "../../src/interfaces/ILight.hpp"

class DirectionalLight : public ILight {
public:
    DirectionalLight(const Vector3D& dir, double intensity = 0.6);
    ~DirectionalLight() override = default;

    Color computeContribution(const HitInfo& hit, const IPrimitive& scene) const override;

private:
    Vector3D _toLight;   // normalised direction towards the light source
    double   _intensity;
};

extern "C" ILight *createLight();

#endif /* !DIRECTIONALLIGHT_HPP_ */