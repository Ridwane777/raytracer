#include "SceneBuilder.hpp"
#include "../math/Point3D.hpp"
#include "../math/Vector3D.hpp"

#include <stdexcept>


SceneBuilder::SceneBuilder(const std::string& pluginDir)
    : _pluginDir(pluginDir)
{
}

BuiltScene SceneBuilder::build(const SceneConfig& cfg)
{
    BuiltScene scene;

    scene.camera            = buildCamera(cfg.camera);
    scene.ambientIntensity  = cfg.lights.ambient;
    scene.diffuseIntensity  = cfg.lights.diffuse;

    buildPrimitives(scene, cfg.primitives);
    buildLights(scene.lights, cfg.lights);

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


void SceneBuilder::buildPrimitives(BuiltScene& scene,
                                   const PrimitivesConfig& cfg)
{
    for (const auto& s : cfg.spheres)
        addSphere(scene, s);

    for (const auto& p : cfg.planes)
        addPlane(scene, p);

    for (const auto& c : cfg.cylinders)
        addCylinder(scene, c);

    for (const auto& c : cfg.cones)
        addCone(scene, c);
}

void SceneBuilder::addSphere(BuiltScene& scene, const SphereConfig& cfg)
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

    auto primitive = _primitiveFactory.create(pluginPath("sphere"), params);
    scene.primitives.add(primitive.get());
    scene.ownedPrimitives.push_back(std::move(primitive));
}

void SceneBuilder::addPlane(BuiltScene& scene, const PlaneConfig& cfg)
{
    // L'axe est encodé en double : 0=X, 1=Y, 2=Z
    double axisVal = 1.0; // Y par défaut
    if (cfg.axis == "X" || cfg.axis == "x") axisVal = 0.0;
    else if (cfg.axis == "Z" || cfg.axis == "z") axisVal = 2.0;

    std::map<std::string, double> params = {
        {"axis",    axisVal},
        {"position", cfg.position},
        {"color_r", static_cast<double>(cfg.color.r) / 255.0},
        {"color_g", static_cast<double>(cfg.color.g) / 255.0},
        {"color_b", static_cast<double>(cfg.color.b) / 255.0}
    };

    auto primitive = _primitiveFactory.create(pluginPath("plane"), params);
    scene.primitives.add(primitive.get());
    scene.ownedPrimitives.push_back(std::move(primitive));
}

void SceneBuilder::addCylinder(BuiltScene& scene, const CylinderConfig& cfg)
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

    auto primitive = _primitiveFactory.create(pluginPath("cylinder"), params);
    scene.primitives.add(primitive.get());
    scene.ownedPrimitives.push_back(std::move(primitive));
}

void SceneBuilder::addCone(BuiltScene& scene, const ConeConfig& cfg)
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

    auto primitive = _primitiveFactory.create(pluginPath("cone"), params);
    scene.primitives.add(primitive.get());
    scene.ownedPrimitives.push_back(std::move(primitive));
}

void SceneBuilder::buildLights(std::vector<std::shared_ptr<ILight>>& lights,
                                const LightsConfig& cfg)
{
    {
        std::map<std::string, double> params = {
            {"intensity", cfg.ambient}
        };
        try {
            auto light = _lightFactory.create(pluginPath("ambient"), params);
            lights.push_back(light);
        } catch (...) {}
    }

    for (const auto& pt : cfg.point) {
        std::map<std::string, double> params = {
            {"x",         pt.x},
            {"y",         pt.y},
            {"z",         pt.z},
            {"intensity", cfg.diffuse}
        };
        try {
            auto light = _lightFactory.create(pluginPath("point"), params);
            lights.push_back(light);
        } catch (const std::exception& e) {
            (void)e;
        }
    }

   for (const auto& dir : cfg.directional) {
        std::map<std::string, double> params = {
            {"dir_x",     dir.x},
            {"dir_y",     dir.y},
            {"dir_z",     dir.z},
            {"intensity", cfg.diffuse}
        };
        try {
            auto light = _lightFactory.create(pluginPath("directional"), params);
            lights.push_back(light);
        } catch (const std::exception& e) {
            (void)e;
        }
    }
}

std::string SceneBuilder::pluginPath(const std::string& name) const
{
    return _pluginDir + "/raytracer_" + name + ".so";
}

