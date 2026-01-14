#define NOMINMAX
#include "render/shader.hpp"
#include <glad/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace fanta {

// ============================================================================
// ShaderProgram
// ============================================================================

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept 
    : program_id_(other.program_id_), uniform_cache_(std::move(other.uniform_cache_)) {
    other.program_id_ = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        if (program_id_) glDeleteProgram(program_id_);
        program_id_ = other.program_id_;
        uniform_cache_ = std::move(other.uniform_cache_);
        other.program_id_ = 0;
    }
    return *this;
}

ShaderProgram::~ShaderProgram() {
    if (program_id_) glDeleteProgram(program_id_);
}

ShaderError ShaderProgram::compile(const char* vert_src, const char* frag_src) {
    ShaderError result;
    
    auto compile_shader = [&](GLenum type, const char* src) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info[512];
            glGetShaderInfoLog(shader, 512, nullptr, info);
            result.has_error = true;
            result.message = std::string(type == GL_VERTEX_SHADER ? "[Vertex] " : "[Fragment] ") + info;
            return 0;
        }
        return shader;
    };

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vert_src);
    if (result.has_error) return result;

    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    if (result.has_error) { glDeleteShader(vs); return result; }

    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vs);
    glAttachShader(program_id_, fs);
    glLinkProgram(program_id_);

    GLint success;
    glGetProgramiv(program_id_, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(program_id_, 512, nullptr, info);
        result.has_error = true;
        result.message = std::string("[Link] ") + info;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return result;
}

void ShaderProgram::use() const {
    if (program_id_) glUseProgram(program_id_);
}

int ShaderProgram::get_uniform_location(const char* name) const {
    if (uniform_cache_.find(name) != uniform_cache_.end())
        return uniform_cache_[name];
        
    int loc = glGetUniformLocation(program_id_, name);
    uniform_cache_[name] = loc;
    return loc;
}

void ShaderProgram::set_float(const char* name, float v) {
    glUniform1f(get_uniform_location(name), v);
}

void ShaderProgram::set_vec2(const char* name, float x, float y) {
    glUniform2f(get_uniform_location(name), x, y);
}

void ShaderProgram::set_vec4(const char* name, float x, float y, float z, float w) {
    glUniform4f(get_uniform_location(name), x, y, z, w);
}

void ShaderProgram::set_int(const char* name, int v) {
    glUniform1i(get_uniform_location(name), v);
}

// ============================================================================
// ShaderManager
// ============================================================================

ShaderManager& ShaderManager::instance() {
    static ShaderManager inst;
    return inst;
}

ShaderError ShaderManager::load(const char* name, const char* vert_src, const char* frag_src) {
    ShaderProgram prog;
    ShaderError err = prog.compile(vert_src, frag_src);
    if (err.has_error) {
        last_errors_[name] = err;
        if (error_callback_) error_callback_(name, err);
        std::cerr << "Shader Error [" << name << "]: " << err.message << std::endl;
        return err;
    }
    
    programs_[name] = std::move(prog);
    last_errors_.erase(name); // Clear previous errors
    return {};
}

ShaderError ShaderManager::load_file(const char* name, const char* vert_path, const char* frag_path) {
    // Basic file reading
    auto read_file = [](const char* path) -> std::string {
        std::ifstream f(path);
        if (!f.is_open()) return "";
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    };

    std::string vs = read_file(vert_path);
    std::string fs = read_file(frag_path);

    if (vs.empty() || fs.empty()) {
        ShaderError err;
        err.has_error = true;
        err.message = "Failed to open shader file: check path";
        last_errors_[name] = err;
        return err;
    }

    file_paths_[name] = {vert_path, frag_path};
    return load(name, vs.c_str(), fs.c_str());
}

void ShaderManager::reload_all() {
    for (const auto& [name, paths] : file_paths_) {
        load_file(name.c_str(), paths.first.c_str(), paths.second.c_str());
    }
}

ShaderProgram* ShaderManager::get(const char* name) {
    auto it = programs_.find(name);
    if (it != programs_.end()) return &it->second;
    return nullptr;
}

} // namespace fanta

