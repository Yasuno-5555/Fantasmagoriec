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
#include <iomanip>
#include <cstdio> // snprintf

#include "view/transition_eval.hpp"
#include "core/animation.hpp"
#include "text/markdown.hpp"

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
        // Universal Background rendering (Shadow, Blur, BG)
        // Combine Blur into RoundedRect for single-pass Glass effect
        if (view->bg_color.a >= 0 || view->elevation > 0 || view->backdrop_blur > 0) {
            dl.add_rounded_rect(
                {rect.x, rect.y},
                {rect.w, rect.h},
                view->border_radius,
                view->bg_color,
                view->elevation,
                view->is_squircle,
                view->border_width,
                view->border_color,
                view->glow_strength,
                view->glow_color,
                view->backdrop_blur 
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
                    float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                    if (internal::GlyphCache::Get().get_glyph_uv(font_id, glyph_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                        dl.add_text({pen_x + ox * scale, pen_y - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, txt->fg_color);
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
            dl.add_rounded_rect(
                {rect.x, rect.y}, {rect.w, rect.h}, 
                btn->border_radius, anim_bg, 
                is_active ? 0.0f : btn->elevation, 
                btn->is_squircle, 
                btn->border_width, 
                btn->border_color, 
                btn->glow_strength, 
                btn->glow_color
            );
            
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
                    float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                    if (internal::GlyphCache::Get().get_glyph_uv(font_id, glyph_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                        dl.add_text({pen_x + ox * scale, pen_y - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, btn->fg_color);
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
                    float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                    if (internal::GlyphCache::Get().get_glyph_uv(font_id, glyph_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                        dl.add_text({pen_x + ox * scale, pen_y - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, text_color);
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
                    float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                    if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                        dl.add_text({lx + ox * scale, ly - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, toggle->fg_color);
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

        // --- Phase 50: Specialized Widgets Rendering ---
        
        else if (view->type == ViewType::Knob) {
            const auto* knob = static_cast<const KnobView*>(view);
            float val = knob->value ? *knob->value : knob->min;
            float t = (val - knob->min) / (knob->max - knob->min);
            t = std::clamp(t, 0.0f, 1.0f);
            
            float cx = rect.x + rect.w * 0.5f;
            float cy = rect.y + rect.h * 0.5f;
            float r = std::min(rect.w, rect.h) * 0.4f;
            
            // Base Circle (Bg)
            dl.add_circle({cx, cy}, r, {0.15f, 0.15f, 0.2f, 1.0f});
            
            // Indicator (Line)
            // Angle range: -225 deg to -45 deg (270 degree span, pointing down-left to down-right)
            // 0 -> -225deg, 1 -> +45deg? No, usually start at 7 o'clock, end at 5 o'clock
            // 7 o'clock = 270 - 45 = 225 deg? No.
            // Let's say: Start = 135 deg (bottom-left), End = 405 deg (bottom-right)?
            // Standard audio knob: -150 to +150 degrees from UP?
            // Let's compute in radians.
            // -150 deg = -2.61 rad
            // +150 deg = +2.61 rad
            
            float angle_start = 0.75f * 3.14159f; // 135 deg
            float angle_end = 2.25f * 3.14159f;   // 405 deg
            float angle = angle_start + t * (angle_end - angle_start);
            
            float ix = cx + std::cos(angle) * (r * 0.8f);
            float iy = cy + std::sin(angle) * (r * 0.8f);
            
            // Glow/Bloom Circle
            if (knob->glow_color.a > 0) {
                 // Simple bloom simulation by circle? Or backend handles bloom?
                 // We just draw the indicator with glow color
                 dl.add_line({cx, cy}, {ix, iy}, 4.0f, knob->glow_color);
                 dl.add_circle({ix, iy}, 4.0f, knob->glow_color);
            } else {
                 dl.add_line({cx, cy}, {ix, iy}, 4.0f, {0.9f, 0.9f, 1.0f, 1.0f});
            }
            
            // Label
            if (knob->label) {
                // ... Label rendering reusing common logic if possible, or specialized ...
                // For now minimal implementation
            }
        }

        else if (view->type == ViewType::Fader) {
            const auto* fader = static_cast<const FaderView*>(view);
            float val = fader->value ? *fader->value : fader->min;
            float t = (val - fader->min) / (fader->max - fader->min);
            t = std::clamp(t, 0.0f, 1.0f);

            // Track
            float track_w = 4.0f;
            float track_x = rect.x + (rect.w - track_w) * 0.5f;
            dl.add_rounded_rect({track_x, rect.y}, {track_w, rect.h}, 2.0f, {0.1f, 0.1f, 0.12f, 1.0f});
            
            // Thumb (Handle)
            float thumb_h = 30.0f;
            float thumb_w = rect.w * 0.8f;
            float thumb_x = rect.x + (rect.w - thumb_w) * 0.5f;
            // Cartesion: 0 at bottom
            float avail_h = rect.h - thumb_h;
            float thumb_y = rect.y + avail_h * (1.0f - t); 
            
            dl.add_rounded_rect({thumb_x, thumb_y}, {thumb_w, thumb_h}, 4.0f, {0.15f, 0.15f, 0.18f, 1.0f}, 4.0f); // Shadow
            dl.add_rounded_rect({thumb_x, thumb_y}, {thumb_w, thumb_h}, 4.0f, {0.6f, 0.6f, 0.65f, 1.0f}); // Body
            
            // Center line on thumb
            dl.add_line({thumb_x, thumb_y + thumb_h*0.5f}, {thumb_x+thumb_w, thumb_y + thumb_h*0.5f}, 2.0f, {0.2f,0.2f,0.2f,1.0f});
        }
        
        else if (view->type == ViewType::Dragger) {
            const auto* dragger = static_cast<const DraggerView*>(view);
            float val = dragger->value ? *dragger->value : 0;
            
            // Background
            dl.add_rounded_rect({rect.x, rect.y}, {rect.w, rect.h}, 4.0f, {0.2f, 0.2f, 0.25f, 1.0f});
            if (interaction::IsActive(dragger->id)) {
                 dl.add_rounded_rect({rect.x, rect.y}, {rect.w, rect.h}, 4.0f, {0.3f, 0.3f, 0.35f, 1.0f}); // Active highlight
            }
            
            // Text Value
            char buf[64];
            snprintf(buf, 64, "%.2f", val);
            
             float scale = 14.0f / (float)internal::FontManager::BASE_SDF_SIZE;
             float lx = rect.x + 8; float ly = rect.y + (rect.h + 14.0f * 0.6f) * 0.5f;
             for (const char* p = buf; *p; ) {
                 uint32_t cp = internal::NextUTF8(p);
                 auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                 float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                 if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                     dl.add_text({lx + ox * scale, ly - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {1,1,1,1});
                     lx += adv * scale;
                 }
             }
        }
        
        else if (view->type == ViewType::Collapsible) {
            const auto* col = static_cast<const CollapsibleView*>(view);
            bool expanded = col->expanded ? *col->expanded : false;
            
            // Header
            internal::Vec2 header_pos = {rect.x, rect.y};
            internal::Vec2 header_size = {rect.w, col->header_height};
            
            dl.add_rounded_rect(header_pos, header_size, 4.0f, {0.25f, 0.25f, 0.3f, 1.0f});
            
            // Icon (Triangle)
            float icon_size = 10.0f;
            float icon_x = rect.x + 8;
            float icon_y = rect.y + (col->header_height - icon_size) * 0.5f;
            
            internal::Path path;
            path.is_fill = true;
            path.color = {0.8f, 0.8f, 0.8f, 1.0f};
            if (expanded) {
                // Down pointing
                path.points.push_back({internal::PathVerb::MoveTo, {icon_x, icon_y + icon_size*0.25f}});
                path.points.push_back({internal::PathVerb::LineTo, {icon_x + icon_size, icon_y + icon_size*0.25f}});
                path.points.push_back({internal::PathVerb::LineTo, {icon_x + icon_size*0.5f, icon_y + icon_size*0.75f}});
            } else {
                // Right pointing
                path.points.push_back({internal::PathVerb::MoveTo, {icon_x + icon_size*0.25f, icon_y}});
                path.points.push_back({internal::PathVerb::LineTo, {icon_x + icon_size*0.75f, icon_y + icon_size*0.5f}});
                path.points.push_back({internal::PathVerb::LineTo, {icon_x + icon_size*0.25f, icon_y + icon_size}});
            }
            path.points.push_back({internal::PathVerb::Close});
            dl.add_path(path); // Requires add_path in DrawList
            
            if (col->label) {
                 float scale = 14.0f / (float)internal::FontManager::BASE_SDF_SIZE;
                 float lx = icon_x + icon_size + 8; float ly = rect.y + (col->header_height + 14.0f * 0.6f) * 0.5f;
                 for (const char* p = col->label; *p; ) {
                     uint32_t cp = internal::NextUTF8(p);
                     auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                     float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                     if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                         dl.add_text({lx + ox * scale, ly - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {1,1,1,1});
                         lx += adv * scale;
                     }
                 }
            }
        }
        
        else if (view->type == ViewType::Toast) {
             const auto* toast = static_cast<const ToastView*>(view);
             // Render floating pill
             dl.add_rounded_rect({rect.x, rect.y}, {rect.w, rect.h}, 20.0f, {0.1f, 0.1f, 0.1f, 0.9f}, 4.0f);
             if (toast->message) {
                 // Text rendering...
                 float scale = 14.0f / (float)internal::FontManager::BASE_SDF_SIZE;
                 float lx = rect.x + 16; float ly = rect.y + (rect.h + 14.0f * 0.6f) * 0.5f;
                 for (const char* p = toast->message; *p; ) {
                     uint32_t cp = internal::NextUTF8(p);
                     auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                     float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                     if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                         dl.add_text({lx + ox * scale, ly - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {1,1,1,1});
                         lx += adv * scale;
                     }
                 }
             }
        }
        
        else if (view->type == ViewType::Tooltip) {
             const auto* tt = static_cast<const TooltipView*>(view);
             dl.add_rounded_rect({rect.x, rect.y}, {rect.w, rect.h}, 4.0f, {0,0,0,0.8f});
             // Text...
             if (tt->text) {
                 float scale = 12.0f / (float)internal::FontManager::BASE_SDF_SIZE;
                 float lx = rect.x + 4; float ly = rect.y + (rect.h + 12.0f * 0.6f) * 0.5f;
                 for (const char* p = tt->text; *p; ) {
                     uint32_t cp = internal::NextUTF8(p);
                     auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                     float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                     if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                         dl.add_text({lx + ox * scale, ly - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {1,1,1,1});
                         lx += adv * scale;
                     }
                 }
             }
        }
        
        else if (view->type == ViewType::Node) {
            const auto* node = static_cast<const NodeView*>(view);
            // Main Box
            dl.add_rounded_rect({rect.x, rect.y}, {rect.w, rect.h}, 8.0f, {0.18f, 0.18f, 0.2f, 0.95f}, 8.0f); // Shadow
            
            // Selection Border
            if (interaction::IsActive(node->id) || node->selected) {
                 dl.add_rounded_rect({rect.x-1, rect.y-1}, {rect.w+2, rect.h+2}, 9.0f, {0.9f, 0.6f, 0.2f, 0.5f});
            }

            // Title Header
            float header_h = 24.0f;
            dl.add_rounded_rect({rect.x, rect.y}, {rect.w, header_h}, 8.0f, {0.25f, 0.25f, 0.28f, 1.0f}); // Background
            // Flatten bottom corners of header? Requires path or clip. For now just draw over body.
            
            // Title Text
            if (node->title) {
                 float scale = 14.0f / (float)internal::FontManager::BASE_SDF_SIZE;
                 float lx = rect.x + 8; float ly = rect.y + (header_h + 14.0f * 0.6f) * 0.5f;
                 for (const char* p = node->title; *p; ) {
                     uint32_t cp = internal::NextUTF8(p);
                     auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                     float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                     if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                         dl.add_text({lx + ox * scale, ly - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {1,1,1,1});
                         lx += adv * scale;
                     }
                 }
            }
        }
        
        else if (view->type == ViewType::Socket) {
             const auto* sock = static_cast<const SocketView*>(view);
             
             float r = 6.0f;
             float cy = rect.y + rect.h * 0.5f;
             float cx = sock->is_input ? rect.x + r + 4 : rect.x + rect.w - r - 4;
             
             dl.add_circle({cx, cy}, r, sock->color);
             dl.add_circle({cx, cy}, r * 0.6f, {0.1f, 0.1f, 0.1f, 1.0f}); // Hole
             
             // Label
             if (sock->name) {
                 float scale = 12.0f / (float)internal::FontManager::BASE_SDF_SIZE;
                 float lx = sock->is_input ? cx + r + 6 : rect.x + rect.w - r - 6 - (strlen(sock->name)*7.0f); // Approx right align
                 // Simple left align for input, rightish for output?
                 if (!sock->is_input) lx = rect.x + rect.w - r - 8 - 40; // Hack offset for now
                 
                 float ly = rect.y + (rect.h + 12.0f * 0.6f) * 0.5f;
                 // Actually standard loop...
                 lx = sock->is_input ? cx + r + 8 : rect.x + rect.w - r - 8 - 50; // Just push it left
                 
                  for (const char* p = sock->name; *p; ) {
                     uint32_t cp = internal::NextUTF8(p);
                     auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                     float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                     if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                         dl.add_text({lx + ox * scale, ly - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {0.8f,0.8f,0.8f,1});
                         lx += adv * scale;
                     }
                 }
             }
        }

        else if (view->type == ViewType::Markdown) {
            const auto* md = static_cast<const MarkdownView*>(view);
            if (md->source) {
                // Parse markdown
                markdown::MarkdownParser parser;
                markdown::MarkdownDocument doc = parser.parse(md->source);
                
                float pen_y = rect.y + 4;
                float line_h = 16.0f * md->line_spacing;
                
                for (const auto& block : doc.blocks) {
                    if (block.type == markdown::BlockType::Empty) {
                        pen_y += line_h * 0.5f;
                        continue;
                    }
                    
                    if (block.type == markdown::BlockType::HorizontalRule) {
                        dl.add_rounded_rect({rect.x, pen_y + 4}, {rect.w - 20, 2.0f}, 1.0f, {0.5f, 0.5f, 0.5f, 0.5f});
                        pen_y += line_h;
                        continue;
                    }
                    
                    // Determine font size and color
                    float font_size = markdown::get_font_size(block.type);
                    internal::ColorF text_col = markdown::is_heading(block.type) ? md->heading_color : md->text_color;
                    float scale = font_size / (float)internal::FontManager::BASE_SDF_SIZE;
                    
                    // Calculate indent
                    float indent = block.indent_level * 20.0f;
                    if (block.type == markdown::BlockType::BulletList || block.type == markdown::BlockType::NumberedList) {
                        indent += 15.0f;
                    }
                    
                    // Draw bullet point
                    if (block.type == markdown::BlockType::BulletList) {
                        dl.add_circle({rect.x + indent - 10, pen_y + font_size * 0.5f}, 3.0f, md->text_color);
                    }
                    
                    // Code block background
                    if (block.type == markdown::BlockType::CodeBlock) {
                        // Simple estimate for code block height
                        int lines = 1;
                        for (const auto& span : block.spans) {
                            for (char c : span.text) if (c == '\n') lines++;
                        }
                        float code_h = lines * (font_size * 1.2f) + 16;
                        dl.add_rounded_rect({rect.x + 4, pen_y}, {rect.w - 28, code_h}, 4.0f, md->code_bg);
                        indent += 10;
                    }
                    
                    // Draw spans
                    float pen_x = rect.x + indent;
                    float baseline_y = pen_y + font_size * 0.8f;
                    
                    for (const auto& span : block.spans) {
                        internal::ColorF span_col = text_col;
                        if ((static_cast<uint8_t>(span.style) & static_cast<uint8_t>(markdown::InlineStyle::Link)) != 0) {
                            span_col = md->link_color;
                        }
                        if ((static_cast<uint8_t>(span.style) & static_cast<uint8_t>(markdown::InlineStyle::Code)) != 0) {
                            // Draw inline code bg
                            // Approximate width... simple hack
                            float approx_w = span.text.size() * font_size * 0.6f;
                            dl.add_rounded_rect({pen_x - 2, pen_y}, {approx_w + 4, font_size + 4}, 3.0f, md->code_bg);
                        }
                        
                        const char* p = span.text.c_str();
                        while (*p) {
                            // Handle newlines in code blocks
                            if (*p == '\n') {
                                p++;
                                pen_x = rect.x + indent;
                                baseline_y += font_size * 1.2f;
                                continue;
                            }
                            
                            uint32_t cp = internal::NextUTF8(p);
                            auto [f_id, g_idx] = internal::FontManager::Get().get_glyph_index(cp);
                            float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                            if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                                dl.add_text({pen_x + ox * scale, baseline_y - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, span_col);
                                pen_x += adv * scale;
                            }
                        }
                    }
                    
                    pen_y += line_h;
                    if (markdown::is_heading(block.type)) pen_y += font_size * 0.3f; // Extra spacing after headings
                    if (block.type == markdown::BlockType::CodeBlock) pen_y += 20; // Extra padding
                }
            }
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
                        float u0, v0, u1, v1, gw, gh, adv, ox, oy;
                        if (internal::GlyphCache::Get().get_glyph_uv(f_id, g_idx, u0, v0, u1, v1, gw, gh, adv, ox, oy)) {
                             dl.add_text({pen_x + ox * scale, pen_y - oy * scale}, {gw * scale, gh * scale}, {u0, v0, u1, v1}, {0, 1, 0, 1});
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
