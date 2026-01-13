// Fantasmagorie v2 - Renderer Implementation
#include "renderer.hpp"
#include <algorithm>

namespace fanta {

void Renderer::submit(NodeStore& store, NodeID root, DrawList& list) {
    if (!store.exists(root)) return;
    submit_node(store, root, list);
}

void Renderer::submit_node(NodeStore& store, NodeID id, DrawList& list) {
    auto& layout = store.layout[id];
    
    // Skip if invisible (size 0)
    if (layout.w <= 0 || layout.h <= 0) return;
    
    // Resolve Style (mix current style with visual state if implicit animation is active)
    // For now, we use the `style` component directly as it's the target.
    // In Implicit Animation Phase, we will use `visual_state.current`.
    bool use_visual_state = store.visual_state.find(id) != store.visual_state.end() 
                            && store.visual_state[id].initialized;
                            
    const ResolvedStyle* s = use_visual_state ? &store.visual_state[id].current : &store.style[id];
    
    // Apply Visual Scale (Tactile Effect)
    float vx = layout.x;
    float vy = layout.y;
    float vw = layout.w;
    float vh = layout.h;
    
    if (std::abs(s->scale - 1.0f) > 0.001f) {
        float cx = layout.x + layout.w * 0.5f;
        float cy = layout.y + layout.h * 0.5f;
        vw = layout.w * s->scale;
        vh = layout.h * s->scale;
        vx = cx - vw * 0.5f;
        vy = cy - vh * 0.5f;
    }
    
    // 1. Shadow Layer
    if (s->shadow.color.a > 0) {
        list.add_shadow(
            vx + s->shadow.offset_x,
            vy + s->shadow.offset_y,
            vw + s->shadow.spread * 2, // Spread increases rect size
            vh + s->shadow.spread * 2,
            s->shadow.blur_radius,
            s->shadow.color.u32(),
            s->shadow.blur_radius // Softness = Blur Radius for now
        );
    }
    
    // 2. Background Layer
    if (s->bg.a > 0) {
        list.add_rect(vx, vy, vw, vh, 
                     s->bg.u32(), s->corner_radius);
    }
    
    // 3. Content Layer (Text/Image)
    auto& render = store.render[id];
    if (render.is_text && !render.text.empty()) {
        // Center text vertically relative to visual rect
        float tx = vx + 5; // Padding
        float ty = vy + (vh - 14) / 2; // Vert center approx
        
        list.add_text(tx, ty, render.text.c_str(), s->text.u32());
    } else if (render.is_image && render.texture) {
        list.add_image(vx, vy, vw, vh, render.texture,
                      render.u0, render.v0, render.u1, render.v1);
    } else if (render.is_icon) {
        // Icon rendering
        // Use text alignment logic or just center?
        // Usually icons in buttons are centered or use layout.
        // But Icon widget sets size.
        // Let's assume layout bounds are correct.
        
        // Fit icon in layout but keep aspect ratio (usually 1:1)
        float size = std::min(vw, vh);
        float ix = vx + (vw - size) * 0.5f;
        float iy = vy + (vh - size) * 0.5f;
        
        uint32_t color = render.icon_color;
        // If color was not set (white default), verify?
        // Actually Icon widget initializes it. Use it.
        // If we wanted to inherit text color:
        // if (color == 0) color = s->text.u32(); // Hypothetical
        
        list.add_icon(ix, iy, size, render.icon, color);
    }
    
    // 4. Border Layer
    if (s->border.width > 0) {
        bool biased = (s->border.color_light.a > 0 || s->border.color_dark.a > 0);
        
        if (!biased && s->border.color.a > 0) {
            // Standard uniform border
            list.add_border(vx, vy, vw, vh, 
                           s->border.color.u32(), s->border.width, s->corner_radius);
        } else if (biased) {
            // Biased Border (Glass Effect) - Draws 4 separate sides
            // Note: Corner radius handling for separate sides is complex without path stroking.
            // For MVP, we draw lines/rects. If corners are large, this might look glitchy.
            // But for typical buttons/panels (radius ~4-8), overlapping lines is "okay".
            
            Color c_top = (s->border.color_light.a > 0) ? s->border.color_light : s->border.color;
            Color c_left = c_top;
            Color c_bot = (s->border.color_dark.a > 0) ? s->border.color_dark : s->border.color;
            Color c_right = c_bot;
            
            float th = s->border.width;
            
            // Top
            list.add_rect(vx, vy, vw, th, c_top.u32(), 0);
            // Left
            list.add_rect(vx, vy, th, vh, c_left.u32(), 0);
            // Right
            list.add_rect(vx + vw - th, vy, th, vh, c_right.u32(), 0);
            // Bottom
            list.add_rect(vx, vy + vh - th, vw, th, c_bot.u32(), 0);
        }
    }
    
    // 5. Children (Recursion)
    auto& tree = store.tree[id];
    for (NodeID child_id : tree.children) {
        submit_node(store, child_id, list);
    }
    
    // 5.5. Scrollbar (if scrollable)
    auto& scroll = store.scroll[id];
    if (scroll.scrollable && scroll.max_scroll_y > 0) {
        // Calculate scrollbar dimensions
        float bar_width = 6.0f;
        float bar_x = vx + vw - bar_width - 2.0f; // 2px from right edge
        
        float viewport_h = vh;
        float content_h = viewport_h + scroll.max_scroll_y;
        float thumb_h = std::max(20.0f, viewport_h * (viewport_h / content_h));
        float thumb_travel = viewport_h - thumb_h;
        float thumb_y = vy + (scroll.max_scroll_y > 0 
            ? thumb_travel * (scroll.scroll_y / scroll.max_scroll_y) 
            : 0);
        
        // Track (background)
        list.add_rect(bar_x, vy, bar_width, viewport_h, 0x20202040, bar_width / 2);
        
        // Thumb (foreground)
        list.add_rect(bar_x, thumb_y, bar_width, thumb_h, 0x80808080, bar_width / 2);
    }
    
    // 6. Overlay Layer (Focus Ring)
    if (s->show_focus_ring) {
        // Draw focus ring outside border?
        float offset = 2.0f;
        list.add_border(
            vx - offset, vy - offset, 
            vw + offset*2, vh + offset*2,
            s->focus_ring_color.u32(), 
            s->focus_ring_width, 
            s->corner_radius + offset
        );
    }
}

} // namespace fanta
