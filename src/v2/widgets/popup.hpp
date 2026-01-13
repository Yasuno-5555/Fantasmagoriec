// Fantasmagorie v2 - Popup Widget
#pragma once

#include "widget_base.hpp"
#include "../style/theme.hpp"

namespace fanta {

class PopupBuilder {
    const char* label_;
    float width_ = 200;
    float height_ = -1; // Auto
    float x_ = 0;
    float y_ = 0;
    bool visible_ = true;
    
public:
    explicit PopupBuilder(const char* label) : label_(label) {}
    
    // Fluent API
    PopupBuilder& width(float w) { width_ = w; return *this; }
    PopupBuilder& height(float h) { height_ = h; return *this; }
    PopupBuilder& position(float x, float y) { x_ = x; y_ = y; return *this; }
    PopupBuilder& visible(bool v) { visible_ = v; return *this; }
    
    // Begin popup - content goes between begin() and g_ctx->end_node()
    void begin() {
        if (!g_ctx || !visible_) return;
        
        NodeID id = g_ctx->begin_node(label_);
        NodeHandle n = g_ctx->get(id);
        
        // Absolute positioning
        n.constraint().position_type = LayoutConstraints::Position::Absolute;
        n.constraint().pos_x = x_;
        n.constraint().pos_y = y_;
        n.constraint().width = width_;
        if (height_ > 0) n.constraint().height = height_;
        n.constraint().padding = 12;
        
        // Style
        auto theme = current_theme();
        n.style().bg = theme->color.surface_variant;
        n.style().corner_radius = theme->spacing.corner_large;
        n.style().shadow.color = Color::Hex(0x00000080);
        n.style().shadow.blur_radius = 16;
        n.style().shadow.offset_y = 8;
        n.style().border.width = 1;
        n.style().border.color = theme->color.border;
        
        // Layout
        n.layout().dir = LayoutDir::Column;
        
        // Input - block clicks from falling through
        n.input().clickable = true;
        n.input().hoverable = true;
        
        // Mark as overlay (for z-ordering)
        // Note: The actual layer switching happens in Renderer
        // For now we just mark it
        n.scroll().clip_content = false; // Popups don't clip
    }
};

inline PopupBuilder Popup(const char* label = "popup") {
    return PopupBuilder(label);
}

} // namespace fanta
