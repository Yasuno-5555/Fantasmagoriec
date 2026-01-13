// Fantasmagorie v2 - Playground Implementation
#include "playground.hpp"
#include "../core/debug.hpp"
#include "../widgets/label.hpp"
#include "../widgets/button.hpp"
#include "../widgets/inspector.hpp"

namespace fanta {

void Playground::render_editor(UIContext& ctx) {
    // Left Panel: Toolbar + Inspector
    // Use PanelScope if available, or manual node
    NodeID id = ctx.begin_node("playground_editor");
    NodeHandle n = ctx.get(id);
    n.style().bg = Color::Hex(0x202020FF);
    n.constraint().width = 300; // Fixed width sidebar
    n.constraint().grow = 0;
    n.layout().dir = LayoutDir::Column;
    n.constraint().padding = 8;
    
    render_toolbar(ctx);
    render_inspector(ctx);
    
    ctx.end_node();
}

void Playground::render_preview(UIContext& ctx) {
    // Right Panel: Live Preview
    NodeID id = ctx.begin_node("playground_preview");
    NodeHandle n = ctx.get(id);
    n.style().bg = Color::Hex(0x101010FF); // Darker background
    n.constraint().grow = 1;
    n.layout().dir = LayoutDir::Column;
    n.constraint().padding = 20;
    
    // Placeholder content
    ui::Label("Live Preview").build();
    
    ctx.end_node();
}

void Playground::draw_debug_overlay(UIContext& ctx, DrawList& list) {
    DebugOverlay::draw_layout_bounds(ctx, list);
    DebugOverlay::draw_hover_highlight(ctx, list);
}

void Playground::render_toolbar(UIContext& ctx) {
    NodeID id = ctx.begin_node("pg_toolbar");
    NodeHandle n = ctx.get(id);
    n.layout().dir = LayoutDir::Row;
    n.constraint().height = 40;
    n.style().bg = Color::Hex(0x2A2A2AFF);
    n.style().corner_radius = 4;
    n.constraint().padding = 4;
    
    // Toggle Layout Debug
    if (ui::Button(ctx.show_layout_bounds ? "Hide Layout" : "Show Layout").build_clicked()) {
        ctx.show_layout_bounds = !ctx.show_layout_bounds;
    }
    
    ctx.end_node();
}

void Playground::render_inspector(UIContext& ctx) {
    // Inspect the preview panel
    NodeID preview_id = ctx.get_id("playground_preview");
    ui::Inspector(preview_id).build();
}

} // namespace fanta
