#include "Scene.hpp"

Scene::Scene(const Camera& cam) : camera(cam)
{
}

void Scene::addPrimitive(std::shared_ptr<IPrimitive> primitive)
{
    primitives.add(primitive.get());         // le raw ptr dans la liste
    _ownedPrimitives.push_back(std::move(primitive)); // ownership ici
}

void Scene::addLight(std::shared_ptr<ILight> light)
{
    lights.push_back(std::move(light));
}
