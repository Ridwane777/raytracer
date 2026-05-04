#ifndef AMBIENTLIGHT_HPP_
#define AMBIENTLIGHT_HPP_

#include "../../src/interfaces/ILight.hpp"

class AmbientLight : public ILight {
public:
    explicit AmbientLight(double intensity = 0.4);
    ~AmbientLight() override = default;

    Color computeContribution(const HitInfo& hit,
                              const IPrimitive& scene) const override;

private:
    double _intensity;
};

extern "C" ILight *createLight();

#endif /* !AMBIENTLIGHT_HPP_ */