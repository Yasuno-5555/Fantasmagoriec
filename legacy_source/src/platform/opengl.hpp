#pragma once

#include <cstdint>
#include <memory>
#include "render/render_context.hpp"

namespace fanta {

// The RenderContext is now a pure Pimpl facade.
// It contains NO OpenGL-specific types or headers in this file.
class RenderContextImplementation;

class RenderBackend {
public:
    RenderBackend();
    ~RenderBackend();

    bool init();
    void shutdown();

    void begin_frame(int w, int h, float dpr);
    void end_frame();

    // High-level drawing commands (delegated to Impl)
    void draw_rect(const cmd::RectCmd& r);
    void draw_circle(const cmd::CircleCmd& c);
    void draw_line(const cmd::LineCmd& l);
    void draw_text(const cmd::TextCmd& t);
    TypographyStats get_stats() const;

    void execute(const PaintList& list);

private:
    std::unique_ptr<RenderContextImplementation> impl_;
};

// Global-ish access (optional, if you still want the facade style)
void load_gl_functions();

} // namespace fanta


