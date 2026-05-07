#include "Cylinder.hpp"

Cylinder::Cylinder(const Point3D& center, double radius, double height, const Color& color) : _center(center), _radius(radius), _height(height), _color(color)
{
}
/*
Cylinder::~Cylinder()
{
}*/

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

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    // Cles alignees avec SceneBuilder :
    //   "x","y","z"     -> position du centre
    //   "r"             -> rayon
    //   "height"        -> hauteur
    //   "color_r/g/b"   -> couleur (0.0 - 1.0)
    Point3D center(params.at("x"), params.at("y"), params.at("z"));
    double  radius = params.at("r");
    double  height = params.at("height");
    Color   color(params.at("color_r"), params.at("color_g"), params.at("color_b"));

    return new Cylinder(center, radius, height, color);
}

extern "C" void destroyPrimitive(IPrimitive* p) { delete p; }