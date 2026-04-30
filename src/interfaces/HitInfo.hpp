#ifndef HITINFO_HPP_
#define HITINFO_HPP_
#include "../math/Point3D.hpp"

// Informations sur le point touché, passées à chaque lumière.
struct HitInfo {
    Point3D  point;     // point d'intersection dans la scène
    Vector3D normal;    // normale de la surface en ce point
    Vector3D viewDir;   // direction normalisée vers la caméra (pour Phong)
};

#endif /* !HITINFO_HPP_ */
