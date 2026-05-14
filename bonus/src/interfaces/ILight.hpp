#ifndef ILIGHT_HPP_
#define ILIGHT_HPP_
#include "HitInfo.hpp"
#include "IPrimitive.hpp"

class ILight {
public:
    virtual ~ILight() = default;

    virtual Color computeContribution(const HitInfo& hit,
                                      const IPrimitive& scene) const = 0;
};

#endif 
