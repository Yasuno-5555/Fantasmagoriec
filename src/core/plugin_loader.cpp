#include "plugin_loader.hpp"
#include <iostream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fanta {
namespace internal {

void* PluginLoader::Load(const std::string& path) {
#if defined(_WIN32)
    HMODULE h = LoadLibraryA(path.c_str());
    if (!h) {
        std::cerr << "[Plugin] Failed to load DLL: " << path << " Error: " << GetLastError() << std::endl;
        return nullptr;
    }
    return (void*)h;
#else
    void* h = dlopen(path.c_str(), RTLD_LAZY);
    if (!h) {
        std::cerr << "[Plugin] Failed to load SO: " << path << " Error: " << dlerror() << std::endl;
        return nullptr;
    }
    return h;
#endif
}

void PluginLoader::Unload(void* handle) {
    if (!handle) return;
#if defined(_WIN32)
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}

void* PluginLoader::GetSymbol(void* handle, const std::string& name) {
    if (!handle) return nullptr;
#if defined(_WIN32)
    return (void*)GetProcAddress((HMODULE)handle, name.c_str());
#else
    return dlsym(handle, name.c_str());
#endif
}

bool PluginManager::LoadPlugin(const std::string& path) {
    void* handle = PluginLoader::Load(path);
    if (!handle) return false;

    // Call standard entry point if exists
    typedef void (*PluginInitFn)();
    auto init_fn = (PluginInitFn)PluginLoader::GetSymbol(handle, "FantaPluginInit");
    if (init_fn) {
        init_fn();
        std::cout << "[Plugin] Loaded and initialized: " << path << std::endl;
    } else {
        std::cout << "[Plugin] Loaded: " << path << " (No FantaPluginInit found)" << std::endl;
    }

    PluginInfo info;
    info.name = path;
    info.handle = handle;
    m_plugins[path] = info;
    return true;
}

void PluginManager::UnloadAll() {
    for (auto& pair : m_plugins) {
        PluginLoader::Unload(pair.second.handle);
    }
    m_plugins.clear();
}

} // internal
} // fanta
