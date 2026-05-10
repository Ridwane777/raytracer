#ifndef LIGHTFACTORY_HPP_
#define LIGHTFACTORY_HPP_
#include <memory>
#include <string>
#include <map>
#include "../interfaces/ILight.hpp"
#include "../DLLoader.hpp"

class LightFactory {
public:
    LightFactory()  = default;
    ~LightFactory() = default;

    using LightPtr = std::shared_ptr<ILight>;

    LightPtr create(const std::string& pluginPath,
                    const std::map<std::string, double>& params);

private:
    using Loader = DLLoader<ILight, const std::map<std::string, double>&>;
};

#endif 