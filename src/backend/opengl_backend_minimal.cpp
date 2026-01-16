#define NOMINMAX
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "stb_image_write.h"

#include "core/contexts_internal.hpp"
#include "backend/drawlist.hpp"
#include "text/glyph_cache.hpp"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <imm.h>

// Global Old WndProc
// Must be global to avoid namespace issues with linkage if we were exporting, 
// but static is fine here.
static WNDPROC g_PrevWndProc = NULL;

// Forward decl
namespace fanta { namespace internal { struct EngineContext; EngineContext& GetEngineContext(); } }

static LRESULT CALLBACK FantaWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_IME_SETCONTEXT:
            lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
            break;
            
        case WM_IME_STARTCOMPOSITION:
            fanta::internal::GetEngineContext().input.ime_active = true;
            fanta::internal::GetEngineContext().input.ime_composition.clear();
            fanta::internal::GetEngineContext().input.ime_cursor = 0;
            return 0;
            
        case WM_IME_ENDCOMPOSITION:
            fanta::internal::GetEngineContext().input.ime_active = false;
            fanta::internal::GetEngineContext().input.ime_composition.clear();
            return 0;
            
        case WM_IME_COMPOSITION:
            fanta::internal::GetEngineContext().input.ime_active = true;
            HIMC himc = ImmGetContext(hwnd);
            if (himc) {
                if (lParam & GCS_COMPSTR) {
                    LONG len = ImmGetCompositionStringW(himc, GCS_COMPSTR, NULL, 0);
                    if (len > 0) {
                        std::vector<wchar_t> buf(len / sizeof(wchar_t));
                        ImmGetCompositionStringW(himc, GCS_COMPSTR, buf.data(), len);
                        int u8len = WideCharToMultiByte(CP_UTF8, 0, buf.data(), (int)buf.size(), NULL, 0, NULL, NULL);
                        if (u8len > 0) {
                            fanta::internal::GetEngineContext().input.ime_composition.resize(u8len);
                            WideCharToMultiByte(CP_UTF8, 0, buf.data(), (int)buf.size(), fanta::internal::GetEngineContext().input.ime_composition.data(), u8len, NULL, NULL);
                        }
                    } else {
                        fanta::internal::GetEngineContext().input.ime_composition.clear();
                    }
                    fanta::internal::GetEngineContext().input.ime_cursor = ImmGetCompositionStringW(himc, GCS_CURSORPOS, NULL, 0);
                }
                
                if (lParam & GCS_RESULTSTR) {
                    LONG len = ImmGetCompositionStringW(himc, GCS_RESULTSTR, NULL, 0);
                    if (len > 0) {
                        std::vector<wchar_t> buf(len / sizeof(wchar_t));
                        ImmGetCompositionStringW(himc, GCS_RESULTSTR, buf.data(), len);
                        std::string res;
                        int u8len = WideCharToMultiByte(CP_UTF8, 0, buf.data(), (int)buf.size(), NULL, 0, NULL, NULL);
                        if (u8len > 0) {
                            res.resize(u8len);
                            WideCharToMultiByte(CP_UTF8, 0, buf.data(), (int)buf.size(), res.data(), u8len, NULL, NULL);
                            fanta::internal::GetEngineContext().input.ime_result += res;
                        }
                    }
                }
                ImmReleaseContext(hwnd, himc);
            }
            return 0;
    }
    return CallWindowProc(g_PrevWndProc, hwnd, msg, wParam, lParam);
}
#endif

namespace fanta {

struct OpenGLTexture : public internal::GpuTexture {
    GLuint id = 0;
    int w, h;
    OpenGLTexture(int width, int height) : w(width), h(height) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    }
    ~OpenGLTexture() { if(id) glDeleteTextures(1, &id); }
    void upload(const void* data, int width, int height) override {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
    }
    uint64_t get_native_handle() const override { return (uint64_t)id; }
    int width() const override { return w; }
    int height() const override { return h; }
};

class OpenGLBackendMinimal : public Backend {
    GLFWwindow* window = nullptr;
    GLuint program = 0;
    GLuint text_program = 0;
    GLuint vao = 0, vbo = 0;
    int logical_w = 0, logical_h = 0;
    float dpr = 1.0f;
    
    struct StateState {
        float tx = 0, ty = 0;
        float scale = 1.0f;
        internal::Rectangle clip_rect = {0,0,0,0};
    };
    std::vector<StateState> state_stack;
    StateState current_state;
    float wheel_delta = 0;
    std::vector<uint32_t> char_queue;
    std::vector<int> key_queue;

