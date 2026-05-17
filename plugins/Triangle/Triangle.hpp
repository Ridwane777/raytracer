#ifndef TRIANGLE_HPP_
#define TRIANGLE_HPP_

#include "../../src/interfaces/IPrimitive.hpp"
#include <map>
#include <string>

class Triangle : public IPrimitive
{
public:
    Triangle(const Point3D &a, const Point3D &b, const Point3D &c, const Color &color);
    ~Triangle() override = default;

    std::optional<double> hits(const Ray &ray) const override;
    Vector3D getNormal(const Point3D &point) const override;
    Color getColor(const Point3D &point) const override;
    double getOpacity() const override;
    double getRefractiveIndex() const override;
    double getReflectivity() const override;
    double getShininess() const override;
    Color getSpecularColor() const override;

private:
    Point3D _a, _b, _c;
    Vector3D _normal;
    Color _color;
    double _transparency;
    double _refractiveIndex;
    double _reflectivity;
    double _shininess;
    Color  _specularColor;
};

extern "C" IPrimitive *createPrimitive(const std::map<std::string, double> &params);
extern "C" void destroyPrimitive(IPrimitive *primitive);

#endif /* !TRIANGLE_HPP_ */