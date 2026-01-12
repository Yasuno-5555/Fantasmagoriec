// Fantasmagorie v2 - Button Implementation
#include "button.hpp"

namespace fanta {

void ButtonBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    // Create node
    NodeID id = g_ctx->begin_node(label_);
    NodeHandle n = g_ctx->get(id);
    
    // Constraints
    if (width_ > 0) n.constraint().width = width_;
    else n.constraint().width = 80;  // Default
    if (height_ > 0) n.constraint().height = height_;
    else n.constraint().height = 32;
    
    // Style based on variant
    uint32_t bg = 0x333333FF;
    uint32_t bg_hover = 0x444444FF;
    uint32_t bg_active = 0x222222FF;
    
    if (is_primary_) {
        bg = 0x4A90D9FF;
        bg_hover = 0x5AA0E9FF;
        bg_active = 0x3A80C9FF;
    } else if (is_danger_) {
        bg = 0xD94A4AFF;
        bg_hover = 0xE95A5AFF;
        bg_active = 0xC93A3AFF;
    }
    
    // Interaction state
    bool hovered = is_hovered(id);
    bool clicked = is_clicked(id);
    
    // Apply style
    n.style().bg = Color::Hex(clicked ? bg_active : (hovered ? bg_hover : bg));
    n.style().corner_radius = 4.0f;
    n.style().text = Color::White();
    
    // Input
    n.input().clickable = true;
    n.input().hoverable = true;
    
    // Render text
    n.render().is_text = true;
    n.render().text = label_;
    
    // Handle click (on release)
    static bool was_down = false;
    if (hovered && was_down && !g_ctx->mouse_down && on_click_) {
        on_click_();
    }
    was_down = g_ctx->mouse_down && hovered;
    
    g_ctx->end_node();
}

} // namespace fanta
