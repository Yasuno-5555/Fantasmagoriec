#pragma once
#include <string>
#include <functional>
#include <map>

#if defined(_WIN32)
#define FANTA_EXPORT __declspec(dllexport)
#else
#define FANTA_EXPORT __attribute__((visibility("default")))
#endif

namespace fanta {
namespace internal {

    class PluginLoader {
    public:
        static void* Load(const std::string& path);
        static void Unload(void* handle);
        static void* GetSymbol(void* handle, const std::string& name);
    };

    struct PluginInfo {
        std::string name;
        std::string version;
        void* handle = nullptr;
    };

    class PluginManager {
    public:
        static PluginManager& Get() {
            static PluginManager instance;
            return instance;
        }

        bool LoadPlugin(const std::string& path);
        void UnloadAll();

    private:
        std::map<std::string, PluginInfo> m_plugins;
    };

} // internal
} // fanta
