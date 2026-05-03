#include "Plane.hpp"

Plane::Plane(char axis, double position, const Color& color) : _axis(axis), _position(position), _color(color)
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

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    char   axis     = static_cast<char>(params.at("axis"));
    double position = params.at("position");
    Color  color(params.at("r"), params.at("g"), params.at("b"));

    return new Plane(axis, position, color);
}

extern "C" void destroyPrimitive(IPrimitive* p)
{
    delete p;
}