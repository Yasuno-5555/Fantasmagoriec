#include "text/font_manager.hpp"
#include <iostream>
// FreeType Headers
#include <ft2build.h>
#include FT_FREETYPE_H

namespace fanta {
namespace internal {

    FontManager& FontManager::Get() {
        static FontManager instance;
        return instance;
    }

    bool FontManager::init() {
        if (FT_Init_FreeType(&library)) {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return false;
        }
        return true;
    }

    void FontManager::shutdown() {
        for (auto face : faces) {
            FT_Done_Face(face);
        }
        faces.clear();
        FT_Done_FreeType(library);
    }

    FontID FontManager::load_font(const std::string& path) {
        FT_Face face;
        if (FT_New_Face(library, path.c_str(), 0, &face)) {
            std::cerr << "ERROR::FREETYPE: Failed to load font: " << path << std::endl;
            return 0; // Invalid
        }
        
        // Phase 4: Store face and return index
        FontID id = static_cast<FontID>(faces.size());
        faces.push_back(face);
        return id;
    }

    FT_Face FontManager::get_face(FontID font) {
        if (font >= faces.size()) return nullptr;
        return faces[font];
    }

    const GlyphMetrics& FontManager::get_metrics(FontID font, uint32_t glyph_index) {
        static GlyphMetrics empty_metrics;
        
        // Ensure cache size
        if (font >= metrics_cache.size()) {
            metrics_cache.resize(font + 1);
        }
        
        // For now we don't actually cache in metrics_cache vector to keep it simple,
        // but the signature must be correct.
        
        FT_Face face = get_face(font);
        if (!face) return empty_metrics;

        // Set size to Base SDF Size for consistent scaling
        FT_Set_Pixel_Sizes(face, 0, BASE_SDF_SIZE);

        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
            return empty_metrics;
        }

        // Return static temp for now (Phase 4), or implement cache
        static GlyphMetrics temp;
        // Metrics are 26.6 fixed point (1/64 pixel)
        temp.advance = static_cast<float>(face->glyph->advance.x >> 6);
        temp.width = static_cast<float>(face->glyph->metrics.width >> 6);
        temp.height = static_cast<float>(face->glyph->metrics.height >> 6);
        temp.bearing_x = static_cast<float>(face->glyph->metrics.horiBearingX >> 6);
        temp.bearing_y = static_cast<float>(face->glyph->metrics.horiBearingY >> 6);
        
        return temp;
    }

    bool FontManager::generate_sdf(FontID font, uint32_t glyph_index, int sdf_size, 
                                   std::vector<uint8_t>& out_buffer, int& out_w, int& out_h) {
        FT_Face face = get_face(font);
        if (!face) return false;

        // Set size for SDF generation
        // Note: SDF generation works best at a decent resolution (e.g. 32-64px)
        FT_Set_Pixel_Sizes(face, 0, sdf_size);

        // Load Glyph
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) { // FT_LOAD_NO_BITMAP?
            return false;
        }

        // Render SDF
        // Note: FT_RENDER_MODE_SDF is available in FreeType 2.11+
        // If compilation fails here, we know the environment is too old.
        // But FetchContent master should be fine.
        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF)) {
            std::cerr << "Failed to render SDF for char " << codepoint << std::endl;
            return false;
        }

        // Output
        FT_Bitmap& bitmap = face->glyph->bitmap;
        out_w = bitmap.width;
        out_h = bitmap.rows;
        
        out_buffer.resize(out_w * out_h);
        
        // Copy data (SDF is 8-bit generic)
        if (bitmap.buffer) {
            std::copy(bitmap.buffer, bitmap.buffer + (out_w * out_h), out_buffer.begin());
        } else {
            // Empty glyph (space), create empty buffer
            std::fill(out_buffer.begin(), out_buffer.end(), 0);
        }

        return true;
    }

    FontID FontManager::find_font_for_char(uint32_t codepoint) {
        // 1. Check fallback stack first if populated
        for (FontID id : font_fallback_stack) {
            FT_Face face = get_face(id);
            if (face && FT_Get_Char_Index(face, codepoint) != 0) return id;
        }
        // 2. Check all loaded fonts as a final safety
        for (size_t i = 0; i < faces.size(); ++i) {
            if (FT_Get_Char_Index(faces[i], codepoint) != 0) return static_cast<FontID>(i);
        }
        return 0; // Default font
    }

} // namespace internal
} // namespace fanta
