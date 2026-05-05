#ifndef DLLOADER_HPP_
#define DLLOADER_HPP_
#include <iostream>
#include <string>
#include <dlfcn.h>
#include "Error.hpp"

template<typename T, typename... Args>
class DLLoader {
public:
    DLLoader() : _handle(nullptr), _lib(nullptr) {}

    DLLoader(std::string& path, std::string& func, Args... args)
        : _handle(nullptr), _lib(nullptr)
    {
        loadlib(path, func, args...);
    }

    ~DLLoader() { clearlib(); }

    DLLoader(const DLLoader&)            = delete;
    DLLoader& operator=(const DLLoader&) = delete;

    void loadlib(std::string& path, std::string& func, Args... args)
    {
        clearlib();

        _handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!_handle)
            throw DLLoaderError(dlerror());

        void* sym = dlsym(_handle, func.c_str());
        const char* err = dlerror();
        if (!sym || err)
            throw DLLoaderError(err ? err : "dlsym: symbole introuvable");
        using CreateFn = T*(*)(Args...);
        _lib = reinterpret_cast<CreateFn>(sym)(args...);

        if (!_lib)
            throw DLLoaderError("createFn() a retourné nullptr pour: " + path);
    }

    void clearlib()
    {
        delete _lib;
        _lib = nullptr;
        if (_handle) {
            dlclose(_handle);
            _handle = nullptr;
        }
    }

    T* getInstance() { return _lib; }

private:
    void* _handle;
    T*    _lib;
};
#endif /* !DLLOADER_HPP_ */
