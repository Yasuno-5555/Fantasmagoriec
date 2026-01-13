// Fantasmagorie v2 - Sandboxed Script Engine
// Secure, lightweight Lua-like interpreter with execution limits
#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <memory>

namespace fanta {
namespace script {

// ============================================================================
// Script Errors
// ============================================================================

class ScriptError : public std::runtime_error {
public:
    explicit ScriptError(const std::string& msg) : std::runtime_error(msg) {}
};

class InstructionLimitExceeded : public ScriptError {
public:
    InstructionLimitExceeded() : ScriptError("SANDBOX: Instruction limit exceeded (possible infinite loop)") {}
};

class MemoryLimitExceeded : public ScriptError {
public:
    MemoryLimitExceeded() : ScriptError("SANDBOX: Memory limit exceeded") {}
};

class FunctionNotAllowed : public ScriptError {
public:
    explicit FunctionNotAllowed(const std::string& name) 
        : ScriptError("SANDBOX: Function '" + name + "' is not in the whitelist") {}
};

// ============================================================================
// Script Value (Variant type)
// ============================================================================

class Value {
public:
    enum class Type { Nil, Bool, Number, String, Function };
    
    Value() : type_(Type::Nil) {}
    Value(bool b) : type_(Type::Bool), bool_val_(b) {}
    Value(double n) : type_(Type::Number), num_val_(n) {}
    Value(int n) : type_(Type::Number), num_val_(n) {}
    Value(const char* s) : type_(Type::String), str_val_(s) {}
    Value(const std::string& s) : type_(Type::String), str_val_(s) {}
    
    Type type() const { return type_; }
    
    bool as_bool() const { return bool_val_; }
    double as_number() const { return num_val_; }
    const std::string& as_string() const { return str_val_; }
    
    bool is_nil() const { return type_ == Type::Nil; }
    bool is_truthy() const {
        if (type_ == Type::Nil) return false;
        if (type_ == Type::Bool) return bool_val_;
        return true;
    }
    
    size_t memory_size() const {
        return sizeof(*this) + str_val_.capacity();
    }
    
private:
    Type type_;
    bool bool_val_ = false;
    double num_val_ = 0;
    std::string str_val_;
};

// ============================================================================
// Sandbox Configuration
// ============================================================================

struct SandboxConfig {
    size_t max_instructions = 10000;      // Max operations per exec()
    size_t max_memory_bytes = 1024 * 64;  // 64KB max memory for variables
    size_t max_string_length = 4096;      // Max string literal length
    size_t max_call_depth = 32;           // Max recursion depth
    bool allow_print = true;              // Allow print() function
};

// ============================================================================
// Sandboxed Script Context
// ============================================================================

class Context {
public:
    using NativeFunc = std::function<Value(const std::vector<Value>&)>;
    
    SandboxConfig config;
    
    Context() {
        // Register safe built-in functions
        register_safe_function("print", [this](const std::vector<Value>& args) -> Value {
            if (!config.allow_print) return Value();
            for (const auto& arg : args) {
                if (arg.type() == Value::Type::String) {
                    printf("%s ", arg.as_string().c_str());
                } else if (arg.type() == Value::Type::Number) {
                    printf("%g ", arg.as_number());
                } else if (arg.type() == Value::Type::Bool) {
                    printf("%s ", arg.as_bool() ? "true" : "false");
                }
            }
            printf("\n");
            return Value();
        });
        
        register_safe_function("len", [](const std::vector<Value>& args) -> Value {
            if (!args.empty() && args[0].type() == Value::Type::String) {
                return Value((int)args[0].as_string().size());
            }
            return Value(0);
        });
        
        register_safe_function("tostring", [](const std::vector<Value>& args) -> Value {
            if (args.empty()) return Value("");
            const auto& v = args[0];
            if (v.type() == Value::Type::String) return v;
            if (v.type() == Value::Type::Number) return Value(std::to_string(v.as_number()));
            if (v.type() == Value::Type::Bool) return Value(v.as_bool() ? "true" : "false");
            return Value("nil");
        });
        
        register_safe_function("tonumber", [](const std::vector<Value>& args) -> Value {
            if (args.empty()) return Value();
            const auto& v = args[0];
            if (v.type() == Value::Type::Number) return v;
            if (v.type() == Value::Type::String) {
                try { return Value(std::stod(v.as_string())); } catch(...) {}
            }
            return Value();
        });
    }
    
    void register_safe_function(const std::string& name, NativeFunc fn) {
        functions_[name] = fn;
        whitelist_.insert(name);
    }
    
    void set_global(const std::string& name, Value val) {
        check_memory_limit(val.memory_size());
        globals_[name] = val;
        update_memory_usage();
    }
    
    Value get_global(const std::string& name) const {
        auto it = globals_.find(name);
        return (it != globals_.end()) ? it->second : Value();
    }
    
