#ifndef CYLINDER_HPP_
#define CYLINDER_HPP_

#include "../../src/interfaces/IPrimitive.hpp"
#include "../../src/math/Point3D.hpp"
#include "../../src/math/Color.hpp"
#include <optional>
#include <map>
#include <cmath>

class Cylinder : public IPrimitive {
public:
    Cylinder(const Point3D& center, double radius,
             double height, const Color& color,
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
    Point3D _center;
    double  _radius;
    double  _height;
    Color   _color;
    double  _transparency;
    double  _refractiveIndex;
    double  _reflectivity;
    double  _shininess;
    Color   _specularColor;
};

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void        destroyPrimitive(IPrimitive* p);

#endif 
