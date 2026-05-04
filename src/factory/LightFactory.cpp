#include "LightFactory.hpp"
#include "../Error.hpp"

std::unique_ptr<ILight> LightFactory::create(const std::string& pluginPath)
{
    auto loader = std::make_unique<DLLoader<ILight>>();

    std::string path = pluginPath;
    std::string func = "createLight";
    loader->loadlib(path, func);

    ILight* raw = loader->getInstance();
    if (!raw)
        throw Error("LightFactory: plugin returned null: " + pluginPath);

    _loaders.push_back(std::move(loader));

    // The DLLoader owns the raw pointer — use a no-op deleter here.
    return std::unique_ptr<ILight>(raw, [](ILight*){});
}

std::vector<std::unique_ptr<ILight>> LightFactory::createSceneLights(
    const std::string& ambientPath,
    const std::string& directionalPath)
{
    std::vector<std::unique_ptr<ILight>> lights;

    if (!ambientPath.empty())
        lights.push_back(create(ambientPath));

    if (!directionalPath.empty())
        lights.push_back(create(directionalPath));

    return lights;
}