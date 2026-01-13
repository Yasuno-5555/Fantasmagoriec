// Fantasmagorie v2 - Safe Plugin System
// Dynamic loading with SEH protection and version checks
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
#include <signal.h>
#include <setjmp.h>
#define FANTA_PLUGIN_EXPORT extern "C"
using PluginHandle = void*;
#endif

namespace fanta {
namespace plugin {

// ============================================================================
// Plugin API Version (for compatibility checks)
// ============================================================================

constexpr int API_VERSION_MAJOR = 2;
constexpr int API_VERSION_MINOR = 0;

// ============================================================================
// Plugin Load Result
// ============================================================================

enum class LoadResult {
    Success,
    FileNotFound,
    InvalidPlugin,       // Missing required exports
    VersionMismatch,     // API version incompatible
    RegistrationFailed,  // register() threw or crashed
    AlreadyLoaded
};

inline const char* load_result_string(LoadResult r) {
    switch (r) {
        case LoadResult::Success: return "Success";
        case LoadResult::FileNotFound: return "File not found";
        case LoadResult::InvalidPlugin: return "Invalid plugin (missing exports)";
        case LoadResult::VersionMismatch: return "API version mismatch";
        case LoadResult::RegistrationFailed: return "Registration failed (crash or exception)";
        case LoadResult::AlreadyLoaded: return "Plugin already loaded";
    }
    return "Unknown error";
}

// ============================================================================
// Plugin Info (must be provided by plugin)
// ============================================================================

struct PluginInfo {
    int api_version_major = 0;
    int api_version_minor = 0;
    const char* name = "";
    const char* version = "";
    const char* author = "";
    const char* description = "";
};

// ============================================================================
// Plugin Interface
// ============================================================================

// Plugins MUST implement:
// FANTA_PLUGIN_EXPORT PluginInfo* fanta_plugin_info();
// FANTA_PLUGIN_EXPORT bool fanta_plugin_register();
// FANTA_PLUGIN_EXPORT void fanta_plugin_unregister();

using PluginInfoFunc = PluginInfo* (*)();
using PluginRegisterFunc = bool (*)();
using PluginUnregisterFunc = void (*)();

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
    
    void unregister_widget(const std::string& name) {
        factories_.erase(name);
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
    
    void unregister_theme(const std::string& name) {
        factories_.erase(name);
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
// Safe Plugin Loader with SEH Protection
// ============================================================================

class PluginLoader {
public:
    static PluginLoader& instance() {
        static PluginLoader l;
        return l;
    }
    
    struct LoadedPlugin {
        PluginHandle handle = nullptr;
        std::string path;
        PluginInfo info;
        bool valid = false;
    };
    
    LoadResult load(const std::string& path) {
        // Check if already loaded
        for (const auto& p : plugins_) {
            if (p.path == path) {
                return LoadResult::AlreadyLoaded;
            }
        }
        
        PluginHandle handle = nullptr;
        
#ifdef _WIN32
        handle = LoadLibraryA(path.c_str());
        if (!handle) {
            DWORD err = GetLastError();
            if (err == ERROR_MOD_NOT_FOUND || err == ERROR_FILE_NOT_FOUND) {
                return LoadResult::FileNotFound;
            }
            return LoadResult::InvalidPlugin;
        }
#else
        handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle) {
            return LoadResult::FileNotFound;
        }
#endif
        
        // Get required exports
        auto info_func = get_func<PluginInfoFunc>(handle, "fanta_plugin_info");
        auto register_func = get_func<PluginRegisterFunc>(handle, "fanta_plugin_register");
        
        if (!info_func || !register_func) {
            close_handle(handle);
            return LoadResult::InvalidPlugin;
        }
        
        // Get plugin info with crash protection
        PluginInfo* info = nullptr;
        if (!safe_call([&]{ info = info_func(); })) {
            close_handle(handle);
            return LoadResult::RegistrationFailed;
        }
        
        if (!info) {
            close_handle(handle);
            return LoadResult::InvalidPlugin;
        }
        
        // Version compatibility check
        if (info->api_version_major != API_VERSION_MAJOR) {
            close_handle(handle);
            return LoadResult::VersionMismatch;
        }
        
        // Call register with crash protection
        bool reg_result = false;
        if (!safe_call([&]{ reg_result = register_func(); })) {
            close_handle(handle);
            return LoadResult::RegistrationFailed;
        }
        
        if (!reg_result) {
            close_handle(handle);
            return LoadResult::RegistrationFailed;
        }
        
        // Success - store plugin
        LoadedPlugin plugin;
        plugin.handle = handle;
        plugin.path = path;
        plugin.info = *info;
        plugin.valid = true;
        
        plugins_.push_back(plugin);
        return LoadResult::Success;
    }
    
    void unload(const std::string& path) {
        for (auto it = plugins_.begin(); it != plugins_.end(); ) {
            if (it->path == path) {
                safe_unload(*it);
                it = plugins_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void unload_all() {
        for (auto& plugin : plugins_) {
            safe_unload(plugin);
        }
        plugins_.clear();
    }
    
    std::vector<LoadedPlugin> loaded_plugins() const {
        return plugins_;
    }
    
    std::string last_error() const { return last_error_; }
    
private:
    std::vector<LoadedPlugin> plugins_;
    std::string last_error_;
    
    void close_handle(PluginHandle handle) {
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    }
    
    void safe_unload(LoadedPlugin& plugin) {
        if (!plugin.valid) return;
        
        auto unregister_func = get_func<PluginUnregisterFunc>(
            plugin.handle, "fanta_plugin_unregister");
        
        if (unregister_func) {
            safe_call([&]{ unregister_func(); });
        }
        
        close_handle(plugin.handle);
        plugin.valid = false;
    }
    
    template<typename T>
    T get_func(PluginHandle handle, const char* name) {
#ifdef _WIN32
        return reinterpret_cast<T>(GetProcAddress(handle, name));
#else
        return reinterpret_cast<T>(dlsym(handle, name));
#endif
    }
    
    // Safe call with crash protection
    // Note: Full SEH requires MSVC. For GCC/MinGW, we use C++ exceptions.
    // Consider using SetUnhandledExceptionFilter for production on Windows.
    template<typename Func>
    bool safe_call(Func&& fn) {
        try {
            fn();
            return true;
        } catch (const std::exception& e) {
            last_error_ = std::string("Plugin exception: ") + e.what();
            return false;
        } catch (...) {
            last_error_ = "Plugin threw an unknown exception";
            return false;
        }
    }
};

// ============================================================================
// Convenience Functions
// ============================================================================

inline LoadResult load(const std::string& path) { 
    return PluginLoader::instance().load(path); 
}

inline void unload(const std::string& path) { 
    PluginLoader::instance().unload(path); 
}

inline std::string last_error() {
    return PluginLoader::instance().last_error();
}

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
