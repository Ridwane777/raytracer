#include "Sphere.hpp"

Sphere::Sphere(const Point3D& center, double radius, const Color& color,
                   double transparency, double refractiveIndex, double reflectivity)
    : _center(center), _radius(radius), _color(color),
      _transparency(transparency), _refractiveIndex(refractiveIndex), _reflectivity(reflectivity)
{
}

std::optional<double> Sphere::hits(const Ray& ray) const
{
    Vector3D oc = ray.origin - _center;

    double a = ray.direction.dot(ray.direction);
    double b = 2.0 * ray.direction.dot(oc);
    double c = oc.dot(oc) - _radius * _radius;

    double discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
        return std::nullopt;

    double sqrtD = std::sqrt(discriminant);

    double t1 = (-b - sqrtD) / (2.0 * a);
    double t2 = (-b + sqrtD) / (2.0 * a);

    constexpr double EPSILON = 1e-4;

    if (t1 > EPSILON) return t1;
    if (t2 > EPSILON) return t2;

    return std::nullopt;
}

Vector3D Sphere::getNormal(const Point3D& point) const
{
    return (point - _center).normalize();
}

Color Sphere::getColor(const Point3D& point) const
{
    (void)point;
    return _color;
}

double Sphere::getOpacity() const
{
    return 1.0 - _transparency;
}

double Sphere::getRefractiveIndex() const
{
    return _refractiveIndex;
}

double Sphere::getReflectivity() const
{
    return _reflectivity;
}

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    Point3D center(params.at("x"), params.at("y"), params.at("z"));
    double  radius = params.at("r");
    double  transparency = params.count("transparency") ? params.at("transparency") : 0.0;
    double  refractiveIndex = params.count("refractiveIndex") ? params.at("refractiveIndex") : 1.0;
    double  reflectivity = params.count("reflectivity") ? params.at("reflectivity") : 0.0;
    Color   color(params.at("color_r"), params.at("color_g"), params.at("color_b"));

    return new Sphere(center, radius, color, transparency, refractiveIndex, reflectivity);
}

extern "C" void destroyPrimitive(IPrimitive* p)
{
    delete p;
}