// Fantasmagorie v2 - Button Implementation
#include "button.hpp"
#include "../style/theme.hpp"

namespace fanta {

void ButtonBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    // Create node
    NodeID id = g_ctx->begin_node(label_);
    NodeHandle n = g_ctx->get(id);
    
    // Apply common properties (constraints + implicit animations)
    // Default size if not set
    if (common.width <= 0) common.width = 80;
    if (common.height <= 0) common.height = 32;
    common.focusable = true;
    
    common.apply(n);
    
    // Style based on variant
    auto theme = current_theme();
    Color bg = theme->color.surface_variant;
    Color bg_hover = theme->color.secondary; 
    Color bg_active = theme->color.primary;
    
    if (is_primary_) {
        bg = theme->color.primary;
        bg_hover = theme->color.primary; // Should lighten in future
        bg_active = theme->color.primary; // Should darken
    } else if (is_danger_) {
        bg = theme->color.error;
    }
    
    // Interaction state
    bool hovered = is_hovered(id);
    bool pressed = is_active(id) && g_ctx->mouse_down;
    
    // Apply style
    n.style().bg = pressed ? bg_active : (hovered ? bg_hover : bg);
    n.style().corner_radius = theme->spacing.corner_medium;
    n.style().text = theme->color.fg;
    
    if (theme->effect.enable_shadows) {
        n.style().shadow.color = Color::Hex(0x00000040);
        n.style().shadow.blur_radius = theme->effect.shadow_radius_base;
        n.style().shadow.offset_y = 2.0f;
    }
    
    // Input
    n.input().clickable = true;
    n.input().hoverable = true;
    
    // Render text
    n.render().is_text = true;
    n.render().text = label_;
    
    // Handle click (on release if it was started on this button)
    if (is_active(id) && g_ctx->mouse_released) {
        if (hovered && on_click_) {
            on_click_();
        }
        clear_active();
    }
    
    g_ctx->end_node();
}

} // namespace fanta
