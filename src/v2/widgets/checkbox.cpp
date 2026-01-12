// Fantasmagorie v2 - Checkbox Implementation
#include "checkbox.hpp"

namespace fanta {

void CheckboxBuilder::build() {
    if (built_ || !g_ctx || !value_) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(label_);
    NodeHandle n = g_ctx->get(id);
    
    n.constraint().width = 120;
    n.constraint().height = 24;
    
    bool checked = *value_;
    n.style().bg = Color::Hex(checked ? 0x4A90D9FF : 0x333333FF);
    n.style().corner_radius = 4.0f;
    
    n.input().clickable = true;
    n.input().hoverable = true;
    
    // Toggle on click
    bool hovered = is_hovered(id);
    static bool was_down = false;
    if (hovered && was_down && !g_ctx->mouse_down) {
        *value_ = !(*value_);
    }
    was_down = g_ctx->mouse_down && hovered;
    
    n.render().is_text = true;
    n.render().text = label_;
    
    g_ctx->end_node();
}

} // namespace fanta
