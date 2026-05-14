#ifndef IPRIMITIVE_HPP_
#define IPRIMITIVE_HPP_
#include "../math/Color.hpp"
#include "../math/Ray.hpp"
#include <optional>

class IPrimitive {
public:
  virtual ~IPrimitive() = default;

  virtual std::optional<double> hits(const Ray &ray) const = 0;
  virtual Vector3D getNormal(const Point3D &point) const = 0;
  virtual Color getColor(const Point3D &point) const = 0;

  virtual double getOpacity() const = 0;
  virtual double getRefractiveIndex() const = 0;
  virtual double getReflectivity() const = 0;
  virtual double getShininess() const = 0;
  virtual Color getSpecularColor() const = 0;
};

#endif
