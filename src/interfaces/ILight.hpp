#ifndef ILIGHT_HPP_
#define ILIGHT_HPP_
#include "HitInfo.hpp"
#include "IPrimitive.hpp"

class ILight {
public:
    virtual ~ILight() = default;

    // Calcule la contribution colorée de cette lumière au point touché.
    // Reçoit la scène entière pour pouvoir envoyer des rayons d'ombre (drop shadows).
    // Retourne Color(0,0,0) si le point est dans l'ombre.
    virtual Color computeContribution(const HitInfo& hit,
                                      const IPrimitive& scene) const = 0;
};

#endif /* !ILIGHT_HPP_ */
