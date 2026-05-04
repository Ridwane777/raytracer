#ifndef LIGHTFACTORY_HPP_
#define LIGHTFACTORY_HPP_

#include <vector>
#include <memory>
#include <string>
#include "../interfaces/ILight.hpp"
#include "../DLLoader.hpp"

class LightFactory {
public:
    LightFactory() = default;
    ~LightFactory() = default;

    std::unique_ptr<ILight> create(const std::string& pluginPath);
    std::vector<std::unique_ptr<ILight>> createSceneLights(const std::string& ambientPath, const std::string& directionalPath);

private:
    std::vector<std::unique_ptr<DLLoader<ILight>>> _loaders;
};

#endif /* !LIGHTFACTORY_HPP_ */