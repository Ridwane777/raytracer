#include "Camera.hpp"

Camera::Camera() : position(), rotation(), width(0), height(0), fov(0.0) 
{
}

Camera::Camera(const Point3D& position, const Vector3D& rotation,
           int width, int height, double fov) : position(position), rotation(rotation), 
           width(width), height(height), fov(fov)
{
}

Camera::Camera(const Camera& other)
    : position(other.position), 
      rotation(other.rotation), 
      width(other.width), 
      height(other.height), 
      fov(other.fov)
{
}

Camera& Camera::operator=(const Camera& other) {
    if (this != &other) {
        position = other.position;
        rotation = other.rotation;
        width = other.width;
        height = other.height;
        fov = other.fov;
    }
    return *this;
}

Rectangle3D Camera::getScreen() const {
    double ratio = (double)width / (double)height;
    double halfHeight = tan((fov * M_PI / 180.0) / 2.0);
    double halfWidth = ratio * halfHeight;

    // Plan image centré à distance 1 devant la caméra (axe -Z par défaut)
    Point3D  origin      = Point3D(position.X - halfWidth, position.Y - halfHeight, position.Z - 1.0);
    Vector3D bottom_side = Vector3D((2.0 * halfWidth), 0.0, 0.0);
    Vector3D left_side   = Vector3D(0.0, (2.0 * halfHeight), 0.0);

    return Rectangle3D(origin, bottom_side, left_side);
}

Ray Camera::getRayForPixel(int px, int py) const {
    Rectangle3D screen = getScreen();

    double u = (double)px / (double)(width  - 1);
    double v = (double)py / (double)(height - 1);

    Point3D  target    = screen.pointAt(u, v);
    Vector3D direction = (target - position).normalize();
    return Ray(position, direction);
}