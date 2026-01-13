#pragma once

#include "types.hpp"
#include "text.hpp"
#include <string>

namespace fanta {


class PaintList;

namespace cmd {
    struct RectCmd;
    struct CircleCmd;
    struct LineCmd;
    struct TextCmd;
}

class RenderContext {
public:
    virtual ~RenderContext() = default;

    int viewport_width = 800;
    int viewport_height = 600;
    float device_pixel_ratio = 1.0f;

    virtual void draw_rect(const cmd::RectCmd& r) = 0;
    virtual void draw_circle(const cmd::CircleCmd& c) = 0;
    virtual void draw_line(const cmd::LineCmd& l) = 0;
    virtual void draw_text(const cmd::TextCmd& t) = 0;
};

} // namespace fanta
