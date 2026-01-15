#include "backend/drawlist.hpp"
#include "core/font_types.hpp"
#include <algorithm>

namespace fanta {
namespace internal {

DrawList::DrawList() = default;
DrawList::~DrawList() = default;
void DrawList::clear() {
    commands.clear();
    text_runs.clear();
    paths.clear();
}

void DrawList::add_rect(const Vec2& pos, const Vec2& size, const ColorF& c, float elevation, const Vec2& wobble) {
    (void)wobble;
    DrawCmd cmd{};
    cmd.type = DrawCmdType::Rectangle;
    cmd.rectangle.pos_x = pos.x;
    cmd.rectangle.pos_y = pos.y;
    cmd.rectangle.size_x = size.x;
    cmd.rectangle.size_y = size.y;
    cmd.rectangle.color_r = c.r;
    cmd.rectangle.color_g = c.g;
    cmd.rectangle.color_b = c.b;
    cmd.rectangle.color_a = c.a;
    cmd.rectangle.elevation = elevation;
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

void DrawList::add_rounded_rect(const Vec2& pos, const Vec2& size, float r, const ColorF& c, float elevation, bool squircle, float blur, const Vec2& wobble) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::RoundedRect;
    cmd.rounded_rect.pos_x = pos.x;
    cmd.rounded_rect.pos_y = pos.y;
    cmd.rounded_rect.size_x = size.x;
    cmd.rounded_rect.size_y = size.y;
    cmd.rounded_rect.radius = r;
    cmd.rounded_rect.color_r = c.r;
    cmd.rounded_rect.color_g = c.g;
    cmd.rounded_rect.color_b = c.b;
    cmd.rounded_rect.color_a = c.a;
    cmd.rounded_rect.elevation = elevation;
    cmd.rounded_rect.is_squircle = squircle;
    cmd.rounded_rect.backdrop_blur = blur;
    cmd.rounded_rect.wobble_x = wobble.x;
    cmd.rounded_rect.wobble_y = wobble.y;
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

void DrawList::add_circle(const Vec2& center, float r, const ColorF& c, float elevation) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::Circle;
    cmd.circle.center_x = center.x;
    cmd.circle.center_y = center.y;
    cmd.circle.r = r;
    cmd.circle.color_r = c.r;
    cmd.circle.color_g = c.g;
    cmd.circle.color_b = c.b;
    cmd.circle.color_a = c.a;
    cmd.circle.elevation = elevation;
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

void DrawList::add_line(const Vec2& p0, const Vec2& p1, float thickness, const ColorF& c) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::Line;
    cmd.line.p0_x = p0.x;
    cmd.line.p0_y = p0.y;
    cmd.line.p1_x = p1.x;
    cmd.line.p1_y = p1.y;
    cmd.line.thickness = thickness;
    cmd.line.color_r = c.r;
    cmd.line.color_g = c.g;
    cmd.line.color_b = c.b;
    cmd.line.color_a = c.a;
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

void DrawList::add_text(const TextRun& run, const ColorF& c) {
    if (run.empty()) return;
    DrawCmd cmd{};
    cmd.type = DrawCmdType::Text;
    cmd.text.run_index = static_cast<uint32_t>(text_runs.size());
    cmd.text.color_r = c.r;
    cmd.text.color_g = c.g;
    cmd.text.color_b = c.b;
    cmd.text.color_a = c.a;
    cmd.blend = current_blend;
    text_runs.push_back(run);
    commands.push_back(cmd);
}

void DrawList::add_text(const Vec2& pos, const Vec2& size, const Vec4& uv, const ColorF& c) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::Glyph; // New type!
    cmd.glyph.pos_x = pos.x;
    cmd.glyph.pos_y = pos.y;
    cmd.glyph.size_x = size.x;
    cmd.glyph.size_y = size.y;
    cmd.glyph.u0 = uv.x;
    cmd.glyph.v0 = uv.y;
    cmd.glyph.u1 = uv.z;
    cmd.glyph.v1 = uv.w;
    cmd.glyph.color_r = c.r;
    cmd.glyph.color_g = c.g;
    cmd.glyph.color_b = c.b;
    cmd.glyph.color_a = c.a;
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

void DrawList::add_bezier(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, float thickness, const ColorF& c) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::Bezier;
    cmd.bezier.p0_x = p0.x; cmd.bezier.p0_y = p0.y;
    cmd.bezier.p1_x = p1.x; cmd.bezier.p1_y = p1.y;
    cmd.bezier.p2_x = p2.x; cmd.bezier.p2_y = p2.y;
    cmd.bezier.p3_x = p3.x; cmd.bezier.p3_y = p3.y;
    cmd.bezier.thickness = thickness;
    cmd.bezier.color_r = c.r; cmd.bezier.color_g = c.g; cmd.bezier.color_b = c.b; cmd.bezier.color_a = c.a;
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

void DrawList::add_blur_rect(const Vec2& pos, const Vec2& size, float r, float strength) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::BlurRect;
    cmd.blur_rect.pos_x = pos.x;
    cmd.blur_rect.pos_y = pos.y;
    cmd.blur_rect.size_x = size.x;
    cmd.blur_rect.size_y = size.y;
    cmd.blur_rect.radius = r;
    cmd.blur_rect.strength = strength;
    // Blend mode might affect how blur is composited (e.g. if we want to mask it)
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

void DrawList::push_transform(const Vec2& translate, float scale) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::PushTransform;
    cmd.transform.tx = translate.x;
    cmd.transform.ty = translate.y;
    cmd.transform.scale = scale;
    commands.push_back(cmd);
}

void DrawList::pop_transform() {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::PopTransform;
    commands.push_back(cmd);
}

void DrawList::push_clip(const Vec2& pos, const Vec2& size) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::PushClip;
    cmd.clip.x = pos.x;
    cmd.clip.y = pos.y;
    cmd.clip.w = size.x;
    cmd.clip.h = size.y;
    commands.push_back(cmd);
}

void DrawList::pop_clip() {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::PopClip;
    commands.push_back(cmd);
}

void DrawList::add_path(const Path& p) {
    DrawCmd cmd{};
    cmd.type = DrawCmdType::Path;
    paths.push_back(p);
    cmd.path_cmd.path_index = static_cast<uint32_t>(paths.size() - 1);
    cmd.blend = current_blend;
    commands.push_back(cmd);
}

} // namespace internal
} // namespace fanta
