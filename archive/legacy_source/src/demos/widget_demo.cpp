// Fantasmagorie v3 - Widget Demo (Final Refinement)
#define NOMINMAX
#include <windows.h>
#include <glad.h>
#include "fanta_core_unified.hpp"
#include "fanta_render_unified.hpp"
#include "widgets/button.hpp"
#include "widgets/slider.hpp"
#include "widgets/label.hpp"
#include "widgets/checkbox.hpp"
#include "widgets/window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>

using namespace fanta;

// Demo State
float blur_val = 20.0f;
bool enable_glass = true;

void main_app(UIContext& ctx, PaintList& paint) {
    // Rich API Styling
    Label("Fantasmagorie Unified").is_bold().build();
    
    {
        // Deep access and standard controls
        Slider("Blur Intensity", &blur_val, 0.0f, 50.0f).width(250).build();
        
        if (enable_glass) {
            Button("Glass Panel")
                .width(400).height(200)
                .blur(blur_val).backdrop(true)
                .bg(Color::Hex(0xFFFFFF20))
                .radius(16.0f)
                .border(Color::Hex(0xFFFFFF40), 1.5f)
                .shadow(Shadow::Level1())
                .build();
        }
    }
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Fantasmagorie v3", NULL, NULL);
    glfwMakeContextCurrent(window);
    
    // Core Logic
    load_gl_functions();
    
    UIContext ctx;
    PaintList paint;
    set_context(&ctx, &paint);
    
    // Note: RenderBackend and other system classes need to be linked
    // For now we focus on the core logic build.
    
    while (!glfwWindowShouldClose(window)) {
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        
        ctx.begin_frame();
        main_app(ctx, paint);
        ctx.end_frame();
        
        ctx.solve_layout(w, h);
        
        glViewport(0, 0, w, h);
        glClearColor(0.05f, 0.05f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // This should now be found
        
        // Render commands execution would go here
        UIRenderer::render(ctx.store, paint);
        paint.flush(*(RenderContext*)nullptr); // Placeholder for actual context
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
