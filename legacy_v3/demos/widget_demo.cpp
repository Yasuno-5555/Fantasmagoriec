#define NOMINMAX
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "fanta_core_unified.hpp"
#include "fanta_widgets_unified.hpp"
#include <iostream>

using namespace fanta;

// Simple GL Backend
class GLRenderer : public RenderContext {
public:
    void rect(const cmd::RectCmd& c) override {
        // glColor4f(c.s.bg.r, c.s.bg.g, c.s.bg.b, c.s.bg.a);
        // glRectf(c.b.x, c.b.y, c.b.x+c.b.w, c.b.y+c.b.h);
    }
    void text(const cmd::TextCmd& c) override {}
    void layer_begin(const cmd::LayerBeginCmd& c) override {}
    void layer_end(const cmd::LayerEndCmd& c) override {}
};

// Global link
namespace fanta { void set_global_ctx(UIContext* c); }

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Clean Rebuild", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();

    UIContext ctx;
    fanta::set_global_ctx(&ctx);
    
    PaintList paint;
    GLRenderer renderer;

    while (!glfwWindowShouldClose(window)) {
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        
        ctx.begin_frame();
        {
            Label("Fresh Text").build();
            Button("Glass").blur(10).backdrop(true).bg(Color::Hex(0xFFFFFF20)).height(50).build();
        }
        ctx.end_frame();
        
        ctx.solve_layout(w, h);
        
        glViewport(0,0,w,h);
        glClearColor(0.1f,0.1f,0.1f,1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render
        for(auto id : ctx.store.nodes) {
            // Trivial render loop
            auto& l = ctx.store.layout[id];
            auto& s = ctx.store.style[id];
            auto& r = ctx.store.render[id];
            
            if(r.blur > 0) paint.layer_begin({l.x,l.y,l.w,l.h}, r.blur, r.backdrop);
            if(s.bg.a > 0) paint.rect({l.x,l.y,l.w,l.h}, s);
            if(r.blur > 0) paint.layer_end();
        }
        paint.flush(renderer);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
