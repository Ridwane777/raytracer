#include <string>
#include <vector>

struct Vec3
{
    double x = 0, y = 0, z = 0;
};

struct Color
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
};

struct SphereConfig
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    Color color;
};

struct PlaneConfig
{
    std::string axis = "Y";
    double position = 0;
    Color color;
};

struct CylinderConfig
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    Color color;
};

struct ConeConfig
{
    double x = 0, y = 0, z = 0;
    double r = 1;
    Color color;
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
