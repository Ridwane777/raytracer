#include "PrimitiveFactory.hpp"
#include "../Error.hpp"

PrimitiveFactory::PrimitivePtr PrimitiveFactory::create(
    const std::string& pluginPath,
    const std::map<std::string, double>& params)
{
    auto loader = std::make_shared<Loader>();
    loader->loadlib(pluginPath, "createPrimitive", "destroyPrimitive", params);

    IPrimitive* raw = loader->getInstance();
    return std::shared_ptr<IPrimitive>(raw, [loader](IPrimitive*) mutable {
        loader.reset(); 
    });
}