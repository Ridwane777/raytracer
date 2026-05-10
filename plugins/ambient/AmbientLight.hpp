#ifndef AMBIENTLIGHT_HPP_
#define AMBIENTLIGHT_HPP_

#include "../../src/interfaces/ILight.hpp"
#include <map>
#include <string>

class AmbientLight : public ILight {
public:
    explicit AmbientLight(double intensity = 0.4);
    ~AmbientLight() override = default;

    Color computeContribution(const HitInfo& hit,
                              const IPrimitive& scene) const override;

private:
    double _intensity;
};

extern "C" ILight* createLight(const std::map<std::string, double>& params);

#endif 