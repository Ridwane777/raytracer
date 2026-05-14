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
    Plane(char axis, double position, const Color& color,
          double transparency, double refractiveIndex, double reflectivity,
          double shininess, const Color& specularColor);

    std::optional<double> hits(const Ray& ray) const override;
    Vector3D getNormal(const Point3D& point) const override;
    Color getColor(const Point3D& point) const override;
    double getOpacity() const override;
    double getRefractiveIndex() const override;
    double getReflectivity() const override;
    double getShininess() const override;
    Color getSpecularColor() const override;

private:
    char _axis;
    double _position;
    Color _color;
    double _transparency;
    double _refractiveIndex;
    double _reflectivity;
    double _shininess;
    Color  _specularColor;
};

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void destroyPrimitive(IPrimitive* p);

#endif 
