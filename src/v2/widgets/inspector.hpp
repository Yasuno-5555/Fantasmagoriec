// Fantasmagorie v2 - Inspector Widget
#pragma once

#include "../core/context.hpp"
#include "../widgets/button.hpp"
#include "../widgets/label.hpp"
#include <string>

namespace fanta {
namespace ui {

class Inspector {
public:
    Inspector(NodeID root) : root_(root) {}
    
    void build() {
        if (!g_ctx) return;
        
        // Root container for inspector
        NodeID id = g_ctx->begin_node("inspector_panel");
        NodeHandle n = g_ctx->get(id);
        n.style().bg = Color::Hex(0x252525FF);
        n.layout().dir = LayoutDir::Column;
        n.constraint().grow = 1;
        n.constraint().padding = 4;
        
        ui::Label("Widget Tree").build();
        
        // Recursive tree render
        render_tree(root_, 0);
        
        g_ctx->end_node();
    }

private:
    NodeID root_;
    
    void render_tree(NodeID id, int depth) {
        if (!g_ctx->store.exists(id)) return;
        
        const auto& tree = g_ctx->store.tree[id];
        
        // Indentation via padding or dummy nodes?
        // Let's use string indent for label for now
        std::string label = std::string(depth * 2, ' ') + "> Node " + std::to_string(id);
        
        // Check if hovered in inspector
        bool is_hovered_in_inspector = false;
        
        // Render row
        {
            NodeID row_id = g_ctx->begin_node(("insp_row_" + std::to_string(id)).c_str());
            NodeHandle row = g_ctx->get(row_id);
            row.layout().dir = LayoutDir::Row;
            row.constraint().height = 20;
            row.style().corner_radius = 2;
            row.input().hoverable = true;
            row.input().clickable = true;
            
            // Highlight if matches debug hover
            if (g_ctx->debug_hover_id == id) {
                row.style().bg = Color::Hex(0x4A90D980); // Highlight blue
            } else if (is_hovered(row_id)) {
                row.style().bg = Color::Hex(0x3A3A3AFF);
                // Set Global Debug Hover
                g_ctx->debug_hover_id = id;
            }
            
            ui::Label(label.c_str()).build();
            
            g_ctx->end_node();
        }
        
        // Children
        for (NodeID child : tree.children) {
            render_tree(child, depth + 1);
        }
    }
};

} // namespace ui
} // namespace fanta
