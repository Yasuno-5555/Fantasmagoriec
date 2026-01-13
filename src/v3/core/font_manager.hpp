#include "glyph_atlas.hpp"
#include "text.hpp"
#include "types.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_TRUETYPE_TABLES_H
#include FT_OUTLINE_H
#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>

namespace fanta {

class FontManager {
public:
    static FontManager& instance() {
        static FontManager inst;
        return inst;
    }

    bool init() {
        if (FT_Init_FreeType(&ft_)) {
            std::cerr << "[FontManager] Failed to init FreeType" << std::endl;
            return false;
        }
        initialized_ = true;
        return true;
    }

    void shutdown() {
        for (auto& pair : faces_) {
            FT_Done_Face(pair.second);
        }
        faces_.clear();

        if (ft_) {
            FT_Done_FreeType(ft_);
            ft_ = nullptr;
        }
        initialized_ = false;
    }

    // Load font from file
    uint32_t load_font(const std::string& path, const std::string& name) {
        if (!initialized_) {
            std::cerr << "[FontManager] Error: Not initialized before load_font call!" << std::endl;
            return 0;
        }

        FT_Face face;
        if (FT_New_Face(ft_, path.c_str(), 0, &face)) {
            std::cerr << "[FontManager] Failed to load font: " << path << std::endl;
            return 0;
        }

        uint32_t id = next_font_id_++;
        fonts_[id] = name;
        faces_[id] = face;
        
        std::cout << "[FontManager] Loaded font '" << name << "' from " << path << std::endl;
        return id;
    }

    // SDF Glyph Data
    struct GlyphData {
        FantaGlyph glyph_metrics;
        std::vector<uint8_t> pixels;
    };

    // Get or generate glyph
    GlyphData get_glyph(uint32_t font_id, uint32_t codepoint, float points, float dpr) {
        auto it = faces_.find(font_id);
        if (it == faces_.end()) return {};

        FT_Face face = it->second;
        
        // Use a fixed large size for SDF generation to maintain quality
        // SDF is scale-independent, but base resolution matters for sharp features.
        const float SDF_BASE_SIZE = 64.0f;
        FT_Set_Pixel_Sizes(face, 0, (FT_UInt)(SDF_BASE_SIZE * dpr));

        // FT_RENDER_MODE_SDF is available in modern FreeType
        auto err = FT_Load_Char(face, codepoint, FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
        if (err) return {};

        FT_Bitmap& bitmap = face->glyph->bitmap;

        // std::cout << "[FontManager] Loaded glyph " << (char)codepoint << " " << bitmap.width << "x" << bitmap.rows << std::endl;
        
        GlyphData data;

        data.glyph_metrics.codepoint = codepoint;
        data.glyph_metrics.size = { (float)bitmap.width / dpr, (float)bitmap.rows / dpr };
        data.glyph_metrics.bearing = { (float)face->glyph->bitmap_left / dpr, (float)face->glyph->bitmap_top / dpr };
        data.glyph_metrics.advance = (float)face->glyph->advance.x / 64.0f / dpr;
        data.glyph_metrics.valid = true;

        if (bitmap.buffer) {
            data.pixels.assign(bitmap.buffer, bitmap.buffer + (bitmap.width * bitmap.rows));
        }

        return data;
    }

    // ============================================================================
    // Typography Shaping (Phase 16)
    // ============================================================================

    struct ShapedGlyph {
        uint32_t codepoint;
        Vec2 pos;
        float advance;
    };

    struct ShapeResult {
        std::vector<ShapedGlyph> glyphs;
        float total_advance = 0;
    };

    // SimpleShaper: Basic horizontal layout if HarfBuzz is off
    ShapeResult shape_simple(uint32_t font_id, const std::string& text, float size, float dpr) {
        auto it = faces_.find(font_id);
        if (it == faces_.end()) return {};

        FT_Face face = it->second;
        const float SDF_BASE_SIZE = 64.0f;
        FT_Set_Pixel_Sizes(face, 0, (FT_UInt)(SDF_BASE_SIZE * dpr));
        
        float scale = size / SDF_BASE_SIZE / dpr;
        ShapeResult res;
        float pen_x = 0;

        // Proper UTF-8 decoding
        size_t i = 0;
        while (i < text.size()) {
            uint32_t cp = 0;
            unsigned char c = text[i];
            
            if ((c & 0x80) == 0) {
                // 1-byte ASCII
                cp = c;
                i += 1;
            } else if ((c & 0xE0) == 0xC0) {
                // 2-byte sequence
                cp = (c & 0x1F) << 6;
                if (i + 1 < text.size()) cp |= (text[i + 1] & 0x3F);
                i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                // 3-byte sequence (Japanese, Chinese, etc.)
                cp = (c & 0x0F) << 12;
                if (i + 1 < text.size()) cp |= (text[i + 1] & 0x3F) << 6;
                if (i + 2 < text.size()) cp |= (text[i + 2] & 0x3F);
                i += 3;
            } else if ((c & 0xF8) == 0xF0) {
                // 4-byte sequence (emoji, etc.)
                cp = (c & 0x07) << 18;
                if (i + 1 < text.size()) cp |= (text[i + 1] & 0x3F) << 12;
                if (i + 2 < text.size()) cp |= (text[i + 2] & 0x3F) << 6;
                if (i + 3 < text.size()) cp |= (text[i + 3] & 0x3F);
                i += 4;
            } else {
                // Invalid, skip
                i += 1;
                continue;
            }

            if (FT_Load_Char(face, cp, FT_LOAD_DEFAULT)) continue;

            ShapedGlyph g;
            g.codepoint = cp;
            g.pos = { pen_x, 0 };
            g.advance = (float)face->glyph->advance.x / 64.0f * scale;

            res.glyphs.push_back(g);
            pen_x += g.advance;
        }
        res.total_advance = pen_x;
        return res;
    }

    FontMetrics get_metrics(uint32_t font_id, float points, float dpr) {
        auto it = faces_.find(font_id);
        if (it == faces_.end()) return {};

        FT_Face face = it->second;
        FT_Set_Pixel_Sizes(face, 0, (FT_UInt)(points * dpr));

        FontMetrics m;
        m.ascent = (float)face->size->metrics.ascender / 64.0f / dpr;
        m.descent = (float)face->size->metrics.descender / 64.0f / dpr;
        m.line_gap = (float)(face->size->metrics.height - (face->size->metrics.ascender - face->size->metrics.descender)) / 64.0f / dpr;
        m.line_height = (float)face->size->metrics.height / 64.0f / dpr;
        m.units_per_em = (float)face->units_per_EM;

        return m;
    }

private:
    FontManager() = default;
    ~FontManager() { shutdown(); }

    FT_Library ft_ = nullptr;
    bool initialized_ = false;
    uint32_t next_font_id_ = 1;
    
    std::unordered_map<uint32_t, std::string> fonts_;
    std::unordered_map<uint32_t, FT_Face> faces_;
};

} // namespace fanta
