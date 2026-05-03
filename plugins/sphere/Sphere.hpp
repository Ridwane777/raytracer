#ifndef SPHERE_HPP_
#define SPHERE_HPP_

#include "../../src/interfaces/IPrimitive.hpp"
#include "../../src/math/Point3D.hpp"
#include "../../src/math/Color.hpp"
#include <optional>
#include <iostream>
#include <map>
#include <cmath>

class Sphere : public IPrimitive {
public:
    Sphere(const Point3D& center, double radius, const Color& color);

    std::optional<double> hits(const Ray& ray) const override;
    Vector3D getNormal(const Point3D& point) const override;
    Color getColor(const Point3D& point) const override;

private:
    Point3D _center;
    double _radius;
    Color _color;
};

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void destroyPrimitive(IPrimitive* p);

#endif /* !SPHERE_HPP_ */
