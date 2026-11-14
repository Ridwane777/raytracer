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

struct TransformData
{
    Vec3 scale = {1.0, 1.0, 1.0};
    double sh_xy = 0.0, sh_xz = 0.0;
    double sh_yx = 0.0, sh_yz = 0.0;
    double sh_zx = 0.0, sh_zy = 0.0;
    Vec3 rotation = {0.0, 0.0, 0.0};
    double transparency = 0.0;
    double refractiveIndex = 1.0;
    double reflectivity = 0.0;
    double shininess = 0.0;
    Colors specular = {255, 255, 255};
};

struct SphereConfig : public TransformData
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    Colors color;
};

struct PlaneConfig : public TransformData
{
    std::string axis = "Y";
    double position = 0;
    Colors color;
};

struct CylinderConfig : public TransformData
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    double height = 1.0;   
    Colors color;
};

struct ConeConfig : public TransformData
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    double height = 1.0;   
    double angle  = 30.0;  
    Colors color;
};

struct TriangleConfig : public TransformData
{
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Colors color;
};

struct TorusConfig : public TransformData
{
    double x = 0, y = 0, z = 0;
    double r1 = 1.0;
    double r2 = 0.2;
    Colors color;
};

struct ObjConfig : public TransformData
{
    std::string filepath;
    double x = 0, y = 0, z = 0;
    Colors color;
};

struct PrimitivesConfig
{
    std::vector<SphereConfig> spheres;
    std::vector<PlaneConfig> planes;
    std::vector<CylinderConfig> cylinders;
    std::vector<ConeConfig> cones;
    std::vector<TriangleConfig> triangles;
    std::vector<TorusConfig> toruses;
    std::vector<ObjConfig> objs;
};

struct PointLight
{
    double x = 0, y = 0, z = 0;
    Colors color;
};

struct DirectionalLight
{
    double x = 0, y = 0, z = 0;
    Colors color;
};

struct LightsConfig
{
    double ambient = 0.4;
    double diffuse = 0.6;
    std::vector<PointLight> point;
    std::vector<DirectionalLight> directional;
};

struct BackgroundConfig
{
    Colors top;
    Colors bottom;
};

struct SceneConfig
{
    CameraConfig camera;
    BackgroundConfig background;
    PrimitivesConfig primitives;
    LightsConfig lights;
};

#endif 
