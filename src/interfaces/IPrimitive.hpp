#ifndef IPRIMITIVE_HPP_
#define IPRIMITIVE_HPP_
#include <optional>
#include "../math/Ray.hpp"
#include "../math/Color.hpp"

class IPrimitive {
public:
    virtual ~IPrimitive() = default;

    // Retourne nullopt si pas d'intersection, sinon t > 0 (distance le long du rayon).
    virtual std::optional<double> hits(const Ray& ray) const = 0;

    // Normale à la surface au point donné (normalisée, vers l'extérieur).
    virtual Vector3D getNormal(const Point3D& point) const = 0;

    // Couleur flat de la primitive au point donné.
    // Le point est fourni pour les textures procédurales (could).
    virtual Color getColor(const Point3D& point) const = 0;
};

#endif /* !IPRIMITIVE_HPP_ */
