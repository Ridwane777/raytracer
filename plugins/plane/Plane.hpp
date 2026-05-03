#ifndef PLANE_HPP_
#define PLANE_HPP_

#include "../../src/interfaces/IPrimitive.hpp"
#include "../../src/math/Color.hpp"
#include <optional>
#include <iostream>
#include <map>
#include <cmath>

class Plane : public IPrimitive {
public:
    Plane(char axis, double position, const Color& color);

    std::optional<double> hits(const Ray& ray) const override;
    Vector3D getNormal(const Point3D& point) const override;
    Color getColor(const Point3D& point) const override;

private:
    char _axis;
    double _position;
    Color _color;
};

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void destroyPrimitive(IPrimitive* p);

#endif /* !PLANE_HPP_ */
