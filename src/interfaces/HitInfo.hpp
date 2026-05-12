#ifndef HITINFO_HPP_
#define HITINFO_HPP_
#include "../math/Point3D.hpp"
#include "../math/Color.hpp"

struct HitInfo {
    Point3D  point;
    Vector3D normal;
    Vector3D viewDir;
    double   shininess;
    Color    specularColor;
};

#endif 
