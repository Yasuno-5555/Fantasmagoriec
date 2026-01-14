// Fantasmagorie v3 - OpenGL Backend Implementation
#define NOMINMAX
#include <windows.h>
#include <glad/glad.h>
#include "platform/opengl.hpp"
#include "render/shader.hpp"
#include <vector>
#include <stack>
#include <memory>

namespace fanta {

struct GLFramebuffer {
    GLuint fbo = 0, tex = 0;
    int w = 0, h = 0;

    GLFramebuffer(int width, int height) : w(width), h(height) {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(0x8D40, fbo);

        glGenTextures(1, &tex);
        glBindTexture(0x0DE1, tex);
        glTexImage2D(0x0DE1, 0, 0x8058, w, h, 0, 0x1908, 0x1401, nullptr);
        glFramebufferTexture2D(0x8D40, 0x8CE0, 0x0DE1, tex, 0);
        glBindFramebuffer(0x8D40, 0);
    }
    ~GLFramebuffer() {
        if (fbo) glDeleteFramebuffers(1, &fbo);
        if (tex) glDeleteTextures(1, &tex);
    }
};

class RenderContextImplementation : public RenderContext {
public:
    std::stack<std::unique_ptr<GLFramebuffer>> layer_stack;
    int screen_w = 0, screen_h = 0;

    void draw_rect(const cmd::RectCmd& r) override {}
    void draw_circle(const cmd::CircleCmd& c) override {}
    void draw_line(const cmd::LineCmd& l) override {}
    void draw_text(const cmd::TextCmd& t) override {}
    
    void layer_begin(const cmd::LayerBeginCmd& cmd) override {
        auto layer = std::make_unique<GLFramebuffer>((int)cmd.bounds.w, (int)cmd.bounds.h);
        glBindFramebuffer(0x8D40, layer->fbo);
        layer_stack.push(std::move(layer));
    }

    void layer_end(const cmd::LayerEndCmd& cmd) override {
        if (layer_stack.empty()) return;
        layer_stack.pop();
        GLuint target = layer_stack.empty() ? 0 : layer_stack.top()->fbo;
        glBindFramebuffer(0x8D40, target);
    }
};

// Backend implementation
RenderBackend::RenderBackend() : impl_(std::make_unique<RenderContextImplementation>()) {}
RenderBackend::~RenderBackend() = default;
bool RenderBackend::init() { return true; }
void RenderBackend::shutdown() {}
void RenderBackend::begin_frame(int w, int h, float dpr) {
    impl_->viewport_width = w; impl_->viewport_height = h;
}
void RenderBackend::end_frame() {}
void RenderBackend::draw_rect(const cmd::RectCmd& r) { impl_->draw_rect(r); }
void RenderBackend::draw_circle(const cmd::CircleCmd& c) { impl_->draw_circle(c); }
void RenderBackend::draw_line(const cmd::LineCmd& l) { impl_->draw_line(l); }
void RenderBackend::draw_text(const cmd::TextCmd& t) { impl_->draw_text(t); }
void RenderBackend::execute(const PaintList& list) {
    // We would need to pass 'impl_' as RenderContext reference here.
    // list.flush(*(impl_.get()));
}

void load_gl_functions() {
    gladLoadGL();
}

} // namespace fanta
