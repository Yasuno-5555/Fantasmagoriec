#include "text/text_layout.hpp"
#include "text/glyph_cache.hpp" // Changed from font_manager.hpp
#include "core/types_internal.hpp"

namespace fanta {
namespace internal {

    TextRun TextLayout::transform(const std::string& text, FontID font_id, float font_size) {
        TextRun run;
        run.font_id = font_id;
        run.font_size = font_size;
        run.original_text = text;
        
        float pen_x = 0;
        float pen_y = 0; // Baseline
        
        // Access GlyphCache for UVs and approximate metrics
        auto& cache = GlyphCache::Get();
        
        run.bounds.x = 0;
        run.bounds.y = -font_size * 0.8f;  // Actual quad top position (above baseline)
        run.bounds.h = font_size; 
        
        // Loop chars
        for (char c : text) {
            float u0, v0, u1, v1, aw, ah;
            // Get UVs from cache (generates on fly if needed)
            if (cache.get_glyph_uv(font_id, c, u0, v0, u1, v1, aw, ah)) {
                // Approximate metrics for Phase 4
                float advance = font_size * 0.6f; 
                
                GlyphQuad q;
                q.x0 = pen_x;
                q.y0 = pen_y - font_size * 0.8f; 
                q.x1 = pen_x + font_size * 0.6f;
                q.y1 = pen_y + font_size * 0.2f; 
                q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
                
                run.quads.push_back(q);
                pen_x += advance;
            }
        }
        run.bounds.w = pen_x;
        return run;
    }

    Vec2 TextLayout::measure(const std::string& text, FontID font_id, float font_size, float max_width, float line_height) {
        // Reuse transform
        TextRun run = transform(text, font_id, font_size);
        return {run.bounds.w, run.bounds.h};
    }

} // namespace internal
} // namespace fanta
