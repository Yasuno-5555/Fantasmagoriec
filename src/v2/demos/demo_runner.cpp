// Fantasmagorie v2 - Visual Demo Runner
// Runs the demo UI in a GLFW window with actual rendering

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "platform/gl_lite.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <vector>

#include "core/context.hpp"
#include "core/layout.hpp"
#include "render/draw_list.hpp"
#include "render/renderer.hpp"
#include "widgets/widget_base.hpp"
#include "style/theme.hpp"

// Forward declaration
namespace fanta {
    void build_demo_ui();
}

// Simple immediate mode renderer
class SimpleRenderer {
public:
    GLuint program_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLint loc_color_ = -1;
    GLint loc_resolution_ = -1;
    int width_ = 800, height_ = 600;
    
    void init() {
        // Vertex shader
        const char* vs = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform vec2 uResolution;
void main() {
    vec2 pos = aPos / uResolution * 2.0 - 1.0;
    pos.y = -pos.y;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";
        // Fragment shader
        const char* fs = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    FragColor = uColor;
}
)";
        
        GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs_id, 1, &vs, nullptr);
        glCompileShader(vs_id);
        
        GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs_id, 1, &fs, nullptr);
        glCompileShader(fs_id);
        
        program_ = glCreateProgram();
        glAttachShader(program_, vs_id);
        glAttachShader(program_, fs_id);
        glLinkProgram(program_);
        
        loc_color_ = glGetUniformLocation(program_, "uColor");
        loc_resolution_ = glGetUniformLocation(program_, "uResolution");
        
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), nullptr, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    }
    
    void set_size(int w, int h) { width_ = w; height_ = h; }
    
    void draw_rect(float x, float y, float w, float h, uint32_t color) {
        float verts[12] = {
            x, y,
            x + w, y,
            x + w, y + h,
            x, y,
            x + w, y + h,
            x, y + h
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        
        float r = ((color >> 24) & 0xFF) / 255.0f;
        float g = ((color >> 16) & 0xFF) / 255.0f;
        float b = ((color >> 8) & 0xFF) / 255.0f;
        float a = (color & 0xFF) / 255.0f;
        
        glUseProgram(program_);
        glUniform4f(loc_color_, r, g, b, a);
        glUniform2f(loc_resolution_, (float)width_, (float)height_);
        
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    void render(fanta::DrawList& list) {
        for (int layer = 0; layer < (int)fanta::RenderLayer::Count; ++layer) {
            for (auto& cmd : list.cmds[layer]) {
                if (cmd.type == fanta::DrawType::Rect) {
                    draw_rect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.color);
                }
            }
        }
    }
};

static SimpleRenderer g_renderer;
static double g_scroll_y = 0;

void scroll_callback(GLFWwindow*, double, double yoffset) {
    g_scroll_y = yoffset;
}

int main() {
    std::cout << "Fantasmagorie v2 - Visual Demo" << std::endl;
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Fantasmagorie v2 Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Load GL functions
    init_gl_lite();
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    g_renderer.init();
    
    // Initialize Fantasmagorie
    fanta::UIContext ctx;
    fanta::DrawList draw_list;
    fanta::set_context(&ctx, &draw_list);
    fanta::use_theme<fanta::MaterialTheme>();
    
    auto last_time = std::chrono::high_resolution_clock::now();
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Input
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        ctx.mouse_pos = fanta::Vec2((float)mx, (float)my);
        ctx.mouse_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        ctx.scroll_delta = (float)g_scroll_y;
        g_scroll_y = 0;
        
        ctx.input_keys.clear();
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            ctx.input_keys.push_back(GLFW_KEY_TAB);
        }
        
        // Frame
        ctx.begin_frame();
        draw_list.clear();
        
        fanta::build_demo_ui();
        
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        fanta::LayoutEngine::solve(ctx.store, ctx.root_id, (float)w, (float)h);
        
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;
        ctx.update_visuals(dt);
        
        fanta::Renderer::submit(ctx.store, ctx.root_id, draw_list);
        ctx.end_frame();
        
        // Render
        glViewport(0, 0, w, h);
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        g_renderer.set_size(w, h);
        g_renderer.render(draw_list);
        
        glfwSwapBuffers(window);
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "Demo ended." << std::endl;
    return 0;
}
