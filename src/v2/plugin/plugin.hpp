// Fantasmagorie v2 - Plugin System
// Dynamic loading of widgets and themes from DLL/dylib/.so
#pragma once

#include "../core/types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define FANTA_PLUGIN_EXPORT extern "C" __declspec(dllexport)
using PluginHandle = HMODULE;
#else
#include <dlfcn.h>
#define FANTA_PLUGIN_EXPORT extern "C"
using PluginHandle = void*;
#endif

namespace fanta {
namespace plugin {

// ============================================================================
// Plugin Info
// ============================================================================

struct PluginInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
};

// ============================================================================
// Plugin Interface
// ============================================================================

// Plugins must implement these functions:
// FANTA_PLUGIN_EXPORT PluginInfo* fanta_plugin_info();
// FANTA_PLUGIN_EXPORT void fanta_plugin_register();
// FANTA_PLUGIN_EXPORT void fanta_plugin_unregister();

using PluginInfoFunc = PluginInfo* (*)();
using PluginRegisterFunc = void (*)();

// ============================================================================
// Widget Factory
// ============================================================================

using WidgetFactory = std::function<void(const char* id)>;

class WidgetRegistry {
public:
    static WidgetRegistry& instance() {
        static WidgetRegistry r;
        return r;
    }
    
    void register_widget(const std::string& name, WidgetFactory factory) {
        factories_[name] = factory;
    }
    
    bool create(const std::string& name, const char* id) {
        auto it = factories_.find(name);
        if (it != factories_.end()) {
            it->second(id);
            return true;
        }
        return false;
    }
    
    std::vector<std::string> list() const {
        std::vector<std::string> result;
        for (const auto& [name, _] : factories_) {
            result.push_back(name);
        }
        return result;
    }
    
private:
    std::unordered_map<std::string, WidgetFactory> factories_;
};

// ============================================================================
// Theme Factory
// ============================================================================

class ThemeRegistry {
public:
    static ThemeRegistry& instance() {
        static ThemeRegistry r;
        return r;
    }
    
    void register_theme(const std::string& name, std::function<void*()> factory) {
        factories_[name] = factory;
    }
    
    void* create(const std::string& name) {
        auto it = factories_.find(name);
        if (it != factories_.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    std::vector<std::string> list() const {
        std::vector<std::string> result;
        for (const auto& [name, _] : factories_) {
            result.push_back(name);
        }
        return result;
    }
    
private:
    std::unordered_map<std::string, std::function<void*()>> factories_;
};

// ============================================================================
// Plugin Loader
// ============================================================================

class PluginLoader {
public:
    static PluginLoader& instance() {
        static PluginLoader l;
        return l;
    }
    
    bool load(const std::string& path) {
        PluginHandle handle = nullptr;
        
#ifdef _WIN32
        handle = LoadLibraryA(path.c_str());
#else
        handle = dlopen(path.c_str(), RTLD_NOW);
#endif
        
        if (!handle) {
            return false;
        }
        
        // Get plugin info
        auto info_func = get_func<PluginInfoFunc>(handle, "fanta_plugin_info");
        auto register_func = get_func<PluginRegisterFunc>(handle, "fanta_plugin_register");
        
        if (register_func) {
            register_func();
        }
        
        LoadedPlugin plugin;
        plugin.handle = handle;
        plugin.path = path;
        if (info_func) {
            PluginInfo* info = info_func();
            if (info) plugin.info = *info;
        }
        
        plugins_.push_back(plugin);
        return true;
    }
    
    void unload(const std::string& path) {
        for (auto it = plugins_.begin(); it != plugins_.end(); ) {
            if (it->path == path) {
                auto unregister_func = get_func<PluginRegisterFunc>(
                    it->handle, "fanta_plugin_unregister");
                if (unregister_func) {
                    unregister_func();
                }
                
#ifdef _WIN32
                FreeLibrary(it->handle);
#else
                dlclose(it->handle);
#endif
                it = plugins_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void unload_all() {
        for (auto& plugin : plugins_) {
            auto unregister_func = get_func<PluginRegisterFunc>(
                plugin.handle, "fanta_plugin_unregister");
            if (unregister_func) {
                unregister_func();
            }
#ifdef _WIN32
            FreeLibrary(plugin.handle);
#else
            dlclose(plugin.handle);
#endif
        }
        plugins_.clear();
    }
    
    const std::vector<PluginInfo> loaded_plugins() const {
        std::vector<PluginInfo> result;
        for (const auto& p : plugins_) {
            result.push_back(p.info);
        }
        return result;
    }
    
private:
    struct LoadedPlugin {
        PluginHandle handle;
        std::string path;
        PluginInfo info;
    };
    
    std::vector<LoadedPlugin> plugins_;
    
    template<typename T>
    T get_func(PluginHandle handle, const char* name) {
#ifdef _WIN32
        return reinterpret_cast<T>(GetProcAddress(handle, name));
#else
        return reinterpret_cast<T>(dlsym(handle, name));
#endif
    }
};

// ============================================================================
// Convenience
// ============================================================================

inline bool load(const std::string& path) { return PluginLoader::instance().load(path); }
inline void unload(const std::string& path) { PluginLoader::instance().unload(path); }

template<typename T>
void register_widget(const std::string& name) {
    WidgetRegistry::instance().register_widget(name, [](const char* id) {
        // Widget creation logic
    });
}

template<typename T>
void register_theme(const std::string& name) {
    ThemeRegistry::instance().register_theme(name, []() -> void* {
        static T theme;
        return &theme;
    });
}

} // namespace plugin
} // namespace fanta
