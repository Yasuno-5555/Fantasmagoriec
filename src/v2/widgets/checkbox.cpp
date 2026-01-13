#include "checkbox.hpp"
#include "../style/theme.hpp"

namespace fanta {

void CheckboxBuilder::build() {
    if (built_ || !g_ctx || !value_) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(label_);
    NodeHandle n = g_ctx->get(id);
    
    // Defaults
    if (common.width <= 0) common.width = 120;
    if (common.height <= 0) common.height = 24;
    common.apply(n);
    
    bool checked = *value_;
    auto theme = current_theme();
    
    n.style().bg = checked ? theme->color.primary : theme->color.surface_variant;
    n.style().corner_radius = theme->spacing.corner_small;
    
    n.input().clickable = true;
    n.input().hoverable = true;
    
    // Toggle on click
    bool hovered = is_hovered(id);
    is_clicked(id); // Set active if clicked
    
    if (is_active(id) && g_ctx->mouse_released) {
        if (hovered) {
            *value_ = !(*value_);
        }
        clear_active();
    }
    
    n.render().is_text = true;
    n.render().text = label_;
    
    g_ctx->end_node();
}

} // namespace fanta
