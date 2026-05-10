#ifndef HITINFO_HPP_
#define HITINFO_HPP_
#include "../math/Point3D.hpp"

struct HitInfo {
    Point3D  point;
    Vector3D normal;
    Vector3D viewDir;
};

#endif 
