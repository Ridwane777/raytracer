#ifndef TORUS_HPP_
#define TORUS_HPP_

#include "../../src/interfaces/IPrimitive.hpp"
#include "../../src/math/Point3D.hpp"
#include "../../src/math/Color.hpp"
#include <optional>
#include <map>
#include <vector>

class Torus : public IPrimitive {
public:
    Torus(const Point3D& center, double r1, double r2, const Color& color,
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
    double  _r1; // Major radius
    double  _r2; // Minor radius
    Color   _color;
    double  _transparency;
    double  _refractiveIndex;
    double  _reflectivity;
    double  _shininess;
    Color   _specularColor;

    // Helper functions for solving quartic equations
    static int solveQuadric(double c[3], double s[2]);
    static int solveCubic(double c[4], double s[3]);
    static int solveQuartic(double c[5], double s[4]);
};

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void        destroyPrimitive(IPrimitive* p);

#endif
