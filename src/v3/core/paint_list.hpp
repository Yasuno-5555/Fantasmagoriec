// Fantasmagorie v3 - Draw Command System
#pragma once

#include "types.hpp"
#include "path.hpp"
#include "render_context.hpp"
#include <string>
#include <memory>
#include <vector>

namespace fanta {

struct DrawCommand {
    virtual ~DrawCommand() = default;
    virtual uint32_t get_type_code() const = 0;
    virtual void execute(RenderContext& ctx) const = 0;
};

namespace cmd {

struct RectCmd : DrawCommand {
    static constexpr uint32_t ID = 0x0001;
    fanta::Rect bounds;
    CornerRadius radius;
    Color fill = Color::White();
    Color border_color = Color::Clear();
    float border_width = 0;
    ElevationShadow shadow;
    uint32_t get_type_code() const override { return ID; }
    void execute(RenderContext& ctx) const override { ctx.draw_rect(*this); }
};

struct CircleCmd : DrawCommand {
    static constexpr uint32_t ID = 0x0002;
    Vec2 center;
    float radius = 0;
    Color fill = Color::White();
    ElevationShadow shadow;
    uint32_t get_type_code() const override { return ID; }
    void execute(RenderContext& ctx) const override { ctx.draw_circle(*this); }
};

struct LineCmd : DrawCommand {
    static constexpr uint32_t ID = 0x0003;
    Vec2 from;
    Vec2 to;
    float thickness = 1.0f;
    Color color = Color::White();
    uint32_t get_type_code() const override { return ID; }
    void execute(RenderContext& ctx) const override { ctx.draw_line(*this); }
};

struct TextCmd : DrawCommand {
    static constexpr uint32_t ID = 0x1001;
    Vec2 pos;
    std::string content;
    uint32_t font_id = 0;
    float font_size = 14.0f;
    Color color = Color::White();
    uint32_t get_type_code() const override { return ID; }
    void execute(RenderContext& ctx) const override { ctx.draw_text(*this); }
};

// ... clipping and transforms can be added back later if needed to simplify current debug
} 

class PaintList {
public:
    template<typename T, typename... Args>
    T& add(Args&&... args) {
        auto cmd = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = cmd.get();
        commands_.push_back(std::move(cmd));
        return *ptr;
    }
    
    void set_font(uint32_t font_id) { current_font_id = font_id; }
    uint32_t get_font() const { return current_font_id; }

    cmd::RectCmd& rect(Rect bounds, Color fill = Color::White()) {
        auto& r = add<cmd::RectCmd>(); r.bounds = bounds; r.fill = fill; return r;
    }
    
    cmd::CircleCmd& circle(Vec2 center, float radius, Color fill = Color::White()) {
        auto& c = add<cmd::CircleCmd>(); c.center = center; c.radius = radius; c.fill = fill; return c;
    }
    
    cmd::LineCmd& line(Vec2 from, Vec2 to, Color color = Color::White(), float thickness = 1.0f) {
        auto& l = add<cmd::LineCmd>(); l.from = from; l.to = to; l.color = color; l.thickness = thickness; return l;
    }

    cmd::TextCmd& text(Vec2 pos, const std::string& str, float size = 14.0f, Color color = Color::White()) {
        auto& t = add<cmd::TextCmd>(); t.pos = pos; t.content = str; t.font_id = current_font_id; t.font_size = size; t.color = color; return t;
    }

    void flush(RenderContext& ctx) { execute_all(ctx); clear(); }
    void execute_all(RenderContext& ctx) const { for (const auto& cmd_ptr : commands_) cmd_ptr->execute(ctx); }
    void clear() { commands_.clear(); }
    
private:
    std::vector<std::unique_ptr<DrawCommand>> commands_;
    uint32_t current_font_id = 0;
};

} // namespace fanta
