// Fantasmagorie v3 - Typography Core
// Basic types for text rendering
#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace fanta {

// ============================================================================
// Glyph Metrics & Stats
// ============================================================================

struct TypographyStats {
    int total_glyphs = 0;
    int atlas_pages = 0;
    float atlas_utilization = 0;
    int lru_hits = 0;
    int lru_misses = 0;
    int active_buffer_idx = 0;
    bool harfbuzz_enabled = false;
};

struct FantaGlyph {
    uint32_t codepoint = 0;
    
    // Metrics in logical pixels
    Vec2 size;          // Width, Height of glyph bitmap
    Vec2 bearing;       // Offset from baseline to top-left of glyph
    float advance = 0;  // Horizontal advance to next glyph
    
    // Atlas coordinates (0-1)
    Vec2 uv_min;
    Vec2 uv_max;
    
    bool valid = false;
};

// ============================================================================
// Font Metrics
// ============================================================================
struct FontMetrics {
    float ascent = 0;       // Distance from baseline to top
    float descent = 0;      // Distance from baseline to bottom (usually negative)
    float line_gap = 0;     // Recommended extra space between lines
    float line_height = 0;  // ascent - descent + line_gap
    float units_per_em = 0;
};

// ============================================================================
// Text Selection & Shaping (Simplified)
// ============================================================================
struct TextRun {
    std::string text;
    uint32_t font_id = 0;
    float font_size = 14.0f;
    Color color = Color::White();
    
    // TODO: Shaping results (HarfBuzz) would go here
};

} // namespace fanta
