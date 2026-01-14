// Fantasmagorie v3 - OpenGL Backend (Consolidated)
#define NOMINMAX
#include <windows.h>
#include <glad/glad.h>
#include "render/render_context.hpp"
// #include "platform/opengl.hpp" // If needed, or just define RenderBackend here

namespace fanta {

// Minimal backend required for WidgetDemo linking
class RenderContextImplementation : public RenderContext {
public:
    void draw_rect(const cmd::RectCmd&) override {}
    void draw_circle(const cmd::CircleCmd&) override {}
    void draw_line(const cmd::LineCmd&) override {}
    void draw_text(const cmd::TextCmd&) override {}
    void layer_begin(const cmd::LayerBeginCmd&) override {}
    void layer_end(const cmd::LayerEndCmd&) override {}
};

RenderBackend::RenderBackend() : impl_(std::make_unique<RenderContextImplementation>()) {}
RenderBackend::~RenderBackend() = default;
bool RenderBackend::init() { return true; }
void RenderBackend::shutdown() {}
void RenderBackend::begin_frame(int w, int h, float dpr) {}
void RenderBackend::end_frame() {}
void RenderBackend::execute(const PaintList& list) {}

// Global
void load_gl_functions() { gladLoadGL(); }

}
