#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Must be before stb_truetype to avoid macro conflicts

#include "font.hpp"
#include <cstdio>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>


Font::~Font() {
    if (texture_id) {
        // glDeleteTextures(1, &texture_id); // Need to expose this in gl_lite if not present
    }
    if (cdata) {
        delete[] (stbtt_bakedchar*)cdata;
    }
}

bool Font::load(const std::string& filename, float size) {
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    std::vector<unsigned char> ttf_buffer(len);
    fread(ttf_buffer.data(), 1, len, f);
    fclose(f);
    
    tex_width = 512;
    tex_height = 512;
    std::vector<unsigned char> temp_bitmap(tex_width * tex_height);
    
    cdata = new stbtt_bakedchar[96];
    
    // Bake ASCII 32..126
    int res = stbtt_BakeFontBitmap(ttf_buffer.data(), 0, size, temp_bitmap.data(), tex_width, tex_height, 32, 96, (stbtt_bakedchar*)cdata);
    
    if (res <= 0) {
        // printf("Failed to bake font, texture too small?\n");
        // Try larger?
    }
    
    height = size;
    
    // Upload to GL
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // 1 byte per pixel (alpha) -> GL_RED
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return true; 
}


float Font::get_quad(int char_index, float* x, float* y, Quad& q) {
    if (char_index < 32 || char_index >= 128) return 0;
    
    stbtt_aligned_quad quad;
    stbtt_GetBakedQuad((stbtt_bakedchar*)cdata, tex_width, tex_height, char_index - 32, x, y, &quad, 1);
    
    q.x0 = quad.x0;
    q.y0 = quad.y0;
    q.x1 = quad.x1;
    q.y1 = quad.y1;
    
    q.s0 = quad.s0;
    q.t0 = quad.t0;
    q.s1 = quad.s1;
    q.t1 = quad.t1;
    
    return 0; // x is advanced by ptr
}

float Font::get_width(const std::string& text) {
    float x = 0;
    float y = 0;
    for (char c : text) {
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad((stbtt_bakedchar*)cdata, tex_width, tex_height, c - 32, &x, &y, &q, 1);
        }
    }
    return x;
}
