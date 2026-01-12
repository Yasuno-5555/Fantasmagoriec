#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "../platform/gl_lite.hpp"
#include "font_manager.hpp"

#include "../core/draw_types.hpp"

// DrawCmd defined in draw_types.hpp


struct DrawList {
    // Fixed size array of vectors (4 = RenderLayer::Count)
    std::vector<DrawCmd> cmds[4];
    
    // Clip Stack
    struct Rect { float x,y,w,h; };
    std::vector<Rect> clip_stack;
    Rect current_clip = {-10000, -10000, 20000, 20000}; 
    
    RenderLayer current_layer = RenderLayer::Content;
    
    void clear();
    void push_clip_rect(float x, float y, float w, float h);
    void pop_clip_rect();
    void set_layer(RenderLayer l) { current_layer = l; }

    void add_rect(float x, float y, float w, float h, uint32_t color, float corner_radius=0.0f);
    void add_texture_rect(float x, float y, float w, float h, void* texture_id, float u0, float v0, float u1, float v1, uint32_t color);
    void add_text(struct Font& font, const std::string& text, float x, float y, uint32_t color, bool bold=false, bool italic=false);
    void add_image(uint32_t tex_id, float x, float y, float w, float h);
    void add_focus_ring(float x, float y, float w, float h, uint32_t color, float thickness=2.0f);
    void add_caret(float x, float y, float h, uint32_t c);
    
    // Phase 4: Async UI
    void add_arc(float cx, float cy, float radius, float min_angle, float max_angle, uint32_t color, float thickness=1.0f);
    
    // Phase 20: Graph Primitives
    void add_line(float x1, float y1, float x2, float y2, uint32_t color, float thickness=1.0f);
    void add_bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, uint32_t color, float thickness=2.0f);
    
    // Core Polish
    void add_scissor(float x, float y, float w, float h);
    
    // Phase 6: Modern UI
    void add_shadow_rect(float x, float y, float w, float h, float radius, uint32_t color, float softness);
};

struct TextMeasurement { float w, h; };
TextMeasurement MeasureText(struct Font& font, const std::string& text);

class Renderer {
public:
    unsigned sp=0, vao=0, vbo=0;
    
    // Cache current texture to avoid state changes
    GLuint current_gl_texture = 0; 
    
    void init();
    void render(const DrawList& l, float w, float h, GLuint atlas_tex);
};
