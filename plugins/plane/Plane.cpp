#include "Plane.hpp"

Plane::Plane(char axis, double position, const Color& color,
             double transparency, double refractiveIndex, double reflectivity)
    : _axis(axis), _position(position), _color(color),
      _transparency(transparency), _refractiveIndex(refractiveIndex), _reflectivity(reflectivity)
{
}

/*Plane::~Plane()
{
}*/

std::optional<double> Plane::hits(const Ray& ray) const
{
    double origin_axis;
    double dir_axis;

    if (_axis == 'X') {
        origin_axis = ray.origin.X;
        dir_axis    = ray.direction.X;
    } else if (_axis == 'Y') {
        origin_axis = ray.origin.Y;
        dir_axis    = ray.direction.Y;
    } else {
        origin_axis = ray.origin.Z;
        dir_axis    = ray.direction.Z;
    }

    if (std::abs(dir_axis) < 1e-8)
        return std::nullopt; // rayon parallèle au plan

    double t = (_position - origin_axis) / dir_axis;

    if (t < 1e-4)
        return std::nullopt;

    return t;
}

Vector3D Plane::getNormal(const Point3D& point) const
{
    (void)point;
    if (_axis == 'X') return Vector3D(1, 0, 0);
    if (_axis == 'Y') return Vector3D(0, 1, 0);
    return Vector3D(0, 0, 1);
}

Color Plane::getColor(const Point3D& point) const
{
    (void)point;
    return _color;
}

double Plane::getOpacity() const
{
    return 1.0 - _transparency;
}

double Plane::getRefractiveIndex() const
{
    return _refractiveIndex;
}

double Plane::getReflectivity() const
{
    return _reflectivity;
}

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    double axisVal = params.at("axis");
    char   axis    = 'Y'; // default
    if (axisVal == 0.0)      axis = 'X';
    else if (axisVal == 2.0) axis = 'Z';

    double position = params.at("position");
    double transparency = params.count("transparency") ? params.at("transparency") : 0.0;
    double refractiveIndex = params.count("refractiveIndex") ? params.at("refractiveIndex") : 1.0;
    double reflectivity = params.count("reflectivity") ? params.at("reflectivity") : 0.0;
    Color  color(params.at("color_r"), params.at("color_g"), params.at("color_b"));

    return new Plane(axis, position, color, transparency, refractiveIndex, reflectivity);
}

extern "C" void destroyPrimitive(IPrimitive* p)
{
    delete p;
}