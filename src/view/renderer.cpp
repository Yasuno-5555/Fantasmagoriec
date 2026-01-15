// UI Layer: Stateless Renderer
// Renders View AST using computed_rect from Flexbox layout
// DrawList is God. All output goes through it.

#include "view/definitions.hpp"
#include "view/interaction.hpp"
#include "core/contexts_internal.hpp"
#include "backend/drawlist.hpp"
#include "text/font_manager.hpp"
#include "text/glyph_cache.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "core/utf8.hpp"
#include <cstring>
#include <cmath>

#include "view/transition_eval.hpp"

namespace fanta {
namespace ui {

    void ComputeFlexLayout(ViewHeader* root, float screen_w, float screen_h);

    // --- Render Phase ---
    // Universal Masquerade: Every widget draws its own common visual props.
    
    void RenderViewRecursive(const ViewHeader* view, internal::DrawList& dl, int depth = 0) {
        if (!view) return;
        
        const auto& rect = view->computed_rect;
        
        // Phase 10: Rainbow Borders (Layout Depth Inspect)
        if (internal::GetEngineContext().config.show_debug_overlay) {
            float hue = fmod(depth * 30.0f, 360.0f);
            float h1 = hue / 60.0f;
            float x = (1.0f - std::abs(std::fmod(h1, 2.0f) - 1.0f));
            float r=0, g=0, b=0;
            if(h1 < 1) { r=1; g=x; b=0; }
            else if(h1 < 2) { r=x; g=1; b=0; }
            else if(h1 < 3) { r=0; g=1; b=x; }
            else if(h1 < 4) { r=0; g=x; b=1; }
            else if(h1 < 5) { r=x; g=0; b=1; }
            else { r=1; g=0; b=x; }
            dl.add_rounded_rect({rect.x, rect.y}, {rect.w, rect.h}, 0, {r, g, b, 0.3f});
        }

        // --- 1. Universal Background rendering (Shadow, Blur, BG) ---
        // Every widget can have these now.
        if (view->backdrop_blur > 0.0f) {
            dl.add_blur_rect({rect.x, rect.y}, {rect.w, rect.h}, view->border_radius, view->backdrop_blur);
        }

        if (view->bg_color.a > 0 || view->elevation > 0) {
            dl.add_rounded_rect(
                {rect.x, rect.y},
                {rect.w, rect.h},
                view->border_radius,
                view->bg_color,
                view->elevation,
                view->is_squircle, 
                0.0f, 
                {view->wobble_x, view->wobble_y}
            );
        }

        // --- 2. Type-Specific Rendering ---
        if (view->type == ViewType::Text) {
            const auto* txt = static_cast<const TextView*>(view);
            if (txt->text && txt->text[0] != '\0') {
                float pen_x = rect.x;
                float pen_y = rect.y + rect.h * 0.8f; 
                float scale = txt->font_size / (float)internal::FontManager::BASE_SDF_SIZE;
                const char* p = txt->text;
                while (*p) {
                    uint32_t codepoint = internal::NextUTF8(p);
                    auto [font_id, glyph_idx] = internal::FontManager::Get().get_glyph_index(codepoint);
                    float u0, v0, u1, v1, gw, gh, adv;
                    if (internal::GlyphCache::Get().get_glyph_uv(font_id, glyph_idx, u0, v0, u1, v1, gw, gh, adv)) {
                        dl.add_text({pen_x, pen_y - gh * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, txt->fg_color);
                        pen_x += adv * scale;
                    }
                }
            }
        }
        else if (view->type == ViewType::Button) {
            const auto* btn = static_cast<const ButtonView*>(view);
            bool is_hot = interaction::IsHot(btn->id);
            bool is_active = interaction::IsActive(btn->id);
            internal::ColorF bg = btn->bg_color;
            if (is_active) bg = btn->active_color;
            else if (is_hot) bg = btn->hover_color;
            internal::ColorF anim_bg = EvaluateTransitionColor(btn->id, bg);
            
            // Re-render button background with interactive color
            dl.add_rounded_rect({rect.x, rect.y}, {rect.w, rect.h}, btn->border_radius, anim_bg, is_active ? 0.0f : btn->elevation, btn->is_squircle, 0.0f, {0, 0});
            
            if (btn->label && btn->label[0] != '\0') {
                float text_w = 0;
                float scale = btn->font_size / (float)internal::FontManager::BASE_SDF_SIZE;
                const char* p = btn->label; const char* start = p;
                while (*p) {
                    uint32_t codepoint = internal::NextUTF8(p);
                    auto [font_id, glyph_idx] = internal::FontManager::Get().get_glyph_index(codepoint);
                    text_w += internal::FontManager::Get().get_metrics(font_id, glyph_idx).advance * scale;
                }
                float pen_x = rect.x + (rect.w - text_w) * 0.5f;
                float pen_y = rect.y + (rect.h + btn->font_size * 0.6f) * 0.5f; 
                p = start; 
                while (*p) {
                    uint32_t codepoint = internal::NextUTF8(p);
                    auto [font_id, glyph_idx] = internal::FontManager::Get().get_glyph_index(codepoint);
                    float u0, v0, u1, v1, gw, gh, adv;
                    if (internal::GlyphCache::Get().get_glyph_uv(font_id, glyph_idx, u0, v0, u1, v1, gw, gh, adv)) {
                        dl.add_text({pen_x, pen_y - gh * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, btn->fg_color);
                        pen_x += adv * scale;
                    }
                }
            }
        }
        else if (view->type == ViewType::TextInput || view->type == ViewType::TextArea) {
            // Text area and input handle their own complex text rendering
            // but can now use the Universal Background rendered above!
            // We just need to render the internal text/cursor.
            
            bool is_area = (view->type == ViewType::TextArea);
            const char* buf = is_area ? static_cast<const TextAreaView*>(view)->buffer : static_cast<const TextInputView*>(view)->buffer;
            const char* placeholder = is_area ? static_cast<const TextAreaView*>(view)->placeholder : static_cast<const TextInputView*>(view)->placeholder;
            bool focused = interaction::IsFocused(view->id);
            
            const char* display_text = (buf && buf[0] != '\0') ? buf : placeholder;
            internal::ColorF text_color = (buf && buf[0] != '\0') ? view->fg_color : internal::ColorF{0.4f,0.4f,0.45f,1};
            
            float scale = view->font_size / (float)internal::FontManager::BASE_SDF_SIZE;
            float pen_x = rect.x + 8;
            float pen_y = rect.y + (rect.h + view->font_size * 0.6f) * 0.5f;
            
            if (display_text) {
                const char* p = display_text;
                while (*p) {
                    uint32_t codepoint = internal::NextUTF8(p);
                    auto [font_id, glyph_idx] = internal::FontManager::Get().get_glyph_index(codepoint);
                    float u0, v0, u1, v1, gw, gh, adv;
                    if (internal::GlyphCache::Get().get_glyph_uv(font_id, glyph_idx, u0, v0, u1, v1, gw, gh, adv)) {
                        dl.add_text({pen_x, pen_y - gh * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, text_color);
                        pen_x += adv * scale;
                    }
                }
            }
            
            if (focused && (fmod(internal::GetEngineContext().frame.time, 1.0) < 0.5)) {
                dl.add_rounded_rect({pen_x, rect.y + 4}, {2, rect.h - 8}, 0, {0.3f, 0.6f, 1.0f, 1.0f});
            }
        }
        else if (view->type == ViewType::Toggle) {
            const auto* toggle = static_cast<const ToggleView*>(view);
            bool val = toggle->value ? *toggle->value : false;
            internal::ColorF track_color = val ? internal::ColorF{0.2f, 0.6f, 0.4f, 1.0f} : internal::ColorF{0.2f, 0.2f, 0.25f, 1.0f};
            dl.add_rounded_rect({rect.x, rect.y + rect.h*0.25f}, {40, rect.h*0.5f}, 10.0f, track_color);
            float thumb_x = val ? rect.x + 22 : rect.x + 2;
            dl.add_rounded_rect({thumb_x, rect.y + rect.h*0.25f + 2}, {16, rect.h*0.5f - 4}, 8.0f, {0.9f, 0.9f, 1.0f, 1.0f});
            if (toggle->label) {
                float scale = 14.0f / (float)internal::FontManager::BASE_SDF_SIZE;
                float lx = rect.x + 50; float ly = rect.y + (rect.h + 14.0f * 0.6f) * 0.5f;
                for (const char* p = toggle->label; *p; ) {
                    uint32_t cp = internal::NextUTF8(p);
                    auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                    float u0, v0, u1, v1, gw, gh, adv;
                    if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv)) {
                        dl.add_text({lx, ly - gh * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, toggle->fg_color);
                        lx += adv * scale;
                    }
                }
            }
        }
        else if (view->type == ViewType::Slider) {
            const auto* slider = static_cast<const SliderView*>(view);
            float val = slider->value ? *slider->value : 0;
            float t = (val - slider->min) / (slider->max - slider->min);
            t = std::max(0.0f, std::min(1.0f, t));
            dl.add_rounded_rect({rect.x, rect.y + rect.h*0.4f}, {rect.w, rect.h*0.2f}, 4.0f, {0.15f, 0.15f, 0.2f, 1.0f});
            dl.add_rounded_rect({rect.x, rect.y + rect.h*0.4f}, {rect.w * t, rect.h*0.2f}, 4.0f, {0.3f, 0.5f, 0.8f, 1.0f});
            dl.add_rounded_rect({rect.x + rect.w * t - 6, rect.y + rect.h*0.2f}, {12, rect.h*0.6f}, 4.0f, {0.9f, 0.9f, 1.0f, 1.0f}, 4.0f);
        }
        else if (view->type == ViewType::Scroll) {
            auto& persist = internal::GetEngineContext().persistent;
            internal::Vec2 offset = persist.scroll_offsets[view->id.value];
            dl.push_clip({rect.x, rect.y}, {rect.w, rect.h});
            dl.push_transform({-offset.x, -offset.y}, 1.0f);
            for (ViewHeader* child = view->first_child; child; child = child->next_sibling) RenderViewRecursive(child, dl, depth + 1);
            dl.pop_transform();
            dl.pop_clip();
            return; 
        }
        else if (view->type == ViewType::Bezier) {
            const auto* bz = static_cast<const BezierView*>(view);
            dl.add_bezier({rect.x+bz->p0[0],rect.y+bz->p0[1]},{rect.x+bz->p1[0],rect.y+bz->p1[1]},{rect.x+bz->p2[0],rect.y+bz->p2[1]},{rect.x+bz->p3[0],rect.y+bz->p3[1]},bz->thickness,bz->fg_color);
            return;
        }

        // --- 3. Default Child Recursion ---
        for (ViewHeader* child = view->first_child; child; child = child->next_sibling) {
            RenderViewRecursive(child, dl, depth + 1);
        }
    }

    // --- Public Entry Point ---
    void RenderUI(ViewHeader* root, float screen_w, float screen_h, internal::DrawList& dl) {
        if (!root) return;
        auto& ctx = internal::GetEngineContext();
        ctx.debug.clear();
        if (ctx.config.deterministic_mode) ctx.frame.dt = ctx.config.fixed_dt;
        interaction::BeginInteractionPass();
        ComputeFlexLayout(root, screen_w, screen_h);
        RenderViewRecursive(root, dl);
        
        // Phase 10: Debug Overlay
        if (ctx.config.show_debug_overlay && !ctx.debug.commands.empty()) {
            float y_off = 10.0f, x_off = 10.0f, font_size = 14.0f, line_h = font_size * 1.5f;
            dl.add_rounded_rect({x_off-5, y_off-5}, {300, 20.0f + ctx.debug.commands.size() * line_h}, 4.0f, {0, 0, 0, 0.6f}, 0.0f);
            for (const auto& cmd : ctx.debug.commands) {
                if (cmd.type == internal::DebugOverlayCmd::Rect) dl.add_rounded_rect({cmd.x, cmd.y}, {cmd.w, cmd.h}, 0.0f, cmd.color);
                else {
                    float pen_x = x_off, pen_y = y_off + font_size, scale = font_size / (float)internal::FontManager::BASE_SDF_SIZE;
                    const char* p = cmd.label.c_str();
                    while (*p) {
                        uint32_t cp = internal::NextUTF8(p); auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                        float u0, v0, u1, v1, gw, gh, adv;
                        if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv)) {
                             dl.add_text({pen_x, pen_y - gh * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {0, 1, 0, 1});
                             pen_x += adv * scale;
                        }
                    }
                }
                y_off += line_h;
            }
        }
    }

} // namespace ui
} // namespace fanta
