// Fantasmagorie v2 - OpenGL ES 3.0 Backend Implementation
#include "backend_gl.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Minimal OpenGL function loading (for standalone compilation)
// In real usage, use GLAD, GLEW, or gl3w
#include <cstdio>
#include <cstring>

// Placeholder - in real implementation, link against OpenGL
#ifndef GL_TRIANGLES
#define GL_TRIANGLES 0x0004
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_BYTE 0x1401
#define GL_RGBA 0x1908
#define GL_TEXTURE_2D 0x0DE1
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_SCISSOR_TEST 0x0C11
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#endif

namespace fanta {

// Shader sources (GLSL ES 3.0 compatible)
static const char* vs_source = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

out vec2 vUV;
out vec4 vColor;

uniform vec2 uResolution;

void main() {
    vec2 pos = aPos / uResolution * 2.0 - 1.0;
    pos.y = -pos.y;
    gl_Position = vec4(pos, 0.0, 1.0);
    vUV = aUV;
    vColor = aColor;
}
)";

static const char* fs_source = R"(
#version 330 core
in vec2 vUV;
in vec4 vColor;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uMode;  // 0=Rect, 1=Text, 2=Image, 3=Shadow

void main() {
    if (uMode < 0.5) {
        // Rect: solid color
        FragColor = vColor;
    } else if (uMode < 1.5) {
        // Text: sample alpha from texture
        float a = texture(uTexture, vUV).r;
        FragColor = vec4(vColor.rgb, vColor.a * a);
    } else if (uMode < 2.5) {
        // Image: sample RGBA
        FragColor = texture(uTexture, vUV) * vColor;
    } else {
        // Shadow: soft falloff
        FragColor = vColor;
    }
}
)";

bool OpenGLBackend::init() {
    if (initialized_) return true;
    
    // In real implementation: compile shaders, create VAO/VBO
    // For now, just mark as initialized
    compile_shaders();
    setup_buffers();
    
    initialized_ = true;
    return true;
}

void OpenGLBackend::shutdown() {
    // Cleanup OpenGL resources
    initialized_ = false;
}

void OpenGLBackend::begin_frame(int width, int height) {
    width_ = width;
    height_ = height;
}

void OpenGLBackend::end_frame() {
    // Swap buffers handled by platform layer
}

void OpenGLBackend::render(const DrawList& list, Font* font) {
    if (!initialized_) return;
    
    // Render each layer in order
    for (int layer = 0; layer < (int)RenderLayer::Count; ++layer) {
        render_layer(list.cmds[layer], font);
    }
}

void OpenGLBackend::render_layer(const std::vector<DrawCmd>& cmds, Font* font) {
    // TODO: Batch and render commands
    // This is a placeholder - real implementation would:
    // 1. Sort by texture
    // 2. Build vertex buffer
    // 3. Issue draw calls with appropriate uniforms
    (void)cmds;
    (void)font;
}

void* OpenGLBackend::create_texture(int w, int h, const void* pixels, int channels) {
    // TODO: glGenTextures, glTexImage2D
    (void)w; (void)h; (void)pixels; (void)channels;
    return nullptr;
}

void OpenGLBackend::update_texture(void* tex, int x, int y, int w, int h, const void* pixels) {
    // TODO: glTexSubImage2D
    (void)tex; (void)x; (void)y; (void)w; (void)h; (void)pixels;
}

void OpenGLBackend::destroy_texture(void* tex) {
    // TODO: glDeleteTextures
    (void)tex;
}

void OpenGLBackend::compile_shaders() {
    // TODO: Compile and link shaders
    // shader_program_ = ...
}

void OpenGLBackend::setup_buffers() {
    // TODO: Create VAO and VBO
    // vao_ = ...
    // vbo_ = ...
}

} // namespace fanta
