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

// Stub GL functions when not linking against real OpenGL
// In real usage, these come from GLAD/GLEW
inline void glEnable(unsigned int) {}
inline void glDisable(unsigned int) {}
inline void glScissor(int, int, int, int) {}
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
    if (cmds.empty()) return;
    
    // Simplest immediate mode renderer for verification
    // In production: Batching is mandatory
    
    Rect current_scissor = {-1, -1, -1, -1};
    glEnable(GL_SCISSOR_TEST);
    
    // We assume backend setup (viewport) is correct
    // Note: OpenGL Scissor is lower-left origin, but our UI is top-left
    // Need to flip Y: sc_y = height - (y + h)
    
    for (const auto& cmd : cmds) {
        // Update Scissor if changed
        // Check if clip changed
        if (cmd.cx != current_scissor.x || cmd.cy != current_scissor.y || 
            cmd.cw != current_scissor.w || cmd.ch != current_scissor.h) {
            
            int screen_h = height_; // Backend window height
            int sc_x = (int)cmd.cx;
            int sc_y = screen_h - (int)(cmd.cy + cmd.ch);
            int sc_w = (int)cmd.cw;
            int sc_h = (int)cmd.ch;
            
            if (sc_w < 0) sc_w = 0;
            if (sc_h < 0) sc_h = 0;
            
            glScissor(sc_x, sc_y, sc_w, sc_h);
            
            current_scissor = Rect{cmd.cx, cmd.cy, cmd.cw, cmd.ch};
        }
        
        // Draw Command...
        // For verification, we just print or do nothing if no real GL linked
        // In real backend:
        // if (cmd.type == DrawType::Rect) draw_rect(...)
    }
    
    glDisable(GL_SCISSOR_TEST);
    
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
