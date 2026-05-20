#ifndef SCENEBUILDER_HPP_
#define SCENEBUILDER_HPP_

#include "Scene.hpp"
#include "../parser/SceneConfig.hpp"
#include "../factory/PrimitiveFactory.hpp"
#include "../factory/LightFactory.hpp"
#include "../math/Matrix4x4.hpp"

#include <string>

class SceneBuilder {
public:
    explicit SceneBuilder(const std::string& pluginDir = ".");
    ~SceneBuilder() = default;

    Scene build(const SceneConfig& cfg);

private:
    std::string         _pluginDir;
    PrimitiveFactory    _primitiveFactory;
    LightFactory        _lightFactory;

    Camera buildCamera(const CameraConfig& cfg);

    void buildPrimitives(Scene& scene, const PrimitivesConfig& cfg);
    void addSphere  (Scene& scene, const SphereConfig&   cfg);
    void addPlane   (Scene& scene, const PlaneConfig&    cfg);
    void addCylinder(Scene& scene, const CylinderConfig& cfg);
    void addCone    (Scene& scene, const ConeConfig&     cfg);
    void addTriangle(Scene& scene, const TriangleConfig& cfg);

    void buildLights(Scene& scene, const LightsConfig& cfg);

    Matrix4x4 buildTransformMatrix(const TransformData& t, double x, double y, double z) const;

    std::string pluginPath(const std::string& name) const;
};

#endif 