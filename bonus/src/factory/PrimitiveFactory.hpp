#ifndef PRIMITIVEFACTORY_HPP_
#define PRIMITIVEFACTORY_HPP_

#include <memory>
#include <string>
#include <map>
#include "../interfaces/IPrimitive.hpp"
#include "../DLLoader.hpp"

class PrimitiveFactory {
public:
    PrimitiveFactory()  = default;
    ~PrimitiveFactory() = default;

    using PrimitivePtr = std::shared_ptr<IPrimitive>;

    PrimitivePtr create(const std::string& pluginPath,
                        const std::map<std::string, double>& params);

private:
    using Loader = DLLoader<IPrimitive, const std::map<std::string, double>&>;
};

#endif 
