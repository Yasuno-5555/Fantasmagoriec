// Fantasmagorie v2 - Label Implementation
#include "label.hpp"

namespace fanta {

void LabelBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(text_);
    NodeHandle n = g_ctx->get(id);
    
    // Auto-size (will be calculated by layout)
    n.constraint().width = -1;  // Auto
    n.constraint().height = 20; // Line height
    
    n.style().text = Color::Hex(color_);
    n.render().is_text = true;
    n.render().text = text_;
    
    g_ctx->end_node();
}

} // namespace fanta
