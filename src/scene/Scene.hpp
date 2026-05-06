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

    // Ajoute une primitive dans la scène.
    // Le shared_ptr transféré garde ownership → la .so reste chargée.
    void addPrimitive(std::shared_ptr<IPrimitive> primitive);

    // Ajoute une lumière dans la scène.
    void addLight(std::shared_ptr<ILight> light);

private:
    // Garde les shared_ptr vivants aussi longtemps que la scène existe.
    std::vector<std::shared_ptr<IPrimitive>> _ownedPrimitives;
};

#endif /* !SCENE_HPP_ */
