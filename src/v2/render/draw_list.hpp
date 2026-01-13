// Fantasmagorie v2 - Draw List
// Intermediate representation for rendering commands
#pragma once

#include "../core/types.hpp"
#include <vector>
#include <string>

namespace fanta {

// ============================================================================
// Draw Types
// ============================================================================


enum class DrawType : uint32_t {
    Rect = 0,
    Text = 1,
    Image = 2,
    Border = 3,
    Icon = 4,    // New!
    Line = 10,
    Bezier = 11,
    Arc = 12,
    Scissor = 13,
    Shadow = 14,
    Custom = 99
};

enum class RenderLayer {
    Background,
    Content,
    Overlay,
    Tooltip,
    Count
};

// ============================================================================
// Draw Command (64 bytes aligned)
// ============================================================================

struct DrawCmd {
    DrawType type = DrawType::Rect;
    float x = 0, y = 0, w = 0, h = 0;
    uint32_t color = 0xFFFFFFFF;
    float p1 = 0, p2 = 0, p3 = 0, p4 = 0;  // Type-specific params
    float cx = 0, cy = 0, cw = 0, ch = 0;  // Clip rect
    void* texture = nullptr;
    const char* text = nullptr;
};

// ============================================================================
// Draw List
// ============================================================================

class DrawList {
public:
    std::vector<DrawCmd> cmds[(int)RenderLayer::Count];
    RenderLayer current_layer = RenderLayer::Content;
    
    // Clip Stack
    std::vector<Rect> clip_stack;
    Rect current_clip = {-10000, -10000, 20000, 20000};
    
    void clear() {
        for (auto& v : cmds) v.clear();
        clip_stack.clear();
        current_clip = {-10000, -10000, 20000, 20000};
        current_layer = RenderLayer::Content;
    }
    
    void set_layer(RenderLayer l) { current_layer = l; }
    
    void push_clip(float x, float y, float w, float h) {
        clip_stack.push_back(current_clip);
        // Intersect with current
        float nx = std::max(x, current_clip.x);
        float ny = std::max(y, current_clip.y);
        float nx2 = std::min(x + w, current_clip.x + current_clip.w);
        float ny2 = std::min(y + h, current_clip.y + current_clip.h);
        
        // Ensure strictly positive size
        if (nx2 < nx) nx2 = nx;
        if (ny2 < ny) ny2 = ny;
        
        current_clip = {nx, ny, nx2 - nx, ny2 - ny};
    }
    
    void pop_clip() {
        if (!clip_stack.empty()) {
            current_clip = clip_stack.back();
            clip_stack.pop_back();
        }
    }
    
    // ========================================================================
    // Drawing Commands
    // ========================================================================
    
    void add_rect(float x, float y, float w, float h, uint32_t color, float radius = 0) {
        DrawCmd cmd;
        cmd.type = DrawType::Rect;
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.color = color;
        cmd.p1 = radius;  // Corner radius
        cmd.cx = current_clip.x; cmd.cy = current_clip.y;
        cmd.cw = current_clip.w; cmd.ch = current_clip.h;
        cmds[(int)current_layer].push_back(cmd);
    }
    
    void add_text(float x, float y, const char* text, uint32_t color, void* font = nullptr) {
        DrawCmd cmd;
        cmd.type = DrawType::Text;
        cmd.x = x; cmd.y = y;
        cmd.color = color;
        cmd.text = text;
        cmd.texture = font;
        cmd.cx = current_clip.x; cmd.cy = current_clip.y;
        cmd.cw = current_clip.w; cmd.ch = current_clip.h;
        cmds[(int)current_layer].push_back(cmd);
    }

    void add_border(float x, float y, float w, float h, uint32_t color, float thickness, float radius = 0) {
        DrawCmd cmd;
        cmd.type = DrawType::Border;
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.color = color;
        cmd.p1 = radius;
        cmd.p2 = thickness;
        cmd.cx = current_clip.x; cmd.cy = current_clip.y;
        cmd.cw = current_clip.w; cmd.ch = current_clip.h;
        cmds[(int)current_layer].push_back(cmd);
    }
    
    void add_image(float x, float y, float w, float h, void* texture,
                   float u0 = 0, float v0 = 0, float u1 = 1, float v1 = 1) {
        DrawCmd cmd;
        cmd.type = DrawType::Image;
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.texture = texture;
        cmd.p1 = u0; cmd.p2 = v0; cmd.p3 = u1; cmd.p4 = v1;
        cmd.color = 0xFFFFFFFF;
        cmd.cx = current_clip.x; cmd.cy = current_clip.y;
        cmd.cw = current_clip.w; cmd.ch = current_clip.h;
        cmds[(int)current_layer].push_back(cmd);
    }
    
    void add_line(float x1, float y1, float x2, float y2, uint32_t color, float thickness = 1) {
        DrawCmd cmd;
        cmd.type = DrawType::Line;
        cmd.x = x1; cmd.y = y1;
        cmd.w = x2; cmd.h = y2;  // Store end point in w/h
        cmd.color = color;
        cmd.p1 = thickness;
        cmds[(int)current_layer].push_back(cmd);
    }
    
    void add_shadow(float x, float y, float w, float h, float radius, uint32_t color, float softness) {
        DrawCmd cmd;
        cmd.type = DrawType::Shadow;
        cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
        cmd.color = color;
        cmd.p1 = radius;
        cmd.p2 = softness;
        cmds[(int)current_layer].push_back(cmd);
    }
    
    void add_arc(float cx, float cy, float radius, float start_angle, float end_angle, 
                 uint32_t color, float thickness = 1) {
        DrawCmd cmd;
        cmd.type = DrawType::Arc;
        cmd.x = cx; cmd.y = cy;
        cmd.w = radius;
        cmd.p1 = start_angle;
        cmd.p2 = end_angle;
        cmd.p3 = thickness;
        cmd.color = color;
        cmds[(int)current_layer].push_back(cmd);
    }
    
    void add_bezier(float x1, float y1, float x2, float y2,
                    float cx1, float cy1, float cx2, float cy2,
                    uint32_t color, float thickness = 2) {
        DrawCmd cmd;
        cmd.type = DrawType::Bezier;
        cmd.x = x1; cmd.y = y1;
        cmd.w = x2; cmd.h = y2;
        cmd.p1 = cx1; cmd.p2 = cy1;
        cmd.p3 = cx2; cmd.p4 = cy2;
        cmd.color = color;
        cmds[(int)current_layer].push_back(cmd);
    }
    
    void add_icon(float x, float y, float size, IconType icon, uint32_t color);
};

} // namespace fanta
