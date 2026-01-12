#include "font_manager.hpp"
#include <iostream>

// ========================
// FontAtlas
// ========================

FontAtlas::~FontAtlas() {
    if (texture_id) glDeleteTextures(1, &texture_id);
}

void FontAtlas::init() {
    // Create single channel texture
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // Allocate empty storage
    // Use GL_RED for single channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    
    // Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Pixel alignment 1 for R8
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

bool FontAtlas::add_glyph(int w, int h, const unsigned char* buffer, float out_uvs[4]) {
    // Simple Shelf Packing
    if (cursor_x + w > width) {
        // New line
        cursor_x = 0;
        cursor_y += row_height + 1; // +1 padding
        row_height = 0;
    }
    
    if (cursor_y + h > height) {
        // Full!
        printf("FontAtlas Full!\n");
        return false;
    }
    
    // Upload
    if (w > 0 && h > 0) {
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, cursor_x, cursor_y, w, h, GL_RED, GL_UNSIGNED_BYTE, buffer);
    }
    
    // UVs
    out_uvs[0] = (float)cursor_x / width;
    out_uvs[1] = (float)cursor_y / height;
    out_uvs[2] = (float)(cursor_x + w) / width;
    out_uvs[3] = (float)(cursor_y + h) / height;
    
    // Advance
    cursor_x += w + 1; // +1 padding
    if (h > row_height) row_height = h;
    
    return true;
}

// ========================
// Font
// ========================

bool Font::load(const std::string& path, int size, FT_Library ft, FontAtlas* atlas) {
    this->size = size;
    this->atlas = atlas;
    
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        printf("Failed to load font: %s\n", path.c_str());
        return false;
    }
    
    FT_Set_Pixel_Sizes(face, 0, size);
    return true;
}

Glyph& Font::get_glyph(uint32_t codepoint) {
    // Check cache
    auto it = glyphs.find(codepoint);
    if (it != glyphs.end()) return it->second;
    
    // Load
    if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER)) {
        // Failed, return empty
        static Glyph empty;
        return empty;
    }
    
    // Add to Atlas
    float uvs[4];
    atlas->add_glyph(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer, uvs);
    
    // Store
    Glyph g;
    g.u0 = uvs[0]; g.v0 = uvs[1];
    g.u1 = uvs[2]; g.v1 = uvs[3];
    g.width = (float)face->glyph->bitmap.width;
    g.height = (float)face->glyph->bitmap.rows;
    g.bearing_x = (float)face->glyph->bitmap_left;
    g.bearing_y = (float)face->glyph->bitmap_top;
    g.advance = (float)(face->glyph->advance.x >> 6);
    
    glyphs[codepoint] = g;
    return glyphs[codepoint];
}
