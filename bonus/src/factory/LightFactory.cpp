#include "LightFactory.hpp"

LightFactory::LightPtr LightFactory::create(
    const std::string& pluginPath,
    const std::map<std::string, double>& params)
{
    auto loader = std::make_shared<Loader>();
    loader->loadlib(pluginPath, "createLight", "destroyLight", params);

    return std::shared_ptr<ILight>(loader->getInstance(),
        [loader](ILight*) mutable {
            loader.reset();
        });
}