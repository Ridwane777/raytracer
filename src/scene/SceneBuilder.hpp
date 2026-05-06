#ifndef SCENEBUILDER_HPP_
#define SCENEBUILDER_HPP_

#include "Camera.hpp"
#include "PrimitiveList.hpp"
#include "../interfaces/ILight.hpp"
#include "../parser/SceneConfig.hpp"
#include "../factory/PrimitiveFactory.hpp"
#include "../factory/LightFactory.hpp"

#include <vector>
#include <memory>
#include <string>

struct BuiltScene {
    Camera                                  camera;
    PrimitiveList                           primitives;
    std::vector<std::shared_ptr<ILight>>    lights;

    double ambientIntensity = 0.4;
    double diffuseIntensity = 0.6;
};

class SceneBuilder {
public:
   explicit SceneBuilder(const std::string& pluginDir = ".");
    ~SceneBuilder() = default;

    BuiltScene build(const SceneConfig& cfg);

private:
    std::string         _pluginDir;
    PrimitiveFactory    _primitiveFactory;
    LightFactory        _lightFactory;

    Camera buildCamera(const CameraConfig& cfg);

    void buildPrimitives(PrimitiveList& list,
                         const PrimitivesConfig& cfg);

    void addSphere  (PrimitiveList& list, const SphereConfig&   cfg);
    void addPlane   (PrimitiveList& list, const PlaneConfig&    cfg);
    void addCylinder(PrimitiveList& list, const CylinderConfig& cfg);
    void addCone    (PrimitiveList& list, const ConeConfig&     cfg);

    void buildLights(std::vector<std::shared_ptr<ILight>>& lights,
                     const LightsConfig& cfg);

    std::string pluginPath(const std::string& name) const;
};

#endif /* !SCENEBUILDER_HPP_ */