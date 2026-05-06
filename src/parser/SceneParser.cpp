
#include "SceneParser.hpp"
#include <iostream>
#include <stdexcept>

double SceneParser::getDouble(const libconfig::Setting &s, const char *key, double fallback)
{
    if (!s.exists(key))
        return fallback;

    const libconfig::Setting &v = s[key];

    if (v.getType() == libconfig::Setting::TypeFloat)
        return static_cast<double>(v);
    if (v.getType() == libconfig::Setting::TypeInt)
        return static_cast<int>(v);
    return fallback;
}

SceneConfig SceneParser::parse(const std::string &filepath)
{
    libconfig::Config cfg;

    try {
        cfg.readFile(filepath.c_str());
    } catch (const libconfig::FileIOException &e) {
        throw std::runtime_error("Cannot open scene file: " + filepath);
    } catch (const libconfig::ParseException &e) {
        throw std::runtime_error(
            std::string("Parse error in ") + e.getFile() +
            " at line " + std::to_string(e.getLine()) +
            ": " + e.getError()
        );
    }

    const libconfig::Setting &root = cfg.getRoot();

    SceneConfig scene;
    scene.camera     = parseCamera(root);
    scene.primitives = parsePrimitives(root);
    scene.lights     = parseLights(root);
    return scene;
}

CameraConfig SceneParser::parseCamera(const libconfig::Setting &root)
{
    CameraConfig cam;

    if (!root.exists("camera"))
        return cam;

    const libconfig::Setting &c = root["camera"];

    cam.resolution  = parseResolution(c);
    cam.fieldOfView = getDouble(c, "fieldOfView", 72.0);

    if (c.exists("position"))
        cam.position = parseVec3(c["position"]);
    if (c.exists("rotation"))
        cam.rotation = parseVec3(c["rotation"]);

    return cam;
}

Resolution SceneParser::parseResolution(const libconfig::Setting &cam)
{
    Resolution res;

    if (!cam.exists("resolution"))
        return res;

    const libconfig::Setting &r = cam["resolution"];
    if (r.exists("width"))  res.width  = static_cast<int>(r["width"]);
    if (r.exists("height")) res.height = static_cast<int>(r["height"]);
    return res;
}

Vec3 SceneParser::parseVec3(const libconfig::Setting &s)
{
    Vec3 v;
    v.x = getDouble(s, "x");
    v.y = getDouble(s, "y");
    v.z = getDouble(s, "z");
    return v;
}

Colors SceneParser::parseColor(const libconfig::Setting &s)
{
    Colors c;
    if (!s.exists("color")) return c;

    const libconfig::Setting &col = s["color"];
    if (col.exists("r")) c.r = static_cast<int>(col["r"]);
    if (col.exists("g")) c.g = static_cast<int>(col["g"]);
    if (col.exists("b")) c.b = static_cast<int>(col["b"]);
    return c;
}

PrimitivesConfig SceneParser::parsePrimitives(const libconfig::Setting &root)
{
    PrimitivesConfig prims;

    if (!root.exists("primitives"))
        return prims;

    const libconfig::Setting &p = root["primitives"];
    if (p.exists("spheres")) {
        const libconfig::Setting &spheres = p["spheres"];
        for (int i = 0; i < spheres.getLength(); ++i)
            prims.spheres.push_back(parseSphere(spheres[i]));
    }
    if (p.exists("planes")) {
        const libconfig::Setting &planes = p["planes"];
        for (int i = 0; i < planes.getLength(); ++i)
            prims.planes.push_back(parsePlane(planes[i]));
    }
    if (p.exists("cylinders")) {
        const libconfig::Setting &cylinders = p["cylinders"];
        for (int i = 0; i < cylinders.getLength(); ++i)
            prims.cylinders.push_back(parseCylinder(cylinders[i]));
    }
    if (p.exists("cones")) {
        const libconfig::Setting &cones = p["cones"];
        for (int i = 0; i < cones.getLength(); ++i)
            prims.cones.push_back(parseCone(cones[i]));
    }

    return prims;
}

SphereConfig SceneParser::parseSphere(const libconfig::Setting &s)
{
    SphereConfig sphere;
    sphere.x     = getDouble(s, "x");
    sphere.y     = getDouble(s, "y");
    sphere.z     = getDouble(s, "z");
    sphere.r     = getDouble(s, "r", 1.0);
    sphere.color = parseColor(s);
    return sphere;
}

PlaneConfig SceneParser::parsePlane(const libconfig::Setting &s)
{
    PlaneConfig plane;
    if (s.exists("axis"))
        plane.axis = s["axis"].c_str();
    plane.position = getDouble(s, "position");
    plane.color    = parseColor(s);
    return plane;
}

CylinderConfig SceneParser::parseCylinder(const libconfig::Setting &s)
{
    CylinderConfig cyl;
    cyl.x     = getDouble(s, "x");
    cyl.y     = getDouble(s, "y");
    cyl.z     = getDouble(s, "z");
    cyl.r     = getDouble(s, "r", 1.0);
    cyl.color = parseColor(s);
    return cyl;
}

ConeConfig SceneParser::parseCone(const libconfig::Setting &s)
{
    ConeConfig cone;
    cone.x     = getDouble(s, "x");
    cone.y     = getDouble(s, "y");
    cone.z     = getDouble(s, "z");
    cone.r     = getDouble(s, "r", 1.0);
    cone.color = parseColor(s);
    return cone;
}

LightsConfig SceneParser::parseLights(const libconfig::Setting &root)
{
    LightsConfig lights;

    if (!root.exists("lights"))
        return lights;

    const libconfig::Setting &l = root["lights"];

    lights.ambient = getDouble(l, "ambient", 0.4);
    lights.diffuse = getDouble(l, "diffuse", 0.6);

    if (l.exists("point")) {
        const libconfig::Setting &pts = l["point"];
        for (int i = 0; i < pts.getLength(); ++i)
            lights.point.push_back(parsePointLight(pts[i]));
    }

    if (l.exists("directional")) {
        const libconfig::Setting &dirs = l["directional"];
        for (int i = 0; i < dirs.getLength(); ++i)
            lights.directional.push_back(parseDirectionalLight(dirs[i]));
    }

    return lights;
}

PointLight SceneParser::parsePointLight(const libconfig::Setting &s)
{
    PointLight light;
    light.x = getDouble(s, "x");
    light.y = getDouble(s, "y");
    light.z = getDouble(s, "z");
    return light;
}

DirectionalLight SceneParser::parseDirectionalLight(const libconfig::Setting &s)
{
    DirectionalLight light;
    light.x = getDouble(s, "x");
    light.y = getDouble(s, "y");
    light.z = getDouble(s, "z");
    return light;
}
