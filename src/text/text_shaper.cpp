#include "text_shaper.hpp"
#include "text/font_manager.hpp"
#include <hb.h>
#include <hb-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>

namespace fanta {
namespace internal {

    std::vector<ShapedGlyph> TextShaper::shape(const std::string& text, FontID font) {
        std::vector<ShapedGlyph> result;
        if (text.empty()) return result;

        FontManager& fm = FontManager::Get();
        FT_Face face = fm.get_face(font);
        if (!face) return result;

        // Note: HarfBuzz needs fonts to be set to a specific pixel size for metrics
        // FontManager::get_metrics already sets BASE_SDF_SIZE, let's stick to that for shaping.
        FT_Set_Pixel_Sizes(face, 0, FontManager::BASE_SDF_SIZE);

        hb_font_t* hb_font = hb_ft_font_create_referenced(face);
        hb_buffer_t* buf = hb_buffer_create();
        
        hb_buffer_add_utf8(buf, text.data(), (int)text.size(), 0, -1);
        hb_buffer_guess_segment_properties(buf);

        hb_shape(hb_font, buf, NULL, 0);

        unsigned int glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

        result.reserve(glyph_count);
        for (unsigned int i = 0; i < glyph_count; ++i) {
            ShapedGlyph g;
            g.glyph_index = glyph_info[i].codepoint;
            g.cluster = glyph_info[i].cluster;
            
            // metrics are 26.6 format
            g.x_advance = (float)glyph_pos[i].x_advance / 64.0f;
            g.y_advance = (float)glyph_pos[i].y_advance / 64.0f;
            g.x_offset = (float)glyph_pos[i].x_offset / 64.0f;
            g.y_offset = (float)glyph_pos[i].y_offset / 64.0f;
            
            result.push_back(g);
        }

        hb_buffer_destroy(buf);
        hb_font_destroy(hb_font);

        return result;
    }

} // namespace internal
} // namespace fanta
