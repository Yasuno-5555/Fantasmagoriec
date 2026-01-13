// Fantasmagorie v2 - Debug Utils Implementation
#include "debug.hpp"

namespace fanta {

void DebugOverlay::draw_layout_bounds(UIContext& ctx, DrawList& list) {
    if (!ctx.show_layout_bounds) return;
    
    for (NodeID id : ctx.store.nodes) {
        if (!ctx.store.exists(id)) continue;
        
        const auto& layout = ctx.store.layout[id];
        const auto& constraint = ctx.store.constraints[id];
        
        if (layout.w <= 0 || layout.h <= 0) continue;
        
        // 1. Draw Padding Box (Green tint)
        if (constraint.padding > 0) {
            float p = constraint.padding;
            // Draw 4 rects for padding? Or just a transparent overlay
            // Let's draw a border for the content area
            list.add_border(
                layout.x + p, layout.y + p, 
                layout.w - p*2, layout.h - p*2,
                0x00FF0080, // Semitransparent Green
                1.0f
            );
        }
        
        // 2. Draw Bounds (Blue outline)
        list.add_border(
            layout.x, layout.y, layout.w, layout.h, 
            0x0000FF40, // Faint Blue
            1.0f
        );
    }
}

void DebugOverlay::draw_hover_highlight(UIContext& ctx, DrawList& list) {
    if (ctx.debug_hover_id == INVALID_NODE) return;
    if (!ctx.store.exists(ctx.debug_hover_id)) return;
    
    const auto& layout = ctx.store.layout[ctx.debug_hover_id];
    
    // Draw thick red border + highlight
    list.add_rect(layout.x, layout.y, layout.w, layout.h, 0xFF000020, 0); // Red tint
    list.add_border(
        layout.x, layout.y, layout.w, layout.h, 
        0xFF0000FF, // Solid Red
        2.0f
    );
}

} // namespace fanta
