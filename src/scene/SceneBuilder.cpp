#include "SceneBuilder.hpp"
#include "Transform.hpp"
#include "../math/Point3D.hpp"
#include "../math/Vector3D.hpp"
#include <iostream>
#include <memory>

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
    // 1. La sphère est créée à l'ORIGINE (0, 0, 0)
    std::map<std::string, double> params = {
        {"x",       0.0},
        {"y",       0.0},
        {"z",       0.0},
        {"r",       cfg.r},
        {"color_r", static_cast<double>(cfg.color.r) / 255.0},
        {"color_g", static_cast<double>(cfg.color.g) / 255.0},
        {"color_b", static_cast<double>(cfg.color.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("sphere"), params);
        // 2. Le Decorator Transform applique la matrice de translation
        auto translated = std::make_shared<Transform>(
            base, Matrix4x4::translation(Vector3D(cfg.x, cfg.y, cfg.z)));
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        std::cerr << "[Sphere] " << e.what() << "\n";
    }
}

void SceneBuilder::addPlane(Scene& scene, const PlaneConfig& cfg)
{
    double axisVal = 1.0;
    if (cfg.axis == "X" || cfg.axis == "x") axisVal = 0.0;
    else if (cfg.axis == "Z" || cfg.axis == "z") axisVal = 2.0;

    // 1. Le plan est créé à la position 0 de son axe
    std::map<std::string, double> params = {
        {"axis",     axisVal},
        {"position", 0.0},
        {"color_r",  static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",  static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",  static_cast<double>(cfg.color.b) / 255.0}
    };
    // 2. La translation le place à la bonne position le long de son axe
    double tx = (axisVal == 0.0) ? cfg.position : 0.0;
    double ty = (axisVal == 1.0) ? cfg.position : 0.0;
    double tz = (axisVal == 2.0) ? cfg.position : 0.0;
    try {
        auto base = _primitiveFactory.create(pluginPath("plane"), params);
        auto translated = std::make_shared<Transform>(
            base, Matrix4x4::translation(Vector3D(tx, ty, tz)));
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        std::cerr << "[Plane] " << e.what() << "\n";
    }
}

void SceneBuilder::addCylinder(Scene& scene, const CylinderConfig& cfg)
{
    // 1. Cylindre créé à l'ORIGINE
    std::map<std::string, double> params = {
        {"x",       0.0},
        {"y",       0.0},
        {"z",       0.0},
        {"r",       cfg.r},
        {"height",  cfg.height},
        {"color_r", static_cast<double>(cfg.color.r) / 255.0},
        {"color_g", static_cast<double>(cfg.color.g) / 255.0},
        {"color_b", static_cast<double>(cfg.color.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("cylinder"), params);
        // 2. Translation vers la position finale
        auto translated = std::make_shared<Transform>(
            base, Matrix4x4::translation(Vector3D(cfg.x, cfg.y, cfg.z)));
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        std::cerr << "[Cylinder] " << e.what() << "\n";
    }
}

void SceneBuilder::addCone(Scene& scene, const ConeConfig& cfg)
{
    // 1. Cône créé à l'ORIGINE
    std::map<std::string, double> params = {
        {"x",       0.0},
        {"y",       0.0},
        {"z",       0.0},
        {"r",       cfg.r},
        {"height",  cfg.height},
        {"angle",   cfg.angle},
        {"color_r", static_cast<double>(cfg.color.r) / 255.0},
        {"color_g", static_cast<double>(cfg.color.g) / 255.0},
        {"color_b", static_cast<double>(cfg.color.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("cone"), params);
        // 2. Translation vers la position finale
        auto translated = std::make_shared<Transform>(
            base, Matrix4x4::translation(Vector3D(cfg.x, cfg.y, cfg.z)));
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        std::cerr << "[Cone] " << e.what() << "\n";
    }
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
