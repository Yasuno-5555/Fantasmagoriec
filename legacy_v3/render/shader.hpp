// Fantasmagorie v3 - Shader Manager
// Hot-reload in development, embedded in release
// Includes error logging to prevent debug hell
#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <cstdint>

namespace fanta {

// ============================================================================
// Shader Compilation Result
// ============================================================================

struct ShaderError {
    bool has_error = false;
    std::string message;
    int line = -1;
    
    operator bool() const { return has_error; }
};

// ============================================================================
// Shader Program (OpenGL-agnostic interface)
// ============================================================================

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();
    
    // Non-copyable
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    
    // Movable
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;
    
    // Compilation
    ShaderError compile(const char* vert_src, const char* frag_src);
    
    // Usage
    void use() const;
    bool valid() const { return program_id_ != 0; }
    
    // Uniform setters (with caching)
    void set_float(const char* name, float v);
    void set_vec2(const char* name, float x, float y);
    void set_vec4(const char* name, float x, float y, float z, float w);
    void set_int(const char* name, int v);
    
    // Raw access (for platform layer)
    uint32_t id() const { return program_id_; }
    
private:
    uint32_t program_id_ = 0;
    mutable std::unordered_map<std::string, int> uniform_cache_;
    
    int get_uniform_location(const char* name) const;
};

// ============================================================================
// Shader Manager
// ============================================================================

class ShaderManager {
public:
    static ShaderManager& instance();
    
    // Load from strings (release mode)
    ShaderError load(const char* name, const char* vert_src, const char* frag_src);
    
    // Load from files (development mode with hot-reload)
    ShaderError load_file(const char* name, const char* vert_path, const char* frag_path);
    
    // Hot-reload all file-based shaders
    void reload_all();
    
    // Get compiled shader
    ShaderProgram* get(const char* name);
    
    // Error callback (for UI display or logging)
    using ErrorCallback = std::function<void(const char* shader_name, const ShaderError& error)>;
    void set_error_callback(ErrorCallback cb) { error_callback_ = cb; }
    
    // Development helpers
    bool has_errors() const { return !last_errors_.empty(); }
    const std::unordered_map<std::string, ShaderError>& errors() const { return last_errors_; }
    
private:
    ShaderManager() = default;
    
    std::unordered_map<std::string, ShaderProgram> programs_;
    std::unordered_map<std::string, std::pair<std::string, std::string>> file_paths_; // For reload
    std::unordered_map<std::string, ShaderError> last_errors_;
    ErrorCallback error_callback_;
};

// ============================================================================
// Embedded Shader Macros (for release builds)
// ============================================================================

// Usage: FANTA_EMBED_SHADER(sdf_rect, "...")
#define FANTA_EMBED_SHADER_VERT(name, src) \
    static const char* name##_vert = src;

#define FANTA_EMBED_SHADER_FRAG(name, src) \
    static const char* name##_frag = src;

} // namespace fanta
