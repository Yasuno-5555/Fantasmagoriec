#pragma once

#include "core/types_internal.hpp"
#include "core/font_types.hpp"
#include <vector>

namespace fanta {
namespace internal {

enum class DrawCmdType {
    Rectangle,        
    RoundedRect, 
    Circle,      
    Line,        
    Text,
    Bezier,        // Phase 7
    PushTransform, // Phase 7
    PopTransform,  // Phase 7
    PushClip,      // Phase 8
    PopClip        // Phase 8
};

struct DrawCmd {
    DrawCmdType type;
    
    union {
        struct {
            float pos_x, pos_y;      
            float size_x, size_y;    
            float color_r, color_g, color_b, color_a;
            float elevation;          
        } rectangle;
        
        struct {
            float pos_x, pos_y;      
            float size_x, size_y;     
            float radius;             
            float color_r, color_g, color_b, color_a;
            float elevation;          
        } rounded_rect;
        
        struct {
            float center_x, center_y; 
            float r;                  
            float color_r, color_g, color_b, color_a;
            float elevation;          
        } circle;
        
        struct {
            float p0_x, p0_y;         
            float p1_x, p1_y;         
            float thickness;           
            float color_r, color_g, color_b, color_a;
        } line;

        struct {
            uint32_t run_index; 
            float color_r, color_g, color_b, color_a;
        } text;
        
        // Phase 7: Node Graph Primitives
        struct {
            float p0_x, p0_y;
            float p1_x, p1_y;
            float p2_x, p2_y;
            float p3_x, p3_y;
            float thickness;
            float color_r, color_g, color_b, color_a;
        } bezier;
        
        struct {
            float tx, ty;
            float scale;
        } transform;

        struct {
            float x, y, w, h;
        } clip;
    };
};

struct DrawList {
    std::vector<DrawCmd> commands;
    std::vector<TextRun> text_runs; 
    
    void add_rect(const Vec2& pos, const Vec2& size, const ColorF& c, float elevation=0); 
    void add_rounded_rect(const Vec2& pos, const Vec2& size, float r, const ColorF& c, float elevation=0);
    void add_circle(const Vec2& center, float r, const ColorF& c, float elevation=0);
    void add_line(const Vec2& p0, const Vec2& p1, float thickness, const ColorF& c);
    void add_text(const TextRun& run, const ColorF& c); 
    
    // Phase 7
    void add_bezier(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, float thickness, const ColorF& c);
    void push_transform(const Vec2& translate, float scale);
    void pop_transform(); 
    
    // Phase 8
    void push_clip(const Vec2& pos, const Vec2& size);
    void pop_clip();    
    void clear() { commands.clear(); text_runs.clear(); }
    size_t size() const { return commands.size(); }
    bool empty() const { return commands.empty(); }
};

} // namespace internal
} // namespace fanta
