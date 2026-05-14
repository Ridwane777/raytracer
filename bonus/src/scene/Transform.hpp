#ifndef TRANSFORM_HPP_
#define TRANSFORM_HPP_
#include "../interfaces/IPrimitive.hpp"
#include "../math/Matrix4x4.hpp"
#include <memory>

class Transform : public IPrimitive {
private:
  std::shared_ptr<IPrimitive> _wrapped;
  Matrix4x4 _worldToLocal;
  Matrix4x4 _localToWorld;

public:
  Transform(std::shared_ptr<IPrimitive> primitive, const Matrix4x4 &transform);
  Transform() = default;
  ~Transform() = default;
  std::optional<double> hits(const Ray &ray) const override;
  Vector3D getNormal(const Point3D &point) const override;

  Color getColor(const Point3D &point) const override;
  double getOpacity() const override;
  double getRefractiveIndex() const override;
  double getReflectivity() const override;
  double getShininess() const override;
  Color getSpecularColor() const override;
};

#endif
