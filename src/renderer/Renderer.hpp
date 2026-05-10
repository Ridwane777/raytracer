#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "../scene/Scene.hpp"
#include "../interfaces/HitInfo.hpp"
#include <vector>
#include <mutex>

class Renderer {
public:

    Renderer(const Scene& scene, int maxDepth = 0);
    std::vector<std::vector<Color>> render() const;

private:
    const Scene& _scene;
    int          _maxDepth;

    Color computeColor(const Ray& ray, int depth) const;
    Color computeDirectLighting(const IPrimitive& primitive,
                                const HitInfo& info) const;
    Color computeReflection(const HitInfo& info, int depth) const;
    Color computeRefraction(const IPrimitive& primitive, const HitInfo& info, int depth) const;
    void printProgress(int completedRows, int totalRows) const;

    void renderRows(int startY, int endY, 
                    std::vector<std::vector<Color>>& pixels,
                    std::mutex& progressMutex, 
                    int& completedRows) const;
};

#endif /* !RENDERER_HPP_ */
