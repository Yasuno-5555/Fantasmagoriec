// Fantasmagorie v2 - Debug Utils Implementation
#include "core/debug.hpp"

namespace fanta {

void DebugOverlay::draw_layout_bounds(UIContext& ctx, PaintList& list) {
    if (!ctx.show_layout_bounds) return;
    
    for (NodeID id : ctx.store.nodes) {
        if (!ctx.store.exists(id)) continue;
        
        const auto& l = ctx.store.layout[id];
        const auto& constraint = ctx.store.constraints[id];
        
        if (l.w <= 0 || l.h <= 0) continue;
        
        // 1. Draw Bounds (Blue outline)
        auto& r = list.rect({l.x, l.y, l.w, l.h}, Color::Clear());
        r.border_color = Color::Hex(0x0000FF80);
        r.border_width = 1.0f;
        
        // 2. Draw Padding Box (Green tint)
        if (constraint.padding > 0) {
            float p = constraint.padding;
            auto& pr = list.rect({l.x + p, l.y + p, l.w - p*2, l.h - p*2}, Color::Clear());
            pr.border_color = Color::Hex(0x00FF0080);
            pr.border_width = 1.0f;
        }
    }
}

void DebugOverlay::draw_hover_highlight(UIContext& ctx, PaintList& list) {
    if (ctx.debug_hover_id == INVALID_NODE) return;
    if (!ctx.store.exists(ctx.debug_hover_id)) return;
    
    const auto& l = ctx.store.layout[ctx.debug_hover_id];
    
    // Draw thick red border + highlight
    auto& r = list.rect({l.x, l.y, l.w, l.h}, Color::Hex(0xFF000020));
    r.border_color = Color::Hex(0xFF0000FF);
    r.border_width = 2.0f;
}

} // namespace fanta


