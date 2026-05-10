#include "Cone.hpp"

Cone::Cone(const Point3D& center, double radius, double height, double angle, const Color& color,
           double transparency, double refractiveIndex, double reflectivity)
    : _center(center), _radius(radius), _height(height), _angle(angle), _color(color),
      _transparency(transparency), _refractiveIndex(refractiveIndex), _reflectivity(reflectivity)
{
}

/*Cone::~Cone()
{
}*/

std::optional<double> Cone::hits(const Ray& ray) const
{
    double k  = std::tan(_angle * M_PI / 180.0);
    double k2 = k * k;

    double dx = ray.origin.X - _center.X;
    double dy = ray.origin.Y - _center.Y;
    double dz = ray.origin.Z - _center.Z;

    double a = ray.direction.X * ray.direction.X
             - k2 * ray.direction.Y * ray.direction.Y
             + ray.direction.Z * ray.direction.Z;

    double b = 2.0 * (dx * ray.direction.X
                    - k2 * dy * ray.direction.Y
                    + dz * ray.direction.Z);

    double c = dx * dx - k2 * dy * dy + dz * dz;

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

Vector3D Cone::getNormal(const Point3D& point) const
{
    double k  = std::tan(_angle * M_PI / 180.0);
    double dx = point.X - _center.X;
    double dz = point.Z - _center.Z;
    //double dy = point.Y - _center.Y;

    return Vector3D(dx, -k * std::sqrt(dx*dx + dz*dz), dz).normalize();
}

Color Cone::getColor(const Point3D& point) const
{
    (void)point;
    return _color;
}

double Cone::getOpacity() const
{
    return 1.0 - _transparency;
}

double Cone::getRefractiveIndex() const
{
    return _refractiveIndex;
}

double Cone::getReflectivity() const
{
    return _reflectivity;
}


extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    Point3D center(params.at("x"), params.at("y"), params.at("z"));
    double  radius = params.at("r");
    double  height = params.at("height");
    double  angle  = params.at("angle");
    double  transparency = params.count("transparency") ? params.at("transparency") : 0.0;
    double  refractiveIndex = params.count("refractiveIndex") ? params.at("refractiveIndex") : 1.0;
    double  reflectivity = params.count("reflectivity") ? params.at("reflectivity") : 0.0;
    Color   color(params.at("color_r"), params.at("color_g"), params.at("color_b"));

    return new Cone(center, radius, height, angle, color, transparency, refractiveIndex, reflectivity);
}

extern "C" void destroyPrimitive(IPrimitive* p) { delete p; }