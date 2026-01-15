#pragma once
#include <string>
#include <vector>
#include "../platform/gl_lite.hpp"

// Forward declare stb type to avoid heavy include in header
struct stbtt_bakedchar;

struct Font {
    GLuint texture_id = 0;
    float height = 0;
    
    // For ASCII (32..126)
    // We will allocate this array dynamically to avoid incomplete type in header dependent on include order if we strictly hide it, 
    // but simplifying: we can just include stb_truetype in cpp and store data in vector or raw pointer.
    void* cdata = nullptr; // actually stbtt_bakedchar*
    int tex_width = 512;
    int tex_height = 512;
    
    ~Font();
    
    bool load(const std::string& filename, float font_height);
    
    // Returns advanced x position
    // q is output: x0,y0,s0,t0, x1,y1,s1,t1
    struct Quad {
        float x0, y0, s0, t0;
        float x1, y1, s1, t1;
    };
    
    float get_quad(int char_index, float* x, float* y, Quad& q);
    float get_width(const std::string& text);
};
