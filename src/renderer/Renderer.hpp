#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "../scene/Scene.hpp"
#include "../interfaces/HitInfo.hpp"
#include <vector>

class Renderer {
public:
    // maxDepth = 0 : rendu plat (no recursion)
    // maxDepth > 0 : active les reflets/réfractions (could)
    Renderer(const Scene& scene, int maxDepth = 0);

    // Lance le rendu complet.
    // Retourne un buffer 2D : pixels[y][x] → Color
    // Stateless : peut être parallélisé par bandes de pixels (multithreading).
    std::vector<std::vector<Color>> render() const;

private:
    const Scene& _scene;
    int          _maxDepth;

    // Calcule la couleur du rayon. Récursif pour reflets/réfraction.
    // depth > _maxDepth → retourne la couleur de fond (noir).
    Color computeColor(const Ray& ray, int depth) const;

    // Somme la contribution de toutes les lumières au point touché.
    Color computeDirectLighting(const IPrimitive& primitive,
                                const HitInfo& info) const;

    // Pour les reflets (could) — à implémenter quand maxDepth > 0
    Color computeReflection(const HitInfo& info, int depth) const;

    // Pour la réfraction (could) — à implémenter quand maxDepth > 0
    Color computeRefraction(const HitInfo& info, int depth) const;

    // Affiche la barre de progression dans la console
    void printProgress(int completedRows, int totalRows) const;
};

#endif /* !RENDERER_HPP_ */
