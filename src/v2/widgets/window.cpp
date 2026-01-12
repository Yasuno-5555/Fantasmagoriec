// Fantasmagorie v2 - Window Implementation
#include "window.hpp"

namespace fanta {

// Persistent window state
static std::unordered_map<std::string, Vec2> window_positions;
static std::unordered_map<std::string, Vec2> window_sizes;

void WindowBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    std::string key = title_;
    
    // Get or initialize position
    if (window_positions.find(key) == window_positions.end()) {
        window_positions[key] = {100, 100};
        window_sizes[key] = {width_, height_};
    }
    
    Vec2& pos = window_positions[key];
    Vec2& size = window_sizes[key];
    
    NodeID id = g_ctx->begin_node(title_);
    NodeHandle n = g_ctx->get(id);
    
    // Apply constraints
    n.constraint().width = size.x;
    n.constraint().height = size.y;
    n.constraint().position_type = LayoutConstraints::Position::Absolute;
    n.constraint().pos_x = pos.x;
    n.constraint().pos_y = pos.y;
    
    // Style
    n.style().bg = Color::Hex(0x1E1E1EFF);
    n.style().corner_radius = 8.0f;
    n.style().shadow = Color::Hex(0x00000080);
    n.style().shadow_radius = 16.0f;
    n.style().shadow_offset_y = 4.0f;
    
    n.layout().dir = LayoutDir::Column;
    
    // Title bar (simplified - would be a child node in full impl)
    bool title_hovered = g_ctx->mouse_pos.y < n.layout().y + 32 &&
                         g_ctx->mouse_pos.y >= n.layout().y &&
                         g_ctx->mouse_pos.x >= n.layout().x &&
                         g_ctx->mouse_pos.x < n.layout().x + n.layout().w;
    
    // Dragging
    static NodeID dragging_window = INVALID_NODE;
    static Vec2 drag_offset;
    
    if (draggable_) {
        if (title_hovered && g_ctx->mouse_down && dragging_window == INVALID_NODE) {
            dragging_window = id;
            drag_offset = g_ctx->mouse_pos - pos;
        }
        
        if (dragging_window == id) {
            if (g_ctx->mouse_down) {
                pos = g_ctx->mouse_pos - drag_offset;
            } else {
                dragging_window = INVALID_NODE;
            }
        }
    }
    
    // Close button
    if (closable_) {
        float close_x = n.layout().x + n.layout().w - 28;
        float close_y = n.layout().y + 8;
        bool close_hovered = g_ctx->mouse_pos.x >= close_x && 
                            g_ctx->mouse_pos.x < close_x + 16 &&
                            g_ctx->mouse_pos.y >= close_y &&
                            g_ctx->mouse_pos.y < close_y + 16;
        
        static bool was_down = false;
        if (close_hovered && was_down && !g_ctx->mouse_down && on_close_) {
            on_close_();
        }
        was_down = g_ctx->mouse_down && close_hovered;
    }
    
    // Build children
    build_children();
    
    g_ctx->end_node();
}

} // namespace fanta
