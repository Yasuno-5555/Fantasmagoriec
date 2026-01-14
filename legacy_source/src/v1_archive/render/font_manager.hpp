#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "../platform/gl_lite.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

struct Glyph {
    float u0, v0, u1, v1; // Texture coordinates
    float width, height;  // Size in pixels
    float bearing_x, bearing_y;
    float advance;
};

class FontAtlas {
public:
    GLuint texture_id = 0;
    int width = 512;
    int height = 512;
    int cursor_x = 0;
    int cursor_y = 0;
    int row_height = 0;
    
    ~FontAtlas();
    void init();
    
    // Returns UV rect {u0, v0, u1, v1}
    // and writes glyph data to texture
    // Returns false if full
    bool add_glyph(int w, int h, const unsigned char* buffer, float out_uvs[4]);
};

class Font {
public:
    FT_Face face = nullptr;
    FontAtlas* atlas = nullptr;
    std::map<uint32_t, Glyph> glyphs;
    int size = 16;
    
    bool load(const std::string& path, int size, FT_Library ft, FontAtlas* atlas);
    Glyph& get_glyph(uint32_t codepoint);
};
