#ifndef CAMERA_HPP_
#define CAMERA_HPP_
#include "../math/Ray.hpp"
#include "../math/Rectangle3D.hpp"
#include <cmath>

class Camera {
public:
  Point3D position;
  Vector3D rotation;
  int width;
  int height;
  double fov;
  int antialiasing;

  Camera();
  Camera(const Camera &other);
  Camera &operator=(const Camera &other);
  ~Camera() = default;

  Camera(const Point3D &position, const Vector3D &rotation, int width,
         int height, double fov, int antialiasing = 1);
  Rectangle3D getScreen() const;
  Ray getRayForPixel(double px, double py) const;
};

#endif
