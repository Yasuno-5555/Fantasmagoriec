#pragma once
#include "core/font_types.hpp"
#include <string>
#include <vector>
#include <cstdint>

// Forward declarations to avoid exposing FreeType headers globally
struct FT_LibraryRec_;
struct FT_FaceRec_;
using FT_Library = struct FT_LibraryRec_*;
using FT_Face = struct FT_FaceRec_*;

namespace fanta {
namespace internal {

    // Singleton Font Manager wrapping FreeType
    class FontManager {
    public:
        static FontManager& Get();
        static const int BASE_SDF_SIZE = 32;
        
        bool init();
        void shutdown();
        
        // Load a font from storage. Returns simple ID (index).
        // For Phase 4, we might just load one default font.
        FontID load_font(const std::string& path);
        
        // Get glyph metric (advance, bearing) scaled to specific size?
        // No, SDF approach usually gets metrics at "Master" size (e.g. 64px)
        // and scales them.
        const GlyphMetrics& get_metrics(FontID font, uint32_t glyph_index);
        
        // Generate SDF bitmap for a glyph
        // w, h are output dimensions. buffer is 8-bit alpha.
        // out_off_x, out_off_y are bitmap placement offsets (bearing).
        bool generate_sdf(FontID font, uint32_t glyph_index, int sdf_size, 
                          std::vector<uint8_t>& out_buffer, int& out_w, int& out_h,
                          int& out_off_x, int& out_off_y);

        FT_Face get_face(FontID font);

        // L-1: Glyphs by Codepoint with Fallback
        // Returns {font_id, glyph_index}
        // If not found in primary font, checks fallback stack.
        std::pair<FontID, uint32_t> get_glyph_index(uint32_t codepoint);
        
        void add_fallback_font(FontID font) { font_fallback_stack.push_back(font); }

    private:
        FontManager() = default;
        ~FontManager() = default;

        FT_Library library = nullptr;
        std::vector<FT_Face> faces;
        std::vector<FontID> font_fallback_stack;
        std::vector<std::vector<GlyphMetrics>> metrics_cache;
    };

} // namespace internal
} // namespace fanta
