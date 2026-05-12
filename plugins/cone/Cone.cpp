#include "Cone.hpp"

Cone::Cone(const Point3D& center, double radius,
           double height, double angle, const Color& color,
           double transparency, double refractiveIndex, double reflectivity,
           double shininess, const Color& specularColor)
    : _center(center), _radius(radius), _height(height), _angle(angle), _color(color),
      _transparency(transparency), _refractiveIndex(refractiveIndex), _reflectivity(reflectivity),
      _shininess(shininess), _specularColor(specularColor)
{
}

std::optional<double> Cone::hits(const Ray& ray) const
{
    double k = std::tan(_angle * M_PI / 180.0);
    k = k * k;

    double a = ray.direction.X * ray.direction.X + ray.direction.Z * ray.direction.Z - k * ray.direction.Y * ray.direction.Y;
    double b = 2.0 * (ray.direction.X * (ray.origin.X - _center.X) + ray.direction.Z * (ray.origin.Z - _center.Z) - k * ray.direction.Y * (ray.origin.Y - (_center.Y + _height)));
    double c = (ray.origin.X - _center.X) * (ray.origin.X - _center.X) + (ray.origin.Z - _center.Z) * (ray.origin.Z - _center.Z) - k * (ray.origin.Y - (_center.Y + _height)) * (ray.origin.Y - (_center.Y + _height));

    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return std::nullopt;

    double t = (-b - std::sqrt(discriminant)) / (2.0 * a);
    if (t < 0.0) t = (-b + std::sqrt(discriminant)) / (2.0 * a);

    if (t < 0.0) return std::nullopt;

    double y = ray.origin.Y + t * ray.direction.Y;
    if (y < _center.Y || y > _center.Y + _height) return std::nullopt;

    return t;
}

Vector3D Cone::getNormal(const Point3D& point) const
{
    double k = std::tan(_angle * M_PI / 180.0);
    Vector3D n(point.X - _center.X, -k * (point.Y - (_center.Y + _height)), point.Z - _center.Z);
    return n.normalize();
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

double Cone::getShininess() const
{
    return _shininess;
}

Color Cone::getSpecularColor() const
{
    return _specularColor;
}

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    Point3D center(params.at("x"), params.at("y"), params.at("z"));
    double  radius = params.at("r");
    double  height = params.at("height");
    double  angle = params.at("angle");
    double  transparency = params.count("transparency") ? params.at("transparency") : 0.0;
    double  refractiveIndex = params.count("refractiveIndex") ? params.at("refractiveIndex") : 1.0;
    double  reflectivity = params.count("reflectivity") ? params.at("reflectivity") : 0.0;
    Color   color(params.at("color_r"), params.at("color_g"), params.at("color_b"));
    double  shininess = params.count("shininess") ? params.at("shininess") : 0.0;
    Color   spec(1, 1, 1);
    if (params.count("spec_r") && params.count("spec_g") && params.count("spec_b")) {
        spec = Color(params.at("spec_r"), params.at("spec_g"), params.at("spec_b"));
    }

    return new Cone(center, radius, height, angle, color, transparency, refractiveIndex, reflectivity, shininess, spec);
}

extern "C" void destroyPrimitive(IPrimitive* p)
{
    delete p;
}