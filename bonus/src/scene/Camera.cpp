#include "Camera.hpp"
#include "../math/Matrix4x4.hpp"

Camera::Camera()
    : position(), rotation(), width(0), height(0), fov(0.0), antialiasing(1) {}

Camera::Camera(const Point3D &position, const Vector3D &rotation, int width,
               int height, double fov, int antialiasing)
    : position(position), rotation(rotation), width(width), height(height),
      fov(fov), antialiasing(antialiasing) {}

Camera::Camera(const Camera &other)
    : position(other.position), rotation(other.rotation), width(other.width),
      height(other.height), fov(other.fov), antialiasing(other.antialiasing) {}

Camera &Camera::operator=(const Camera &other) {
  if (this != &other) {
    position = other.position;
    rotation = other.rotation;
    width = other.width;
    height = other.height;
    fov = other.fov;
    antialiasing = other.antialiasing;
  }
  return *this;
}

Rectangle3D Camera::getScreen() const {
  double ratio = (double)width / (double)height;
  double halfHeight = tan((fov * M_PI / 180.0) / 2.0);
  double halfWidth = ratio * halfHeight;

  Matrix4x4 rotMatrix = Matrix4x4::rotationX(rotation.X) *
                        Matrix4x4::rotationY(rotation.Y) *
                        Matrix4x4::rotationZ(rotation.Z);

  Vector3D localOriginVec(-halfWidth, -halfHeight, -1.0);
  Vector3D localBottomVec((2.0 * halfWidth), 0.0, 0.0);
  Vector3D localLeftVec(0.0, (2.0 * halfHeight), 0.0);

  Vector3D rotatedOriginVec = rotMatrix.transformVector(localOriginVec);
  Vector3D bottom_side = rotMatrix.transformVector(localBottomVec);
  Vector3D left_side = rotMatrix.transformVector(localLeftVec);

  Point3D origin = position + rotatedOriginVec;

  return Rectangle3D(origin, bottom_side, left_side);
}

Ray Camera::getRayForPixel(double px, double py) const {
  Rectangle3D screen = getScreen();

  double u = (px + 0.5) / (double)width;
  double v = 1.0 - ((py + 0.5) / (double)height);

  Point3D target = screen.pointAt(u, v);
  Vector3D direction = (target - position).normalize();
  return Ray(position, direction);
}