#ifndef PRIMITIVEFACTORY_HPP_
#define PRIMITIVEFACTORY_HPP_

#include <vector>
#include <memory>
#include <string>
#include <map>
#include "../interfaces/IPrimitive.hpp"
#include "../DLLoader.hpp"

class PrimitiveFactory {
public:
    PrimitiveFactory()  = default;
    ~PrimitiveFactory() = default;

    std::unique_ptr<IPrimitive> create(const std::string& pluginPath,
                                       const std::map<std::string, double>& params);

private:
    using Loader = DLLoader<IPrimitive, const std::map<std::string, double>&>;
    std::vector<std::unique_ptr<Loader>> _loaders;
};

#endif // PRIMITIVEFACTORY_HPP_
