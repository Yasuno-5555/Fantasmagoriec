#pragma once
#include "core/types_internal.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace fanta {
namespace internal {

    using FontID = uint32_t;
    using GlyphID = uint32_t; // FreeType index

    struct GlyphMetrics {
        float advance = 0;
        float bearing_x = 0;
        float bearing_y = 0;
        float width = 0;
        float height = 0;
    };

    struct GlyphQuad {
        // Position relative to text origin
        // We use typical quad coordinates: 
        // x0,y0 = Top-Left, x1,y1 = Bottom-Right
        float x0, y0, x1, y1;
        
        // UV in the global Atlas
        float u0, v0, u1, v1;
    };

    struct TextRun {
        FontID font_id = 0;
        float font_size = 0;
        std::vector<GlyphQuad> quads;
        Rectangle bounds = {0,0,0,0}; // Total bounding box of the run
        
        // Phase 4.5: Native Preview Support
        std::string original_text;
        
        // Helper to check validity
        bool empty() const { return quads.empty() && original_text.empty(); }
    };

} // namespace internal
} // namespace fanta
