#ifndef DLLOADER_HPP_
#define DLLOADER_HPP_
#include <string>
#include <dlfcn.h>
#include "Error.hpp"

template<typename T, typename... Args>
class DLLoader {
public:
    using CreateFn  = T*(*)(Args...);
    using DestroyFn = void(*)(T*);

    DLLoader() : _handle(nullptr), _lib(nullptr), _destroyFn(nullptr) {}

    ~DLLoader() { clearlib(); }

    DLLoader(const DLLoader&)            = delete;
    DLLoader& operator=(const DLLoader&) = delete;

    void loadlib(const std::string& path, const std::string& createfunc,
                 const std::string& destroyfunc, Args... args)
    {
        clearlib();
        _handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!_handle)
            throw DLLoaderError(dlerror());

        _destroyFn = reinterpret_cast<DestroyFn>(loadSym(destroyfunc));

        CreateFn createFn = reinterpret_cast<CreateFn>(loadSym(createfunc));
        _lib = createFn(args...);

        if (!_lib)
            throw DLLoaderError("Erreur de création pour: " + path);
    }

    void clearlib()
    {
        if (_lib && _destroyFn) {
            _destroyFn(_lib);
        }
        _lib       = nullptr;
        _destroyFn = nullptr;

        if (_handle) {
            dlclose(_handle);
            _handle = nullptr;
        }
    }

    T* getInstance() { return _lib; }

private:
    void* _handle;
    T*    _lib;
    DestroyFn _destroyFn;

    void* loadSym(const std::string& sym)
    {
        dlerror();
        void* s = dlsym(_handle, sym.c_str());
        const char* err = dlerror();
        if (err)
            throw DLLoaderError(err);
        return s;
    }
};

#endif 