// Fantasmagorie v2 - ScrollArea Implementation
#include "scroll_area.hpp"
#include <algorithm>

namespace fanta {

// Persistent scroll state
static std::unordered_map<std::string, Vec2> scroll_offsets;

void ScrollAreaBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(id_);
    NodeHandle n = g_ctx->get(id);
    
    apply_constraints(n);
    
    n.style().bg = Color::Transparent();
    n.scroll().scrollable = true;
    n.scroll().clip_content = true;
    
    // Get or init scroll offset
    std::string key = id_;
    if (scroll_offsets.find(key) == scroll_offsets.end()) {
        scroll_offsets[key] = {0, 0};
    }
    Vec2& offset = scroll_offsets[key];
    
    // Handle mouse wheel
    bool hovered = is_hovered(id);
    if (hovered && vertical_) {
        offset.y -= g_ctx->scroll_delta * 30.0f;
        offset.y = std::max(0.0f, offset.y);
    }
    
    n.scroll().scroll_x = offset.x;
    n.scroll().scroll_y = offset.y;
    
    // Build children
    build_children();
    
    // Clamp scroll to content
    float content_h = 0;  // Would be calculated from children
    n.scroll().max_scroll_y = std::max(0.0f, content_h - n.layout().h);
    offset.y = std::min(offset.y, n.scroll().max_scroll_y);
    
    g_ctx->end_node();
}

} // namespace fanta
