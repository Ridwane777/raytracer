#ifndef CAMERA_HPP_
#define CAMERA_HPP_
#include "../math/Ray.hpp"
#include "../math/Rectangle3D.hpp"
#include <cmath>

class Camera {
public:
    Point3D  position;
    Vector3D rotation; // angles en degrés (x, y, z)
    int      width;
    int      height;
    double   fov;      // en degrés

    Camera();
    Camera(const Camera& other);
    Camera& operator=(const Camera& other);
    ~Camera() = default;

    Camera(const Point3D& position, const Vector3D& rotation,
           int width, int height, double fov);

    // Construit le Rectangle3D du plan image.
    // Place le plan à distance 1 devant la caméra.
    // bottom_side et left_side sont calculés depuis le FOV et le ratio.
    Rectangle3D getScreen() const;

    // Génère le rayon pour le pixel (px, py).
    // u = px / (width-1), v = py / (height-1)
    // direction = screen.pointAt(u, v) - position
    Ray getRayForPixel(int px, int py) const;
};

#endif /* !CAMERA_HPP_ */
