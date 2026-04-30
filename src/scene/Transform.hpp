#ifndef TRANSFORM_HPP_
#define TRANSFORM_HPP_
#include "../interfaces/IPrimitive.hpp"
#include "../math/Matrix4x4.hpp"

class Transform : public IPrimitive {
private:
    IPrimitive* _wrapped;
    Matrix4x4   _worldToLocal; // inverse de la matrice de transformation
    Matrix4x4   _localToWorld;

public:
    Transform(IPrimitive* primitive, const Matrix4x4& transform);
    Transform() = default;
    ~Transform() = default;
    std::optional<double> hits(const Ray& ray) const override;
    Vector3D getNormal(const Point3D& point) const override;

    Color getColor(const Point3D& point) const override;
};

#endif /* !TRANSFORM_HPP_ */
