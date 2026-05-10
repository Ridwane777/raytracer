#include "Cylinder.hpp"

Cylinder::Cylinder(const Point3D& center, double radius, double height, const Color& color,
                   double transparency, double refractiveIndex, double reflectivity)
    : _center(center), _radius(radius), _height(height), _color(color),
      _transparency(transparency), _refractiveIndex(refractiveIndex), _reflectivity(reflectivity)
{
}

std::optional<double> Cylinder::hits(const Ray& ray) const
{
    double a = ray.direction.X * ray.direction.X
             + ray.direction.Z * ray.direction.Z;

    if (std::abs(a) < 1e-8)
        return std::nullopt;

    double b = 2.0 * (ray.origin.X * ray.direction.X
                    + ray.origin.Z * ray.direction.Z);
    double c = ray.origin.X * ray.origin.X
             + ray.origin.Z * ray.origin.Z
             - _radius * _radius;

    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0)
        return std::nullopt;

    double sqrtD = std::sqrt(discriminant);
    double t1 = (-b - sqrtD) / (2.0 * a);
    double t2 = (-b + sqrtD) / (2.0 * a);

    constexpr double EPSILON = 1e-4;

    for (double t : {t1, t2}) {
        if (t < EPSILON) continue;

        double y = ray.origin.Y + t * ray.direction.Y;
        if (y < _center.Y || y > _center.Y + _height)
            continue;

        return t;
    }
    return std::nullopt;
}

Vector3D Cylinder::getNormal(const Point3D& point) const
{
    return Vector3D(point.X - _center.X, 0, point.Z - _center.Z).normalize();
}

Color Cylinder::getColor(const Point3D& point) const
{
    (void)point;
    return _color;
}

double Cylinder::getOpacity() const
{
    return 1.0 - _transparency;
}

double Cylinder::getRefractiveIndex() const
{
    return _refractiveIndex;
}

double Cylinder::getReflectivity() const
{
    return _reflectivity;
}

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    Point3D center(params.at("x"), params.at("y"), params.at("z"));
    double  radius = params.at("r");
    double  height = params.at("height");
    double  transparency = params.count("transparency") ? params.at("transparency") : 0.0;
    double  refractiveIndex = params.count("refractiveIndex") ? params.at("refractiveIndex") : 1.0;
    double  reflectivity = params.count("reflectivity") ? params.at("reflectivity") : 0.0;
    Color   color(params.at("color_r"), params.at("color_g"), params.at("color_b"));

    return new Cylinder(center, radius, height, color, transparency, refractiveIndex, reflectivity);
}

extern "C" void destroyPrimitive(IPrimitive* p) { delete p; }