#include "Renderer.hpp"
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>
#include <random>

Renderer::Renderer(const Scene &scene, int maxDepth)
    : _scene(scene), _maxDepth(maxDepth) {}

std::vector<std::vector<Color>> Renderer::render() const {
  const int width = _scene.camera.width;
  const int height = _scene.camera.height;

  std::vector<std::vector<Color>> pixels(height, std::vector<Color>(width));

  unsigned int numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0)
    numThreads = 4;

  std::vector<std::thread> threads;
  std::mutex progressMutex;
  int completedRows = 0;

  int rowsPerThread = height / numThreads;
  for (unsigned int i = 0; i < numThreads; ++i) {
    int startY = i * rowsPerThread;
    int endY = (i == numThreads - 1) ? height : startY + rowsPerThread;
    
    threads.emplace_back([this, startY, endY, &pixels, &progressMutex, &completedRows]() {
        this->renderRows(startY, endY, pixels, progressMutex, completedRows);
    });
  }

  for (auto &t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }
  std::cerr << "\n[Renderer] Render complete!" << std::endl;
  return pixels;
}

Color Renderer::computeColor(const Ray &ray, int depth) const {
  
  if (depth > _maxDepth)
    return Color(0, 0, 0);

  const IPrimitive *closest = _scene.primitives.getClosest(ray);
  if (!closest)
    return Color(0, 0, 0);

  auto t = closest->hits(ray);
  if (!t.has_value())
    return Color(0, 0, 0);

  Point3D hitPoint = ray.at(t.value());
  Vector3D normal = closest->getNormal(hitPoint).normalize();
  Vector3D viewDir = (ray.direction * -1.0).normalize();
  HitInfo info{hitPoint, normal, viewDir};
  Color lighting = computeDirectLighting(*closest, info);
  Color objColor = closest->getColor(hitPoint);
  double opacity = closest->getOpacity();
  double reflectivity = closest->getReflectivity();

  Color baseColor(0, 0, 0);

  if (opacity >= 1.0 || depth == _maxDepth) {
    baseColor = (objColor * lighting);
  } else {
    Color directColor = (objColor * lighting) * opacity;
    Color refractedColor = computeRefraction(*closest, info, depth + 1) * (1.0 - opacity);
    baseColor = directColor + refractedColor;
  }

  if (reflectivity > 0.0 && depth < _maxDepth) {
    Color reflectedColor = computeReflection(info, depth + 1);
    return (baseColor * (1.0 - reflectivity) + reflectedColor * reflectivity).clamp();
  }

  return baseColor.clamp();
}

Color Renderer::computeDirectLighting(const IPrimitive &primitive,
                                      const HitInfo &info) const {
  (void)primitive;
  Color total(0, 0, 0);

  for (const auto &light : _scene.lights) {
    total = total + light->computeContribution(info, _scene.primitives);
  }
  return total;
}

Color Renderer::computeReflection(const HitInfo &info, int depth) const {
  Vector3D incident = (info.viewDir * -1.0).normalize();
  Vector3D reflectDir = incident - info.normal * 2.0 * incident.dot(info.normal);
  Ray reflectionRay(info.point + info.normal * 1e-4, reflectDir.normalize());
  return computeColor(reflectionRay, depth);
}

Color Renderer::computeRefraction(const IPrimitive &primitive, const HitInfo &info, int depth) const {
  double n1 = 1.0;
  double n2 = primitive.getRefractiveIndex();

  Vector3D incident = (info.viewDir * -1.0).normalize();
  Vector3D normal = info.normal;
  double cosI = normal.dot(incident);

  if (cosI < 0.0) {
    cosI = -cosI;
    normal = normal * -1.0;
    std::swap(n1, n2);
  }

  double ratio = n1 / n2;
  double k = 1.0 - ratio * ratio * (1.0 - cosI * cosI);
  if (k < 0.0)
    return computeReflection(info, depth);

  Vector3D refractedDir = incident * ratio + normal * (ratio * cosI - std::sqrt(k));
  Ray refractedRay(info.point + refractedDir * 1e-4, refractedDir.normalize());
  return computeColor(refractedRay, depth);
}

void Renderer::printProgress(int completedRows, int totalRows) const {
  int progress = (completedRows * 100) / totalRows;
  std::cerr << "\r\033[K[Renderer] Rendering progress: " << progress << "% ("
            << completedRows << "/" << totalRows << ")" << std::flush;
}

void Renderer::renderRows(int startY, int endY, 
                          std::vector<std::vector<Color>>& pixels,
                          std::mutex& progressMutex, 
                          int& completedRows) const {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis(0.0, 1.0);

  const int width = _scene.camera.width;
  const int height = _scene.camera.height;
  int samples = _scene.camera.antialiasing;
  if (samples < 1) samples = 1;

  for (int y = startY; y < endY; ++y) {
    for (int x = 0; x < width; ++x) {
      Color finalColor(0, 0, 0);

      for (int s = 0; s < samples; ++s) {
        double offsetX = (samples == 1) ? 0.0 : dis(gen);
        double offsetY = (samples == 1) ? 0.0 : dis(gen);
        Ray ray = _scene.camera.getRayForPixel(x + offsetX, y + offsetY);
        finalColor = finalColor + computeColor(ray, 0);
      }
      pixels[y][x] = finalColor * (1.0 / samples);
    }
    std::lock_guard<std::mutex> lock(progressMutex);
    completedRows++;
    printProgress(completedRows, height);
  }
}
