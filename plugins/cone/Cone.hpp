#ifndef CONE_HPP_
#define CONE_HPP_

#include "../../src/interfaces/IPrimitive.hpp"
#include "../../src/math/Point3D.hpp"
#include "../../src/math/Color.hpp"
#include <optional>
#include <map>
#include <cmath>

class Cone : public IPrimitive {
public:
    Cone(const Point3D& center, double radius,
         double height, double angle, const Color& color);

    std::optional<double> hits(const Ray& ray) const override;
    Vector3D getNormal(const Point3D& point) const override;
    Color getColor(const Point3D& point) const override;

private:
    Point3D _center;
    double  _radius;
    double  _height;
    double  _angle;
    Color   _color;
};

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void        destroyPrimitive(IPrimitive* p);

#endif /* !CONE_HPP_ */
