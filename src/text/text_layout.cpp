#include "text/text_layout.hpp"
#include "text/glyph_cache.hpp" 
#include "text/text_shaper.hpp"
#include "core/types_internal.hpp"

namespace fanta {
namespace internal {

    TextRun TextLayout::transform(const std::string& text, FontID font_id, const TypographyRule& rule) {
        return transform_wrapped(text, font_id, rule, 0);
    }

    TextRun TextLayout::transform_wrapped(const std::string& text, FontID font_id, const TypographyRule& rule, float max_width) {
        TextRun run;
        run.font_id = font_id;
        run.font_size = rule.size;
        run.original_text = text;
        
        // Phase 26: Shape with HarfBuzz
        std::vector<ShapedGlyph> shaped = TextShaper::shape(text, font_id);
        
        float pen_x = 0;
        float pen_y = rule.size * 0.8f; 
        
        auto& cache = GlyphCache::Get();
        
        run.bounds.x = 0;
        run.bounds.y = 0; 
        run.bounds.h = rule.size; 
        
        bool any_glyph_found = false;
        for (const auto& sg : shaped) {
            float u0, v0, u1, v1, aw, ah, adv;
            // GlyphCache now takes glyph_index
            bool found = cache.get_glyph_uv(font_id, sg.glyph_index, u0, v0, u1, v1, aw, ah, adv);
            if (found) {
                any_glyph_found = true;
                float scale = font_size / 32.0f;
                
                // Use HarfBuzz offsets and advances
                float x_offset = sg.x_offset * scale;
                float y_offset = sg.y_offset * scale;
                // Apply Tracking (Letter Spacing)
                // Tracking is usually in ems or points. rule.tracking is in points (approx).
                // Let's assume tracking is added to advance. 
                // Apple tracking is 1/1000 em? Or points? 
                // User said "tracking -0.5pt". 
                // If font size is 28, -0.5pt is relatively small.
                float track_px = rule.tracking * (scale * 32.0f / rule.size); // Dummy conversion or just raw px
                // Actually if user inputs -0.5f in rule as 'points', we directly add it if scale logic matches.
                // Converting: logical px.
                track_px = rule.tracking;
                
                float x_advance = sg.x_advance * scale + track_px;
                
                if (max_width > 0 && pen_x + x_advance > max_width && pen_x > 0) {
                    pen_x = 0;
                    // Apply Line Height (multiplier * size)
                    float lh_px = rule.line_height * rule.size;
                    pen_y += lh_px;
                }

                GlyphQuad q;
                q.x0 = pen_x + x_offset;
                q.y0 = pen_y - rule.size * 0.8f + y_offset; 
                q.x1 = q.x0 + aw * scale;
                q.y1 = q.y0 + ah * scale;
                q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
                
                run.quads.push_back(q);
                pen_x += x_advance;
            }
        }
        
        // Final bounds calculation
        float max_pen_x = 0;
        if (any_glyph_found) {
            if (pen_y > font_size * 0.8f) { // Multiline check
                max_pen_x = max_width > 0 ? max_width : pen_x;
            } else {
                max_pen_x = pen_x;
            }
        }
        
        run.bounds.w = max_pen_x;
        // Height is baseline of last line + descent (approx 0.2*fs)
        run.bounds.h = any_glyph_found ? (pen_y + font_size * 0.2f) : 0;
        
        return run;
    }

    Vec2 TextLayout::measure(const std::string& text, FontID font_id, const TypographyRule& rule, float max_width) {
        // Reuse transform_wrapped
        TextRun run = transform_wrapped(text, font_id, rule, max_width);
        return {run.bounds.w, run.bounds.h};
    }

} // namespace internal
} // namespace fanta
