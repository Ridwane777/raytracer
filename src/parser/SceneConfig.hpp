#ifndef SCENECONFIG_HPP_
#define SCENECONFIG_HPP_

#include <string>
#include <vector>

struct Vec3
{
    double x = 0, y = 0, z = 0;
};

struct Colors
{
    int r = 0, g = 0, b = 0;
};

struct Resolution
{
    int width = 1920, height = 1080;
};

struct CameraConfig
{
    Resolution resolution;
    Vec3 position;
    Vec3 rotation;
    double fieldOfView = 72.0;
    int antialiasing = 1;
};

struct SphereConfig
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    Colors color;
};

struct PlaneConfig
{
    std::string axis = "Y";
    double position = 0;
    Colors color;
};

struct CylinderConfig
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    double height = 1.0;   // hauteur du cylindre
    Colors color;
};

struct ConeConfig
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    double height = 1.0;   // hauteur du cône
    double angle  = 30.0;  // demi-angle au sommet (en degrés)
    Colors color;
};

struct PrimitivesConfig
{
    std::vector<SphereConfig> spheres;
    std::vector<PlaneConfig> planes;
    std::vector<CylinderConfig> cylinders;
    std::vector<ConeConfig> cones;
};

struct PointLight
{
    double x = 0, y = 0, z = 0;
};

struct DirectionalLight
{
    double x = 0, y = 0, z = 0;
};

struct LightsConfig
{
    double ambient = 0.4;
    double diffuse = 0.6;
    std::vector<PointLight> point;
    std::vector<DirectionalLight> directional;
};

struct SceneConfig
{
    CameraConfig camera;
    PrimitivesConfig primitives;
    LightsConfig lights;
};

#endif /* !SCENECONFIG_HPP_ */
