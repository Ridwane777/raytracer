#include "SceneBuilder.hpp"
#include "Transform.hpp"
#include "../math/Point3D.hpp"
#include "../math/Vector3D.hpp"
#include <memory>
#include <fstream>
#include <sstream>
#include "../Error.hpp"

SceneBuilder::SceneBuilder(const std::string& pluginDir)
    : _pluginDir(pluginDir)
{
}

Scene SceneBuilder::build(const SceneConfig& cfg)
{
    Scene scene(buildCamera(cfg.camera));

    scene.ambientIntensity = cfg.lights.ambient;
    scene.diffuseIntensity = cfg.lights.diffuse;
    scene.backgroundTop = Color(cfg.background.top.r / 255.0,
        cfg.background.top.g / 255.0, cfg.background.top.b / 255.0);
    scene.backgroundBottom = Color(cfg.background.bottom.r / 255.0,
        cfg.background.bottom.g / 255.0, cfg.background.bottom.b / 255.0);

    buildPrimitives(scene, cfg.primitives);
    buildLights(scene, cfg.lights);

    return scene;
}

Camera SceneBuilder::buildCamera(const CameraConfig& cfg)
{
    Point3D  pos(cfg.position.x, cfg.position.y, cfg.position.z);
    Vector3D rot(cfg.rotation.x, cfg.rotation.y, cfg.rotation.z);

    return Camera(pos, rot,
                  cfg.resolution.width, cfg.resolution.height,
                  cfg.fieldOfView, cfg.antialiasing);
}

void SceneBuilder::buildPrimitives(Scene& scene, const PrimitivesConfig& cfg)
{
    for (const auto& s : cfg.spheres)   addSphere  (scene, s);
    for (const auto& p : cfg.planes)    addPlane   (scene, p);
    for (const auto& c : cfg.cylinders) addCylinder(scene, c);
    for (const auto& c : cfg.cones)     addCone    (scene, c);
    for (const auto& t : cfg.triangles) addTriangle(scene, t);
    for (const auto& t : cfg.toruses)   addTorus   (scene, t);
    for (const auto& o : cfg.objs)      addObj     (scene, o);
}