    // Safe expression evaluator with instruction counting
    Value eval(const std::string& code) {
        instruction_count_++;
        check_instruction_limit();
        
        std::string trimmed = trim(code);
        if (trimmed.empty()) return Value();
        
        // Function call: name(args)
        size_t paren = trimmed.find('(');
        if (paren != std::string::npos && trimmed.back() == ')') {
            std::string func_name = trim(trimmed.substr(0, paren));
            
            // Whitelist check
            if (whitelist_.find(func_name) == whitelist_.end()) {
                throw FunctionNotAllowed(func_name);
            }
            
            std::string args_str = trimmed.substr(paren + 1, trimmed.size() - paren - 2);
            
            auto it = functions_.find(func_name);
            if (it != functions_.end()) {
                call_depth_++;
                if (call_depth_ > config.max_call_depth) {
                    throw ScriptError("SANDBOX: Max call depth exceeded");
                }
                std::vector<Value> args = parse_args(args_str);
                Value result = it->second(args);
                call_depth_--;
                return result;
            }
        }
        
        // Assignment: name = value
        size_t eq = trimmed.find('=');
        if (eq != std::string::npos && eq > 0 && trimmed[eq-1] != '!' && trimmed[eq-1] != '<' && trimmed[eq-1] != '>') {
            if (eq + 1 < trimmed.size() && trimmed[eq+1] != '=') {
                std::string var_name = trim(trimmed.substr(0, eq));
                std::string value_str = trim(trimmed.substr(eq + 1));
                Value val = eval(value_str);
                set_global(var_name, val);
                return val;
            }
        }
        
        // Variable reference
        auto it = globals_.find(trimmed);
        if (it != globals_.end()) {
            return it->second;
        }
        
        // String literal (with length check)
        if (trimmed.size() >= 2 && 
            ((trimmed.front() == '\'' && trimmed.back() == '\'') ||
             (trimmed.front() == '"' && trimmed.back() == '"'))) {
            std::string str = trimmed.substr(1, trimmed.size() - 2);
            if (str.size() > config.max_string_length) {
                throw ScriptError("SANDBOX: String literal too long");
            }
            return Value(str);
        }
        
        // Number literal
        try {
            return Value(std::stod(trimmed));
        } catch (...) {}
        
        // Boolean literals
        if (trimmed == "true") return Value(true);
        if (trimmed == "false") return Value(false);
        if (trimmed == "nil") return Value();
        
        return Value();
    }
    
    // Execute multi-line script with full sandbox
    void exec(const std::string& script) {
        reset_execution_state();
        
        std::string line;
        for (char c : script) {
            if (c == '\n' || c == ';') {
                if (!line.empty()) {
                    eval(line);
                    line.clear();
                }
            } else {
                line += c;
            }
        }
        if (!line.empty()) {
            eval(line);
        }
    }
    
    // Safe execution wrapper - returns error message on failure
    std::string safe_exec(const std::string& script) {
        try {
            exec(script);
            return "";
        } catch (const ScriptError& e) {
            return e.what();
        } catch (const std::exception& e) {
            return std::string("Script error: ") + e.what();
        }
    }
    
    void reset() {
        globals_.clear();
        current_memory_ = 0;
        reset_execution_state();
    }
    
private:
    std::unordered_map<std::string, NativeFunc> functions_;
    std::unordered_set<std::string> whitelist_;
    std::unordered_map<std::string, Value> globals_;
    
    // Execution state
    size_t instruction_count_ = 0;
    size_t call_depth_ = 0;
    size_t current_memory_ = 0;
    
    void reset_execution_state() {
        instruction_count_ = 0;
        call_depth_ = 0;
    }
    
    void check_instruction_limit() {
        if (instruction_count_ > config.max_instructions) {
            throw InstructionLimitExceeded();
        }
    }
    
    void check_memory_limit(size_t additional) {
        if (current_memory_ + additional > config.max_memory_bytes) {
            throw MemoryLimitExceeded();
        }
    }
    
    void update_memory_usage() {
        current_memory_ = 0;
        for (const auto& [name, val] : globals_) {
            current_memory_ += name.size() + val.memory_size();
        }
    }
    
    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
    
    std::vector<Value> parse_args(const std::string& args_str) {
        std::vector<Value> args;
        std::string current;
        int depth = 0;
        bool in_string = false;
        char string_char = 0;
        
        for (char c : args_str) {
            instruction_count_++;
            check_instruction_limit();
            
            if (!in_string && (c == '\'' || c == '"')) {
                in_string = true;
                string_char = c;
                current += c;
            } else if (in_string && c == string_char) {
                in_string = false;
                current += c;
            } else if (!in_string && c == '(') {
                depth++;
                current += c;
            } else if (!in_string && c == ')') {
                depth--;
                current += c;
            } else if (!in_string && c == ',' && depth == 0) {
                if (!current.empty()) {
                    args.push_back(eval(trim(current)));
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            args.push_back(eval(trim(current)));
        }
        return args;
    }
};

// ============================================================================
// Global Script Context (Sandboxed)
// ============================================================================

inline Context& global_context() {
    static Context ctx;
    return ctx;
}

inline Value eval(const std::string& code) {
    return global_context().eval(code);
}

inline void exec(const std::string& script) {
    global_context().exec(script);
}

inline std::string safe_exec(const std::string& script) {
    return global_context().safe_exec(script);
}

} // namespace script
} // namespace fanta
