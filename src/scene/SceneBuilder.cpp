#include "SceneBuilder.hpp"
#include "../math/Point3D.hpp"
#include "../math/Vector3D.hpp"

SceneBuilder::SceneBuilder(const std::string& pluginDir)
    : _pluginDir(pluginDir)
{
}

Scene SceneBuilder::build(const SceneConfig& cfg)
{
    Scene scene(buildCamera(cfg.camera));

    scene.ambientIntensity = cfg.lights.ambient;
    scene.diffuseIntensity = cfg.lights.diffuse;

    buildPrimitives(scene, cfg.primitives);
    buildLights(scene, cfg.lights);

    return scene;
}

Camera SceneBuilder::buildCamera(const CameraConfig& cfg)
{
    Point3D  pos(cfg.position.x, cfg.position.y, cfg.position.z);
    Vector3D rot(cfg.rotation.x, cfg.rotation.y, cfg.rotation.z);

    return Camera(pos, rot,
                  cfg.resolution.width,
                  cfg.resolution.height,
                  cfg.fieldOfView);
}

void SceneBuilder::buildPrimitives(Scene& scene, const PrimitivesConfig& cfg)
{
    for (const auto& s : cfg.spheres)   addSphere  (scene, s);
    for (const auto& p : cfg.planes)    addPlane   (scene, p);
    for (const auto& c : cfg.cylinders) addCylinder(scene, c);
    for (const auto& c : cfg.cones)     addCone    (scene, c);
}

void SceneBuilder::addSphere(Scene& scene, const SphereConfig& cfg)
{
    std::map<std::string, double> params = {
        {"x",       cfg.x},
        {"y",       cfg.y},
        {"z",       cfg.z},
        {"r",       cfg.r},
        {"color_r", static_cast<double>(cfg.color.r) / 255.0},
        {"color_g", static_cast<double>(cfg.color.g) / 255.0},
        {"color_b", static_cast<double>(cfg.color.b) / 255.0}
    };
    scene.addPrimitive(_primitiveFactory.create(pluginPath("sphere"), params));
}

void SceneBuilder::addPlane(Scene& scene, const PlaneConfig& cfg)
{
    double axisVal = 1.0;
    if (cfg.axis == "X" || cfg.axis == "x") axisVal = 0.0;
    else if (cfg.axis == "Z" || cfg.axis == "z") axisVal = 2.0;

    std::map<std::string, double> params = {
        {"axis",     axisVal},
        {"position", cfg.position},
        {"color_r",  static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",  static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",  static_cast<double>(cfg.color.b) / 255.0}
    };
    scene.addPrimitive(_primitiveFactory.create(pluginPath("plane"), params));
}

void SceneBuilder::addCylinder(Scene& scene, const CylinderConfig& cfg)
{
    std::map<std::string, double> params = {
        {"x",       cfg.x},
        {"y",       cfg.y},
        {"z",       cfg.z},
        {"r",       cfg.r},
        {"color_r", static_cast<double>(cfg.color.r) / 255.0},
        {"color_g", static_cast<double>(cfg.color.g) / 255.0},
        {"color_b", static_cast<double>(cfg.color.b) / 255.0}
    };
    scene.addPrimitive(_primitiveFactory.create(pluginPath("cylinder"), params));
}

void SceneBuilder::addCone(Scene& scene, const ConeConfig& cfg)
{
    std::map<std::string, double> params = {
        {"x",       cfg.x},
        {"y",       cfg.y},
        {"z",       cfg.z},
        {"r",       cfg.r},
        {"color_r", static_cast<double>(cfg.color.r) / 255.0},
        {"color_g", static_cast<double>(cfg.color.g) / 255.0},
        {"color_b", static_cast<double>(cfg.color.b) / 255.0}
    };
    scene.addPrimitive(_primitiveFactory.create(pluginPath("cone"), params));
}

void SceneBuilder::buildLights(Scene& scene, const LightsConfig& cfg)
{
    // Lumière ambiante
    try {
        std::map<std::string, double> params = {{"intensity", cfg.ambient}};
        scene.addLight(_lightFactory.create(pluginPath("ambient"), params));
    } catch (...) {}

    // Lumières ponctuelles (point)
    for (const auto& pt : cfg.point) {
        std::map<std::string, double> params = {
            {"x",         pt.x},
            {"y",         pt.y},
            {"z",         pt.z},
            {"intensity", cfg.diffuse}
        };
        try {
            scene.addLight(_lightFactory.create(pluginPath("point"), params));
        } catch (const std::exception& e) { (void)e; }
    }

    // Lumières directionnelles
    for (const auto& dir : cfg.directional) {
        std::map<std::string, double> params = {
            {"dir_x",     dir.x},
            {"dir_y",     dir.y},
            {"dir_z",     dir.z},
            {"intensity", cfg.diffuse}
        };
        try {
            scene.addLight(_lightFactory.create(pluginPath("directional"), params));
        } catch (const std::exception& e) { (void)e; }
    }
}

std::string SceneBuilder::pluginPath(const std::string& name) const
{
    return _pluginDir + "/raytracer_" + name + ".so";
}