    // Shader Uniforms
    struct {
        GLint pos, size, color, radius, vp, is_sq, wobble;
    } ui_unifs;
    struct {
        GLint pos, size, color, uv0, uv1, atlas, vp;
    } text_unifs;

public:
    bool init(int w, int h, const char* title) override {
        if (!glfwInit()) return false;
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        window = glfwCreateWindow(w, h, title, NULL, NULL);
        if (!window) { glfwTerminate(); return false; }
        
        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, this);
        glfwSetScrollCallback(window, [](GLFWwindow* w, double, double yoff) {
            auto* b = (OpenGLBackendMinimal*)glfwGetWindowUserPointer(w);
            if (b) b->wheel_delta += (float)yoff;
        });
        glfwSetCharCallback(window, [](GLFWwindow* w, uint32_t codepoint) {
            auto* b = (OpenGLBackendMinimal*)glfwGetWindowUserPointer(w);
            if (b) b->char_queue.push_back(codepoint);
        });
        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                auto* b = (OpenGLBackendMinimal*)glfwGetWindowUserPointer(w);
                if (b) b->key_queue.push_back(key);
            }
        });
        
#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(window);
        g_PrevWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)FantaWndProc);
#endif
        
        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) return false;
        
        // Debug Information (User Request)
        printf("[Fanta] OpenGL Initialized\n");
        printf("[Fanta] Version: %s\n", glGetString(GL_VERSION));
        printf("[Fanta] Vendor: %s\n", glGetString(GL_VENDOR));
        printf("[Fanta] Renderer: %s\n", glGetString(GL_RENDERER));
        printf("[Fanta] glBindTexture Address: %p\n", (void*)glBindTexture);
        
        if (!glBindTexture) {
            printf("[Fanta] FATAL: GLAD failed to load glBindTexture!\n");
            return false;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // --- Shared Vertex Shader ---
        const char* vs = R"(#version 330 core
layout(location=0) in vec2 aPos;
uniform vec2 uShapePos, uShapeSize, uViewport;
out vec2 vLocal; // Distance from shape center in pixels
out vec2 vUV;    // 0..1 across the shape quad (0,0 at top-left)
void main() {
    // aPos is {-1,-1, 1,-1, -1,1, 1,1}
    // We want aPos.y = 1 to be screen.y = 0 (Top)
    // and aPos.y = -1 to be screen.y = uViewport.y (Bottom)
    gl_Position = vec4(aPos, 0.0, 1.0);
    
    vec2 normalized = vec2(aPos.x * 0.5 + 0.5, 0.5 - aPos.y * 0.5);
    vec2 screen = normalized * uViewport;
    
    vLocal = screen - (uShapePos + uShapeSize * 0.5);
    vUV = normalized; 
}
)";

        // --- UI Shape Shader ---
        const char* ui_fs = R"(#version 330 core
in vec2 vLocal;
uniform vec2 uShapeSize;
uniform vec4 uColor;
uniform float uRadius;
uniform bool uIsSquircle;
uniform vec2 uWobble;
out vec4 FragColor;
float sdf_rr(vec2 p, vec2 s, float r) {
    if (uIsSquircle) {
        r = min(r, min(s.x, s.y) * 0.5);
        vec2 d = abs(p) - (s - r);
        d = max(d, 0.0) / r;
        return (pow(d.x, 2.4) + pow(d.y, 2.4) - 1.0) * r;
    }
    r = min(r, min(s.x, s.y) * 0.5);
    vec2 d = abs(p) - s + r;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;
}
void main() {
    vec2 p = vLocal;
    p += uWobble * 5.0 * sin(p.y * 0.1 + p.x * 0.05);
    float d = sdf_rr(p, uShapeSize * 0.5, uRadius);
    
    // Soften edges for shadows (approximate blur)
    float edge = (uColor.a < 0.3) ? 10.0 : 1.0;
    float a = 1.0 - smoothstep(-edge, edge, d);
    FragColor = vec4(uColor.rgb, uColor.a * a);
}
)";

        // --- SDF Text Shader ---
        const char* text_fs = R"(#version 330 core
