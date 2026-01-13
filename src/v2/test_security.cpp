// Test compilation of security-hardened modules
#include "script/engine.hpp"
#include "plugin/plugin.hpp"

int main() {
    // Test script sandbox
    fanta::script::Context ctx;
    ctx.config.max_instructions = 1000;
    ctx.config.max_memory_bytes = 1024;
    
    std::string error = ctx.safe_exec("print('Hello sandbox')");
    if (!error.empty()) {
        return 1;
    }
    
    // Test plugin loader result
    fanta::plugin::LoadResult result = fanta::plugin::load("nonexistent.dll");
    const char* msg = fanta::plugin::load_result_string(result);
    (void)msg;
    
    return 0;
}
