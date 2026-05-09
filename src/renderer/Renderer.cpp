#include "Renderer.hpp"

Renderer::Renderer(const Scene& scene, int maxDepth)
    : _scene(scene), _maxDepth(maxDepth)
{
}

#include <thread>
#include <mutex>
#include <iostream>

std::vector<std::vector<Color>> Renderer::render() const
{
    const int width  = _scene.camera.width;
    const int height = _scene.camera.height;

    // Buffer 2D : pixels[y][x]
    std::vector<std::vector<Color>> pixels(height, std::vector<Color>(width));

    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // Valeur par défaut de sécurité
    
    std::vector<std::thread> threads;
    std::mutex progressMutex;
    int completedRows = 0;

    // Fonction lambda exécutée par chaque thread
    auto renderChunk = [&](int startY, int endY) {
        for (int y = startY; y < endY; ++y) {
            for (int x = 0; x < width; ++x) {
                Ray ray = _scene.camera.getRayForPixel(x, y);
                pixels[y][x] = computeColor(ray, 0);
            }
            
            // Mise à jour de la barre de progression
            std::lock_guard<std::mutex> lock(progressMutex);
            completedRows++;
            printProgress(completedRows, height);
        }
    };

    // Découpage de l'image en blocs horizontaux
    int rowsPerThread = height / numThreads;
    for (unsigned int i = 0; i < numThreads; ++i) {
        int startY = i * rowsPerThread;
        int endY = (i == numThreads - 1) ? height : startY + rowsPerThread;
        threads.emplace_back(renderChunk, startY, endY);
    }

    // Attendre que tous les threads aient terminé
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cerr << "\n[Renderer] Render complete!" << std::endl;

    return pixels;
}

Color Renderer::computeColor(const Ray& ray, int depth) const
{
    // Arrêt de la récursion (reflets/réfraction désactivés à depth=0)
    if (depth > _maxDepth)
        return Color(0, 0, 0);

    // Trouver la primitive la plus proche
    const IPrimitive* closest = _scene.primitives.getClosest(ray);
    if (!closest)
        return Color(0, 0, 0); // pas de hit → fond noir

    auto t = closest->hits(ray);
    if (!t.has_value())
        return Color(0, 0, 0);

    // Point d'intersection dans le monde
    Point3D  hitPoint = ray.at(t.value());

    // Normale à la surface (toujours normalisée)
    Vector3D normal  = closest->getNormal(hitPoint).normalize();

    // Vecteur vers la caméra (direction inverse du rayon)
    Vector3D viewDir = (ray.direction * -1.0).normalize();

    HitInfo info{hitPoint, normal, viewDir};

    // Éclairage direct : somme des contributions de toutes les lumières
    Color lighting = computeDirectLighting(*closest, info);

    // Couleur de base de l'objet × éclairage (multiplication composante à composante)
    Color objColor = closest->getColor(hitPoint);

    return (objColor * lighting).clamp();
}

Color Renderer::computeDirectLighting(const IPrimitive& primitive,
                                      const HitInfo& info) const
{
    (void)primitive; // sera utile pour Phong (could)
    Color total(0, 0, 0);

    for (const auto& light : _scene.lights) {
        // _scene.primitives (PrimitiveList) est passée pour les shadow rays
        total = total + light->computeContribution(info, _scene.primitives);
    }
    return total;
}

Color Renderer::computeReflection(const HitInfo& info, int depth) const
{
    // R = D - 2*(D.N)*N
    // Color reflected = computeColor(Ray(info.point + info.normal * 1e-4, R), depth + 1);
    (void)info;
    (void)depth;
    return Color(0, 0, 0);
}


Color Renderer::computeRefraction(const HitInfo& info, int depth) const
{
    // Loi de Snell : r = n1/n2, sinT² = r²*(1 - cosI²)
    (void)info;
    (void)depth;
    return Color(0, 0, 0);
}

void Renderer::printProgress(int completedRows, int totalRows) const
{
    int progress = (completedRows * 100) / totalRows;
    std::cerr << "\r\033[K[Renderer] Rendering progress: " << progress << "% (" 
              << completedRows << "/" << totalRows << ")" << std::flush;
}