in vec2 vLocal;
uniform sampler2D uAtlas;
uniform vec2 uShapeSize;
uniform vec2 uUV0, uUV1;
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    // Correct clipping for the glyph quad
    vec2 halfSize = uShapeSize * 0.5;
    if (abs(vLocal.x) > halfSize.x || abs(vLocal.y) > halfSize.y) {
        discard;
    }

    // Local 0..1 UV within the quad
    vec2 local_uv = vLocal / uShapeSize + 0.5;
    
    // Mix atlas UVs based on local quad position
    vec2 uv = mix(uUV0, uUV1, local_uv);
    float dist = texture(uAtlas, uv).r;
    
    // SDF rendering
    float a = smoothstep(0.40, 0.60, dist);
    FragColor = vec4(uColor.rgb, uColor.a * a);
}
)";

        auto link = [&](const char* vsrc, const char* fsrc) {
            GLuint v = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(v, 1, &vsrc, 0); glCompileShader(v);
            GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(f, 1, &fsrc, 0); glCompileShader(f);
            GLuint p = glCreateProgram();
            glAttachShader(p, v); glAttachShader(p, f);
            glLinkProgram(p);
            glDeleteShader(v); glDeleteShader(f);
            return p;
        };

        program = link(vs, ui_fs);
        text_program = link(vs, text_fs);

        ui_unifs.pos = glGetUniformLocation(program, "uShapePos");
        ui_unifs.size = glGetUniformLocation(program, "uShapeSize");
        ui_unifs.color = glGetUniformLocation(program, "uColor");
        ui_unifs.radius = glGetUniformLocation(program, "uRadius");
        ui_unifs.vp = glGetUniformLocation(program, "uViewport");
        ui_unifs.is_sq = glGetUniformLocation(program, "uIsSquircle");
        ui_unifs.wobble = glGetUniformLocation(program, "uWobble");

        text_unifs.pos = glGetUniformLocation(text_program, "uShapePos");
        text_unifs.size = glGetUniformLocation(text_program, "uShapeSize");
        text_unifs.color = glGetUniformLocation(text_program, "uColor");
        text_unifs.uv0 = glGetUniformLocation(text_program, "uUV0");
        text_unifs.uv1 = glGetUniformLocation(text_program, "uUV1");
        text_unifs.atlas = glGetUniformLocation(text_program, "uAtlas");
        text_unifs.vp = glGetUniformLocation(text_program, "uViewport");
        
        // Fullscreen Quad
        float q[] = {-1,-1, 1,-1, -1,1, 1,1};
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(q), q, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        
        logical_w = w;
        logical_h = h;
        
        return true;
    }
    
    void shutdown() override {
        if (program) glDeleteProgram(program);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    bool is_running() override { return !glfwWindowShouldClose(window); }
    
    void begin_frame(int w, int h) override {
        glfwPollEvents();
        
        // Reset transform/clip state
        state_stack.clear();
        current_state = StateState{};
        current_state.clip_rect = {0, 0, (float)w, (float)h};
        glDisable(GL_SCISSOR_TEST);

        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        dpr = (float)dw / (float)w;
        logical_w = w; logical_h = h;
        glClearColor(0.07f, 0.07f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    void end_frame() override { glfwSwapBuffers(window); }
    
    void render(const internal::DrawList& dl) override {
        // 0. Update Glyph Atlas if needed (Phase D)
        internal::GlyphCache::Get().update_texture();

        GLuint last_prog = 0;
        glBindVertexArray(vao);
        
        auto setup_ui_prog = [&]() {
            if (last_prog == program) return;
            glUseProgram(program);
            glUniform2f(ui_unifs.vp, (float)logical_w, (float)logical_h);
            last_prog = program;
        };
        auto setup_text_prog = [&]() {
            if (last_prog == text_program) return;
            glUseProgram(text_program);
            glUniform2f(text_unifs.vp, (float)logical_w, (float)logical_h);
            auto atlas = internal::GlyphCache::Get().get_atlas_texture();
            if (atlas) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, (GLuint)atlas->get_native_handle());
                glUniform1i(text_unifs.atlas, 0);
            }
            last_prog = text_program;
        };

        for (const auto& cmd : dl.commands) {
            if (cmd.type == internal::DrawCmdType::RoundedRect) {
                setup_ui_prog();
                if (cmd.rounded_rect.elevation > 0) {
                    float elevation = cmd.rounded_rect.elevation;
                    glUniform4f(ui_unifs.color, 0, 0, 0, 0.15f);
                    float expand = elevation * 0.5f;
                    glUniform2f(ui_unifs.pos, cmd.rounded_rect.pos_x + current_state.tx - expand, 
                                             cmd.rounded_rect.pos_y + current_state.ty - expand + elevation * 0.3f);
                    glUniform2f(ui_unifs.size, cmd.rounded_rect.size_x + expand * 2, 
                                              cmd.rounded_rect.size_y + expand * 2);
                    glUniform1f(ui_unifs.radius, cmd.rounded_rect.radius + expand);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }

                glUniform2f(ui_unifs.pos, cmd.rounded_rect.pos_x + current_state.tx, cmd.rounded_rect.pos_y + current_state.ty);
                glUniform2f(ui_unifs.size, cmd.rounded_rect.size_x, cmd.rounded_rect.size_y);
                glUniform4f(ui_unifs.color, cmd.rounded_rect.color_r, cmd.rounded_rect.color_g, cmd.rounded_rect.color_b, cmd.rounded_rect.color_a);
                glUniform1f(ui_unifs.radius, cmd.rounded_rect.radius);
                glUniform1i(ui_unifs.is_sq, cmd.rounded_rect.is_squircle ? 1 : 0);
                glUniform2f(ui_unifs.wobble, 0.0f, 0.0f); // Wobble removed from DrawCmd
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
            else if (cmd.type == internal::DrawCmdType::Glyph) {
                setup_text_prog();
                glUniform2f(text_unifs.pos, cmd.glyph.pos_x + current_state.tx, cmd.glyph.pos_y + current_state.ty);
                glUniform2f(text_unifs.size, cmd.glyph.size_x, cmd.glyph.size_y);
                glUniform2f(text_unifs.uv0, cmd.glyph.u0, cmd.glyph.v0);
                glUniform2f(text_unifs.uv1, cmd.glyph.u1, cmd.glyph.v1);
                glUniform4f(text_unifs.color, cmd.glyph.color_r, cmd.glyph.color_g, cmd.glyph.color_b, cmd.glyph.color_a);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
            else if (cmd.type == internal::DrawCmdType::PushTransform) {
                state_stack.push_back(current_state);
                current_state.tx += cmd.transform.tx;
                current_state.ty += cmd.transform.ty;
                current_state.scale *= cmd.transform.scale;
            }
            else if (cmd.type == internal::DrawCmdType::PopTransform) {
                if (!state_stack.empty()) {
                    current_state = state_stack.back();
                    state_stack.pop_back();
                }
            }
            else if (cmd.type == internal::DrawCmdType::PushClip) {
                state_stack.push_back(current_state);
                float cx = (cmd.clip.x + current_state.tx) * dpr;
                float cy = (float)logical_h * dpr - (cmd.clip.y + current_state.ty + cmd.clip.h) * dpr;
                float cw = cmd.clip.w * dpr;
                float ch = cmd.clip.h * dpr;
                glEnable(GL_SCISSOR_TEST);
                glScissor((GLint)cx, (GLint)cy, (GLsizei)cw, (GLsizei)ch);
            }
            else if (cmd.type == internal::DrawCmdType::PopClip) {
                if (!state_stack.empty()) {
                    current_state = state_stack.back();
                    state_stack.pop_back();
                }
                glDisable(GL_SCISSOR_TEST);
            }
            else if (cmd.type == internal::DrawCmdType::BlurRect) {
                // TODO: Implement Dual Kawase Blur or Copy Texture
            }
            else if (cmd.type == internal::DrawCmdType::Bezier) {
                // TODO: Implement Bezier Tesselation
            }
        }
    }
    
    void get_mouse_state(float& x, float& y, bool& down, float& wheel) override {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        x = (float)mx; y = (float)my;
        down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        wheel = wheel_delta;
        wheel_delta = 0;
    }
    
    void get_keyboard_events(std::vector<uint32_t>& chars, std::vector<int>& keys) override {
        chars = std::move(char_queue);
        keys = std::move(key_queue);
        char_queue.clear();
        key_queue.clear();
    }
    
    internal::GpuTexturePtr create_texture(int w, int h, internal::TextureFormat) override { 
        return std::make_shared<OpenGLTexture>(w, h); 
    }
    Capabilities capabilities() const override { return {}; }
    
#ifndef NDEBUG
    bool capture_screenshot(const char* filename) override { 
        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        std::vector<uint8_t> pixels(dw * dh * 4);
        glReadPixels(0, 0, dw, dh, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        
        // Flip Y
        std::vector<uint8_t> flipped(dw * dh * 4);
        for (int y = 0; y < dh; ++y) {
            memcpy(&flipped[y * dw * 4], &pixels[(dh - 1 - y) * dw * 4], dw * 4);
        }
        
        return stbi_write_png(filename, dw, dh, 4, flipped.data(), dw * 4) != 0;
    }
#endif
};

std::unique_ptr<Backend> CreateOpenGLMinimalBackend() {
    return std::make_unique<OpenGLBackendMinimal>();
}

} // namespace fanta
