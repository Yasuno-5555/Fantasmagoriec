#include "script_engine.hpp"
#include <filesystem>
#include <fstream>

namespace fanta {
namespace scripting {

static std::unique_ptr<ScriptEngine> g_script_engine;
static HotReloader g_hot_reloader;

ScriptEngine& GetScriptEngine() {
    if (!g_script_engine) {
        g_script_engine = std::make_unique<LuaEngine>();
        g_script_engine->initialize();
    }
    return *g_script_engine;
}

HotReloader& GetHotReloader() {
    return g_hot_reloader;
}

void HotReloader::watch(const std::string& path) {
    watch_path = path;
    enabled = true;
    
    // Initial scan
    if (std::filesystem::exists(path)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                auto ftime = std::filesystem::last_write_time(entry);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                file_timestamps[entry.path().string()] = std::chrono::system_clock::to_time_t(sctp);
            }
        }
    }
}

bool HotReloader::check_for_changes() {
    if (!enabled || watch_path.empty()) return false;
    
    bool changed = false;
    
    if (std::filesystem::exists(watch_path)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(watch_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                auto ftime = std::filesystem::last_write_time(entry);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                auto new_time = std::chrono::system_clock::to_time_t(sctp);
                
                auto it = file_timestamps.find(entry.path().string());
                if (it == file_timestamps.end() || it->second != new_time) {
                    file_timestamps[entry.path().string()] = new_time;
                    changed = true;
                }
            }
        }
    }
    
    return changed;
}

void HotReloader::reload_changed_files(ScriptEngine& engine) {
    if (!enabled) return;
    
    for (const auto& [path, _] : file_timestamps) {
        engine.load_file(path);
    }
}

// Convenience API
void RegisterScriptFunction(const std::string& name, 
    std::function<ScriptValue(const std::vector<ScriptValue>&)> func) {
    GetScriptEngine().register_function({name, func});
}

bool RunScript(const std::string& code) {
    return GetScriptEngine().execute(code);
}

bool LoadScriptFile(const std::string& path) {
    return GetScriptEngine().load_file(path);
}

void EnableHotReload(const std::string& watch_path) {
    GetHotReloader().watch(watch_path);
}

} // namespace scripting
} // namespace fanta
