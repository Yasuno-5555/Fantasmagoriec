// Fantasmagorie v3 - Shader Placeholder
#include "render/shader.hpp"
#include <cstdio>

namespace fanta {
    // Shader class impl
    Shader::Shader(const char* vs, const char* fs) {
        // Load and compile logic
    }
    Shader::~Shader() { }
    void Shader::bind() { }
    void Shader::set_mat4(const char* name, const float* mat) { }
    void Shader::set_vec4(const char* name, float x, float y, float z, float w) { }
    void Shader::set_int(const char* name, int v) { }
    void Shader::set_float(const char* name, float v) { }
}
