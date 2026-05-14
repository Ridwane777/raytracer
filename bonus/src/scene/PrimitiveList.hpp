#ifndef PRIMITIVELIST_HPP_
#define PRIMITIVELIST_HPP_

#include "../interfaces/IPrimitive.hpp"
#include <limits>
#include <optional>
#include <vector>

class PrimitiveList : public IPrimitive {
public:
  PrimitiveList() = default;
  ~PrimitiveList() override;

  void add(IPrimitive *primitive);

  std::optional<double> hits(const Ray &ray) const override;
  Vector3D getNormal(const Point3D &point) const override;
  Color getColor(const Point3D &point) const override;
  double getOpacity() const override;
  double getRefractiveIndex() const override;
  double getReflectivity() const override;
  double getShininess() const override;
  Color getSpecularColor() const override;

  const IPrimitive *getClosest(const Ray &ray) const;

private:
  std::vector<IPrimitive *> _children;
};

#endif
