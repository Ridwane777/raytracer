#include "PrimitiveFactory.hpp"
#include "../Error.hpp"

std::unique_ptr<IPrimitive> PrimitiveFactory::create(
    const std::string& pluginPath,
    const std::map<std::string, double>& params)
{
    std::string path = pluginPath;
    std::string func = "createPrimitive";

    auto loader = std::make_unique<Loader>();
    loader->loadlib(path, func, params);

    IPrimitive* raw = loader->getInstance();
    if (!raw)
        throw Error("PrimitiveFactory: plugin returned null: " + pluginPath);

    _loaders.push_back(std::move(loader));

    return std::unique_ptr<IPrimitive>(raw, [](IPrimitive*){});
}
