#pragma once

#include "../core/types_internal.hpp"
#include <string>
#include <functional>
#include <map>
#include <any>

namespace fanta {
namespace scripting {

// Script Value (polymorphic type for Lua/Python interop)
struct ScriptValue {
    enum class Type { Nil, Bool, Number, String, Function, Table, UserData };
    
    Type type = Type::Nil;
    std::any value;
    
    bool is_nil() const { return type == Type::Nil; }
    bool is_bool() const { return type == Type::Bool; }
    bool is_number() const { return type == Type::Number; }
    bool is_string() const { return type == Type::String; }
    
    bool as_bool() const { return std::any_cast<bool>(value); }
    double as_number() const { return std::any_cast<double>(value); }
    std::string as_string() const { return std::any_cast<std::string>(value); }
    
    static ScriptValue Nil() { return {Type::Nil, {}}; }
    static ScriptValue Bool(bool v) { return {Type::Bool, v}; }
    static ScriptValue Number(double v) { return {Type::Number, v}; }
    static ScriptValue String(const std::string& v) { return {Type::String, v}; }
};

// Script Binding (register C++ functions to script)
struct Binding {
    std::string name;
    std::function<ScriptValue(const std::vector<ScriptValue>&)> func;
};

// Scripting Engine Interface (abstract - implemented per language)
class ScriptEngine {
public:
    virtual ~ScriptEngine() = default;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    
    virtual bool execute(const std::string& code) = 0;
    virtual bool load_file(const std::string& path) = 0;
    
    virtual void register_function(const Binding& binding) = 0;
    virtual ScriptValue call(const std::string& func, const std::vector<ScriptValue>& args) = 0;
    
    virtual std::string get_error() const = 0;
};

// Lua Engine Stub (would need Lua library linked)
class LuaEngine : public ScriptEngine {
public:
    bool initialize() override { 
        // Would call luaL_newstate() etc
        return true; 
    }
    
    void shutdown() override {
        // Would call lua_close()
    }
    
    bool execute(const std::string& code) override {
        (void)code;
        return true;
    }
    
    bool load_file(const std::string& path) override {
        (void)path;
        return true;
    }
    
    void register_function(const Binding& binding) override {
        bindings[binding.name] = binding;
    }
    
    ScriptValue call(const std::string& func, const std::vector<ScriptValue>& args) override {
        auto it = bindings.find(func);
        if (it != bindings.end()) {
            return it->second.func(args);
        }
        return ScriptValue::Nil();
    }
    
    std::string get_error() const override { return last_error; }
    
private:
    std::map<std::string, Binding> bindings;
    std::string last_error;
    void* lua_state = nullptr;  // Would be lua_State*
};

// Hot Reload Support
struct HotReloader {
    std::string watch_path;
    std::map<std::string, std::time_t> file_timestamps;
    bool enabled = false;
    
    void watch(const std::string& path);
    bool check_for_changes();
    void reload_changed_files(ScriptEngine& engine);
};

// Global scripting accessors
ScriptEngine& GetScriptEngine();
HotReloader& GetHotReloader();

// Convenience API
void RegisterScriptFunction(const std::string& name, 
    std::function<ScriptValue(const std::vector<ScriptValue>&)> func);
bool RunScript(const std::string& code);
bool LoadScriptFile(const std::string& path);
void EnableHotReload(const std::string& watch_path);

} // namespace scripting
} // namespace fanta
