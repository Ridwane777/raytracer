#ifndef DLLOADER_HPP_
#define DLLOADER_HPP_
#include <iostream>
#include <string>
#include <dlfcn.h>
#include "Error.hpp"

template<typename T>
class DLLoader {
    public:
        DLLoader() : handle(nullptr), lib(nullptr) {};

        DLLoader(std::string &path, std::string &func) : handle(nullptr), lib(nullptr) {
            this->loadlib(path, func);
        };

        ~DLLoader() { this->clearlib(); };

        void loadlib(std::string &path, std::string &func) {
            this->clearlib();
            handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
            if (!handle)
                throw DLLoaderError(dlerror());
            void *sym = dlsym(handle, func.c_str());
            if (!sym)
                throw DLLoaderError(dlerror());
            lib = reinterpret_cast <T *(*)()>(sym)();
        };

        void clearlib() {
            delete lib;
            lib = nullptr;
            if (handle)
                dlclose(handle);
            handle = nullptr;
        };

        T *getInstance() { return lib; };

    private:
        void *handle;
        T *lib;
};

#endif /* !DLLOADER_HPP_ */
