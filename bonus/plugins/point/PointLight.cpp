#include "PointLight.hpp"
#include <algorithm>
#include <cmath>

PointLight::PointLight(const Point3D &position, double intensity,
                       const Color &color)
    : _position(position), _intensity(intensity), _color(color) {}

Color PointLight::computeContribution(const HitInfo &hit,
                                      const IPrimitive &scene) const {
  Vector3D toLight = (_position - hit.point).normalize();
  double distanceToLight = (_position - hit.point).length();

  Point3D shadowOrigin = hit.point + hit.normal * 1e-4;
  Ray shadowRay(shadowOrigin, toLight);

  auto hitDist = scene.hits(shadowRay);
  if (hitDist.has_value() && hitDist.value() < distanceToLight) {
    return Color(0.0, 0.0, 0.0);
  }

  double diff = std::max(0.0, hit.normal.dot(toLight));
  double spec = 0.0;
  if (hit.shininess > 0.0 && diff > 0.0) {
    Vector3D reflectDir =
        (hit.normal * (2.0 * hit.normal.dot(toLight)) - toLight).normalize();
    spec = std::pow(std::max(0.0, reflectDir.dot(hit.viewDir)), hit.shininess);
  }

  Color diffusePart = _color * (_intensity * diff);
  Color specularPart = hit.specularColor * (_intensity * spec);
  return diffusePart + specularPart;
}

extern "C" ILight *createLight(const std::map<std::string, double> &params) {
  double x = params.count("x") ? params.at("x") : 0.0;
  double y = params.count("y") ? params.at("y") : 0.0;
  double z = params.count("z") ? params.at("z") : 0.0;
  double intensity = params.count("intensity") ? params.at("intensity") : 0.6;
  double r = params.count("color_r") ? params.at("color_r") : 1.0;
  double g = params.count("color_g") ? params.at("color_g") : 1.0;
  double b = params.count("color_b") ? params.at("color_b") : 1.0;

  return new PointLight(Point3D(x, y, z), intensity, Color(r, g, b));
}

extern "C" void destroyLight(ILight *light) { delete light; }
