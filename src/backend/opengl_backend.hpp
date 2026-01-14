#pragma once
#include "backend.hpp"
#include "core/types_internal.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace fanta {

class OpenGLBackend : public Backend {
    GLFWwindow* window = nullptr;
    
    // DPI context
    float device_pixel_ratio = 1.0f;
    int logical_width = 0;
    int logical_height = 0;
    
    // Render Resources
    GLuint sdf_program = 0;
    GLuint quad_vao = 0, quad_vbo = 0;
    GLuint text_program = 0;
    GLuint text_vao = 0, text_vbo = 0;
    
    bool compile_sdf_shader();
    bool compile_text_shader(); 
    void setup_quad();
    void setup_text_renderer(); 
    
    float scroll_accum = 0; // Phase 8
    
public:
    bool init(int w, int h, const char* title) override;
    void shutdown() override;
    bool is_running() override;
    void begin_frame(int w, int h) override;
    void end_frame() override;
    void render(const fanta::internal::DrawList& draw_list) override;
    void get_mouse_state(float& x, float& y, bool& down, float& wheel) override;
    Capabilities capabilities() const override { return { false }; }
    
    // Phase 5.2: Screenshot capture
    bool capture_screenshot(const char* filename);
};

} // namespace fanta
