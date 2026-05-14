#include "Scene.hpp"

Scene::Scene(const Camera& cam) : camera(cam)
{
}

void Scene::addPrimitive(std::shared_ptr<IPrimitive> primitive)
{
    primitives.add(primitive.get());         
    _ownedPrimitives.push_back(std::move(primitive)); 
}

void Scene::addLight(std::shared_ptr<ILight> light)
{
    lights.push_back(std::move(light));
}
