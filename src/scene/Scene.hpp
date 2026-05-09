#ifndef SCENE_HPP_
#define SCENE_HPP_

#include "Camera.hpp"
#include "PrimitiveList.hpp"
#include "../interfaces/ILight.hpp"
#include <vector>
#include <memory>

class Scene {
public:
    Camera                                  camera;
    PrimitiveList                           primitives;     // Composite IPrimitive
    std::vector<std::shared_ptr<ILight>>    lights;

    double ambientIntensity = 0.4;
    double diffuseIntensity = 0.6;

    Scene() = default;
    explicit Scene(const Camera& cam);
    ~Scene() = default;
    void addPrimitive(std::shared_ptr<IPrimitive> primitive);
    void addLight(std::shared_ptr<ILight> light);

private:
    std::vector<std::shared_ptr<IPrimitive>> _ownedPrimitives;
};

#endif /* !SCENE_HPP_ */
