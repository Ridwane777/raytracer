#pragma once

#include "SceneConfig.hpp"
#include <libconfig.h++>
#include <string>

class SceneParser {
public:
  SceneParser() = default;
  ~SceneParser() = default;

  SceneConfig parse(const std::string &filepath);

private:
  CameraConfig parseCamera(const libconfig::Setting &root);
  Resolution parseResolution(const libconfig::Setting &cam);
  Vec3 parseVec3(const libconfig::Setting &setting);
  void parseTransformData(const libconfig::Setting &s, TransformData &t);

  PrimitivesConfig parsePrimitives(const libconfig::Setting &root);
  SphereConfig parseSphere(const libconfig::Setting &s);
  PlaneConfig parsePlane(const libconfig::Setting &p);
  CylinderConfig parseCylinder(const libconfig::Setting &c);
  ConeConfig parseCone(const libconfig::Setting &c);
  TriangleConfig parseTriangle(const libconfig::Setting &t);
  TorusConfig parseTorus(const libconfig::Setting &t);
  Colors parseColor(const libconfig::Setting &setting);

  LightsConfig parseLights(const libconfig::Setting &root);
  PointLight parsePointLight(const libconfig::Setting &l);
  DirectionalLight parseDirectionalLight(const libconfig::Setting &l);
  double getDouble(const libconfig::Setting &s, const char *key,
                   double fallback = 0.0);
};
