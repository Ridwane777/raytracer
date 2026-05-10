#ifndef DIRECTIONALLIGHT_HPP_
#define DIRECTIONALLIGHT_HPP_
#include "../../src/interfaces/ILight.hpp"
#include <map>
#include <string>

class DirectionalLight : public ILight {
public:
    DirectionalLight(const Vector3D& dir, double intensity = 0.6, const Color& color = Color(1, 1, 1));
    ~DirectionalLight() override = default;

    Color computeContribution(const HitInfo& hit,
                              const IPrimitive& scene) const override;

private:
    Vector3D _toLight;
    double   _intensity;
    Color    _color;
};

extern "C" ILight* createLight(const std::map<std::string, double>& params);

#endif // DIRECTIONALLIGHT_HPP_