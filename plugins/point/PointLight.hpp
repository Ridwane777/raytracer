#ifndef POINTLIGHT_HPP_
#define POINTLIGHT_HPP_

#include "../../src/interfaces/ILight.hpp"
#include "../../src/math/Point3D.hpp"
#include <map>
#include <string>

class PointLight : public ILight {
private:
    Point3D _position;
    double  _intensity;

public:
    PointLight(const Point3D& position, double intensity);
    ~PointLight() override = default;

    Color computeContribution(const HitInfo& hit, const IPrimitive& scene) const override;
};

#endif /* !POINTLIGHT_HPP_ */
