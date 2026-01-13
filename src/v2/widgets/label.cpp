// Fantasmagorie v2 - Label Implementation
#include "label.hpp"

namespace fanta {

void LabelBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(text_);
    NodeHandle n = g_ctx->get(id);
    
    // Apply common properties
    if (common.width <= 0) common.width = -1; // Auto
    if (common.height <= 0) common.height = 20; // Default line height
    common.apply(n);
    
    n.style().text = Color::Hex(color_);
    n.render().is_text = true;
    n.render().text = text_;
    
    g_ctx->end_node();
}

} // namespace fanta