void SceneBuilder::addSphere(Scene& scene, const SphereConfig& cfg)
{
    std::map<std::string, double> params = {
        {"x",             0.0},
        {"y",             0.0},
        {"z",             0.0},
        {"r",             cfg.r},
        {"transparency",  cfg.transparency},
        {"refractiveIndex", cfg.refractiveIndex},
        {"reflectivity",  cfg.reflectivity},
        {"shininess",     cfg.shininess},
        {"color_r",       static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",       static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",       static_cast<double>(cfg.color.b) / 255.0},
        {"spec_r",        static_cast<double>(cfg.specular.r) / 255.0},
        {"spec_g",        static_cast<double>(cfg.specular.g) / 255.0},
        {"spec_b",        static_cast<double>(cfg.specular.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("sphere"), params);
        Matrix4x4 tr = buildTransformMatrix(cfg, cfg.x, cfg.y, cfg.z);
        auto translated = std::make_shared<Transform>(base, tr);
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        throw Error("[Sphere] " + std::string(e.what()));
    }
}

void SceneBuilder::addPlane(Scene& scene, const PlaneConfig& cfg)
{
    double axisVal = 1.0;
    if (cfg.axis == "X" || cfg.axis == "x") axisVal = 0.0;
    else if (cfg.axis == "Z" || cfg.axis == "z") axisVal = 2.0;

    std::map<std::string, double> params = {
        {"axis",          axisVal},
        {"position",      0.0},
        {"transparency",  cfg.transparency},
        {"refractiveIndex", cfg.refractiveIndex},
        {"reflectivity",  cfg.reflectivity},
        {"shininess",     cfg.shininess},
        {"color_r",       static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",       static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",       static_cast<double>(cfg.color.b) / 255.0},
        {"spec_r",        static_cast<double>(cfg.specular.r) / 255.0},
        {"spec_g",        static_cast<double>(cfg.specular.g) / 255.0},
        {"spec_b",        static_cast<double>(cfg.specular.b) / 255.0}
    };
    double tx = (axisVal == 0.0) ? cfg.position : 0.0;
    double ty = (axisVal == 1.0) ? cfg.position : 0.0;
    double tz = (axisVal == 2.0) ? cfg.position : 0.0;
    try {
        auto base = _primitiveFactory.create(pluginPath("plane"), params);
        Matrix4x4 tr = buildTransformMatrix(cfg, tx, ty, tz);
        auto translated = std::make_shared<Transform>(base, tr);
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        throw Error("[Plane] " + std::string(e.what()));
    }
}

void SceneBuilder::addCylinder(Scene& scene, const CylinderConfig& cfg)
{
    std::map<std::string, double> params = {
        {"x",             0.0},
        {"y",             0.0},
        {"z",             0.0},
        {"r",             cfg.r},
        {"height",        cfg.height},
        {"transparency",  cfg.transparency},
        {"refractiveIndex", cfg.refractiveIndex},
        {"reflectivity",  cfg.reflectivity},
        {"shininess",     cfg.shininess},
        {"color_r",       static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",       static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",       static_cast<double>(cfg.color.b) / 255.0},
        {"spec_r",        static_cast<double>(cfg.specular.r) / 255.0},
        {"spec_g",        static_cast<double>(cfg.specular.g) / 255.0},
        {"spec_b",        static_cast<double>(cfg.specular.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("cylinder"), params);
        Matrix4x4 tr = buildTransformMatrix(cfg, cfg.x, cfg.y, cfg.z);
        auto translated = std::make_shared<Transform>(base, tr);
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        throw Error("[Cylinder] " + std::string(e.what()));
    }
}

void SceneBuilder::addCone(Scene& scene, const ConeConfig& cfg)
{
    std::map<std::string, double> params = {
        {"x",             0.0},
        {"y",             0.0},
        {"z",             0.0},
        {"r",             cfg.r},
        {"height",        cfg.height},
        {"angle",         cfg.angle},
        {"transparency",  cfg.transparency},
        {"refractiveIndex", cfg.refractiveIndex},
        {"reflectivity",  cfg.reflectivity},
        {"shininess",     cfg.shininess},
        {"color_r",       static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",       static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",       static_cast<double>(cfg.color.b) / 255.0},
        {"spec_r",        static_cast<double>(cfg.specular.r) / 255.0},
        {"spec_g",        static_cast<double>(cfg.specular.g) / 255.0},
        {"spec_b",        static_cast<double>(cfg.specular.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("cone"), params);
        Matrix4x4 tr = buildTransformMatrix(cfg, cfg.x, cfg.y, cfg.z);
        auto translated = std::make_shared<Transform>(base, tr);
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        throw Error("[Cone] " + std::string(e.what()));
    }
}

void SceneBuilder::addTriangle(Scene& scene, const TriangleConfig& cfg)
{
    std::map<std::string, double> params = {
        {"ax",            cfg.a.x},
        {"ay",            cfg.a.y},
        {"az",            cfg.a.z},
        {"bx",            cfg.b.x},
        {"by",            cfg.b.y},
        {"bz",            cfg.b.z},
        {"cx",            cfg.c.x},
        {"cy",            cfg.c.y},
        {"cz",            cfg.c.z},
        {"transparency",  cfg.transparency},
        {"refractiveIndex", cfg.refractiveIndex},
        {"reflectivity",  cfg.reflectivity},
        {"shininess",     cfg.shininess},
        {"color_r",       static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",       static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",       static_cast<double>(cfg.color.b) / 255.0},
        {"spec_r",        static_cast<double>(cfg.specular.r) / 255.0},
        {"spec_g",        static_cast<double>(cfg.specular.g) / 255.0},
        {"spec_b",        static_cast<double>(cfg.specular.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("triangle"), params);
        Matrix4x4 tr = buildTransformMatrix(cfg, 0.0, 0.0, 0.0);
        auto translated = std::make_shared<Transform>(base, tr);
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        throw Error("[Triangle] " + std::string(e.what()));
    }
}

void SceneBuilder::addTorus(Scene& scene, const TorusConfig& cfg)
{
    std::map<std::string, double> params = {
        {"x",             0.0},
        {"y",             0.0},
        {"z",             0.0},
        {"r1",            cfg.r1},
        {"r2",            cfg.r2},
        {"transparency",  cfg.transparency},
        {"refractiveIndex", cfg.refractiveIndex},
        {"reflectivity",  cfg.reflectivity},
        {"shininess",     cfg.shininess},
        {"color_r",       static_cast<double>(cfg.color.r) / 255.0},
        {"color_g",       static_cast<double>(cfg.color.g) / 255.0},
        {"color_b",       static_cast<double>(cfg.color.b) / 255.0},
        {"spec_r",        static_cast<double>(cfg.specular.r) / 255.0},
        {"spec_g",        static_cast<double>(cfg.specular.g) / 255.0},
        {"spec_b",        static_cast<double>(cfg.specular.b) / 255.0}
    };
    try {
        auto base = _primitiveFactory.create(pluginPath("torus"), params);
        Matrix4x4 tr = buildTransformMatrix(cfg, cfg.x, cfg.y, cfg.z);
        auto translated = std::make_shared<Transform>(base, tr);
        scene.addPrimitive(translated);
    } catch (const std::exception& e) {
        throw Error("[Torus] " + std::string(e.what()));
    }
}

void SceneBuilder::addObj(Scene& scene, const ObjConfig& cfg)
{
    std::ifstream file(cfg.filepath);
    if (!file.is_open()) {
        throw Error("[Obj] Cannot open .obj file: " + cfg.filepath);
    }

    std::vector<Point3D> vertices;
    std::string line;

    Matrix4x4 tr = buildTransformMatrix(cfg, cfg.x, cfg.y, cfg.z);

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            double vx, vy, vz;
            if (ss >> vx >> vy >> vz) {
                vertices.push_back(Point3D(vx, vy, vz));
            }
        } else if (type == "f") {
            std::vector<int> faceIndices;
            std::string token;
            while (ss >> token) {
                std::stringstream tokenStream(token);
                std::string indexStr;
                std::getline(tokenStream, indexStr, '/');
                if (!indexStr.empty()) {
                    int idx = std::stoi(indexStr);
                    if (idx < 0) {
                        idx = static_cast<int>(vertices.size()) + idx + 1;
                    }
                    faceIndices.push_back(idx);
                }
            }

            if (faceIndices.size() >= 3) {
                for (size_t i = 1; i < faceIndices.size() - 1; ++i) {
                    size_t idx0 = faceIndices[0] - 1;
                    size_t idx1 = faceIndices[i] - 1;
                    size_t idx2 = faceIndices[i + 1] - 1;

                    if (idx0 < vertices.size() && idx1 < vertices.size() && idx2 < vertices.size()) {
                        Point3D a = tr.transformPoint(vertices[idx0]);
                        Point3D b = tr.transformPoint(vertices[idx1]);
                        Point3D c = tr.transformPoint(vertices[idx2]);

                        std::map<std::string, double> params = {
                            {"ax",            a.X},
                            {"ay",            a.Y},
                            {"az",            a.Z},
                            {"bx",            b.X},
                            {"by",            b.Y},
                            {"bz",            b.Z},
                            {"cx",            c.X},
                            {"cy",            c.Y},
                            {"cz",            c.Z},
                            {"transparency",  cfg.transparency},
                            {"refractiveIndex", cfg.refractiveIndex},
                            {"reflectivity",  cfg.reflectivity},
                            {"shininess",     cfg.shininess},
                            {"color_r",       static_cast<double>(cfg.color.r) / 255.0},
                            {"color_g",       static_cast<double>(cfg.color.g) / 255.0},
                            {"color_b",       static_cast<double>(cfg.color.b) / 255.0},
                            {"spec_r",        static_cast<double>(cfg.specular.r) / 255.0},
                            {"spec_g",        static_cast<double>(cfg.specular.g) / 255.0},
                            {"spec_b",        static_cast<double>(cfg.specular.b) / 255.0}
                        };
                        try {
                            auto triangle = _primitiveFactory.create(pluginPath("triangle"), params);
                            scene.addPrimitive(triangle);
                        } catch (const std::exception& e) {
                            throw Error("[Obj Triangle] " + std::string(e.what()));
                        }
                    }
                }
            }
        }
    }
}

void SceneBuilder::buildLights(Scene& scene, const LightsConfig& cfg)
{
    
    try {
        std::map<std::string, double> params = {{"intensity", cfg.ambient}};
        scene.addLight(_lightFactory.create(pluginPath("ambient"), params));
    } catch (const std::exception& e) {
        throw Error("[Ambient Light] " + std::string(e.what()));
    }

    for (const auto& pt : cfg.point) {
        std::map<std::string, double> params = {
            {"x",         pt.x},
            {"y",         pt.y},
            {"z",         pt.z},
            {"intensity", cfg.diffuse},
            {"color_r", static_cast<double>(pt.color.r) / 255.0},
            {"color_g", static_cast<double>(pt.color.g) / 255.0},
            {"color_b", static_cast<double>(pt.color.b) / 255.0}
        };
        try {
            scene.addLight(_lightFactory.create(pluginPath("point"), params));
        } catch (const std::exception& e) {
            throw Error("[Point Light] " + std::string(e.what()));
        }

    }

    for (const auto& dir : cfg.directional) {
        std::map<std::string, double> params = {
            {"dir_x",     dir.x},
            {"dir_y",     dir.y},
            {"dir_z",     dir.z},
            {"intensity", cfg.diffuse},
            {"color_r", static_cast<double>(dir.color.r) / 255.0},
            {"color_g", static_cast<double>(dir.color.g) / 255.0},
            {"color_b", static_cast<double>(dir.color.b) / 255.0}
        };
        try {
            scene.addLight(_lightFactory.create(pluginPath("directional"), params));
        } catch (const std::exception& e) {
            throw Error("[Directional Light] " + std::string(e.what()));
        }

    }
}

std::string SceneBuilder::pluginPath(const std::string& name) const
{
    std::string path = _pluginDir + "/raytracer_" + name + ".so";
    if (path[0] != '/' && path[0] != '.')
        path = "./" + path;
    return path;
}

Matrix4x4 SceneBuilder::buildTransformMatrix(const TransformData& t, double x, double y, double z) const
{
    return Matrix4x4::translation(Vector3D(x, y, z)) *
           Matrix4x4::rotationZ(t.rotation.z) *
           Matrix4x4::rotationY(t.rotation.y) *
           Matrix4x4::rotationX(t.rotation.x) *
           Matrix4x4::shear(t.sh_xy, t.sh_xz, t.sh_yx, t.sh_yz, t.sh_zx, t.sh_zy) *
           Matrix4x4::scale(t.scale.x, t.scale.y, t.scale.z);
}
