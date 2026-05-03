#include "Sphere.hpp"

Sphere::Sphere(const Point3D& center, double radius, const Color& color) : _center(center), _radius(radius), _color(color)
{
}

/*Sphere::~Sphere()
{
}*/

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

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    Point3D center(params.at("x"), params.at("y"), params.at("z"));
    double  radius = params.at("radius");
    Color   color(params.at("r"), params.at("g"), params.at("b"));

    return new Sphere(center, radius, color);
}

extern "C" void destroyPrimitive(IPrimitive* p)
{
    delete p;
}