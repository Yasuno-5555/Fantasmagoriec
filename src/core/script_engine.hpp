#pragma once
#include <string>

namespace fanta {
namespace internal {

    class ScriptEngine {
    public:
        virtual ~ScriptEngine() = default;
        virtual bool init() = 0;
        virtual void shutdown() = 0;
        virtual bool run_string(const std::string& code) = 0;
        virtual bool run_file(const std::string& path) = 0;
    };

    // A Null/No-op engine for when no scripting support is compiled in
    class NullScriptEngine : public ScriptEngine {
    public:
        bool init() override { return true; }
        void shutdown() override {}
        bool run_string(const std::string&) override { return false; }
        bool run_file(const std::string&) override { return false; }
    };

    // Global Registry for the current engine
    class ScriptManager {
    public:
        static ScriptManager& Get() {
            static ScriptManager instance;
            return instance;
        }

        void set_engine(ScriptEngine* engine) { m_engine = engine; }
        ScriptEngine* get_engine() { return m_engine; }

    private:
        ScriptEngine* m_engine = nullptr;
    };

} // internal
} // fanta
