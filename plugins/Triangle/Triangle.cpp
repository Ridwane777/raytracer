#include "Triangle.hpp"
#include <cmath>
#include <map>
#include <string>

Triangle::Triangle(const Point3D &a, const Point3D &b, const Point3D &c,
                   const Color &color)
    : _a(a), _b(b), _c(c), _color(color)
{
    Vector3D ab = b - a;
    Vector3D ac = c - a;
    _normal = ab.cross(ac).normalize();
}

std::optional<double> Triangle::hits(const Ray &ray) const
{
    const double EPSILON = 1e-8;

    Vector3D ab = _b - _a;
    Vector3D ac = _c - _a;

    Vector3D h = ray.direction.cross(ac);
    double a = ab.dot(h);

    if (std::fabs(a) < EPSILON)
        return std::nullopt;

    double f = 1.0 / a;
    Vector3D s = ray.origin - _a;
    double u = f * s.dot(h);

    if (u < 0.0 || u > 1.0)
        return std::nullopt;

    Vector3D q = s.cross(ab);
    double v = f * ray.direction.dot(q);

    if (v < 0.0 || u + v > 1.0)
        return std::nullopt;

    double t = f * ac.dot(q);
    if (t < EPSILON)
        return std::nullopt;

    return t;
}

Vector3D Triangle::getNormal(const Point3D &point) const
{
    (void)point;
    return _normal;
}

Color Triangle::getColor(const Point3D &point) const
{
    (void)point;
    return _color;
}

extern "C" IPrimitive *createPrimitive(const std::map<std::string, double> &params)
{
    double ax = params.count("ax") ? params.at("ax") : 0.0;
    double ay = params.count("ay") ? params.at("ay") : 1.0;
    double az = params.count("az") ? params.at("az") : 0.0;

    double bx = params.count("bx") ? params.at("bx") : -1.0;
    double by = params.count("by") ? params.at("by") : -1.0;
    double bz = params.count("bz") ? params.at("bz") : 0.0;

    double cx = params.count("cx") ? params.at("cx") : 1.0;
    double cy = params.count("cy") ? params.at("cy") : -1.0;
    double cz = params.count("cz") ? params.at("cz") : 0.0;

    double r = params.count("color_r") ? params.at("color_r") : 1.0;
    double g = params.count("color_g") ? params.at("color_g") : 0.0;
    double b = params.count("color_b") ? params.at("color_b") : 0.0;

    return new Triangle(
        Point3D(ax, ay, az),
        Point3D(bx, by, bz),
        Point3D(cx, cy, cz),
        Color(r, g, b));
}

extern "C" void destroyPrimitive(IPrimitive *primitive) { delete primitive; }