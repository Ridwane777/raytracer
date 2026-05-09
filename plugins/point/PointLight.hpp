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
    Color   _color;

public:
    PointLight(const Point3D& position, double intensity, const Color& color = Color(1, 1, 1));
    ~PointLight() override = default;

    Color computeContribution(const HitInfo& hit, const IPrimitive& scene) const override;
};

#endif /* !POINTLIGHT_HPP_ */
