// Fantasmagorie v2 - Script Engine
// Lightweight Lua integration for widget scripting
#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

namespace fanta {
namespace script {

// ============================================================================
// Script Value (Variant type for Lua interop)
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
    
private:
    Type type_;
    bool bool_val_ = false;
    double num_val_ = 0;
    std::string str_val_;
};

// ============================================================================
// Script Context (Lightweight Lua-like interpreter)
// ============================================================================

class Context {
public:
    using NativeFunc = std::function<Value(const std::vector<Value>&)>;
    
    Context() {
        // Register built-in functions
        register_function("print", [](const std::vector<Value>& args) -> Value {
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
    }
    
    void register_function(const std::string& name, NativeFunc fn) {
        functions_[name] = fn;
    }
    
    void set_global(const std::string& name, Value val) {
        globals_[name] = val;
    }
    
    Value get_global(const std::string& name) const {
        auto it = globals_.find(name);
        return (it != globals_.end()) ? it->second : Value();
    }
    
    // Simple expression evaluator
    Value eval(const std::string& code) {
        // Tokenize and parse (simplified)
        std::string trimmed = trim(code);
        
        // Function call: name(args)
        size_t paren = trimmed.find('(');
        if (paren != std::string::npos && trimmed.back() == ')') {
            std::string func_name = trim(trimmed.substr(0, paren));
            std::string args_str = trimmed.substr(paren + 1, trimmed.size() - paren - 2);
            
            auto it = functions_.find(func_name);
            if (it != functions_.end()) {
                std::vector<Value> args = parse_args(args_str);
                return it->second(args);
            }
        }
        
        // Variable reference
        auto it = globals_.find(trimmed);
        if (it != globals_.end()) {
            return it->second;
        }
        
        // String literal
        if (trimmed.size() >= 2 && 
            ((trimmed.front() == '\'' && trimmed.back() == '\'') ||
             (trimmed.front() == '"' && trimmed.back() == '"'))) {
            return Value(trimmed.substr(1, trimmed.size() - 2));
        }
        
        // Number literal
        try {
            return Value(std::stod(trimmed));
        } catch (...) {}
        
        return Value();
    }
    
    // Execute multi-line script
    void exec(const std::string& script) {
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
    
private:
    std::unordered_map<std::string, NativeFunc> functions_;
    std::unordered_map<std::string, Value> globals_;
    
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
// Global Script Context
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

} // namespace script
} // namespace fanta
