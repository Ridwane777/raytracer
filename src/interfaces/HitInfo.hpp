#ifndef HITINFO_HPP_
#define HITINFO_HPP_
#include "../math/Point3D.hpp"

// Informations sur le point touché, passées à chaque lumière.
struct HitInfo {
    Point3D  point;
    Vector3D normal;
    Vector3D viewDir;
};

#endif /* !HITINFO_HPP_ */
