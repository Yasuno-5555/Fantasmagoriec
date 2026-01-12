// Fantasmagorie v2 - TreeView Implementation
#include "tree.hpp"

namespace fanta {

void TreeViewBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(id_);
    NodeHandle n = g_ctx->get(id);
    
    apply_constraints(n);
    n.layout().dir = LayoutDir::Column;
    
    build_node(root_, 0);
    
    g_ctx->end_node();
}

void TreeViewBuilder::build_node(TreeNode& node, int depth) {
    char node_id[64];
    snprintf(node_id, sizeof(node_id), "tree_%s_%d", node.label, depth);
    
    NodeID id = g_ctx->begin_node(node_id);
    NodeHandle n = g_ctx->get(id);
    
    n.constraint().height = 24.0f;
    n.constraint().padding = 4.0f;
    n.layout().dir = LayoutDir::Row;
    
    // Indent
    float left_pad = depth * indent_;
    
    // Expand arrow (if has children)
    bool has_children = !node.children.empty();
    
    if (has_children) {
        char arrow_id[64];
        snprintf(arrow_id, sizeof(arrow_id), "arrow_%s", node.label);
        
        NodeID arrow_node = g_ctx->begin_node(arrow_id);
        NodeHandle arrow = g_ctx->get(arrow_node);
        arrow.constraint().width = 16.0f;
        arrow.input().clickable = true;
        arrow.render().is_text = true;
        arrow.render().text = node.expanded ? "v" : ">";
        
        if (is_clicked(arrow_node)) {
            node.expanded = !node.expanded;
        }
        
        g_ctx->end_node();
    } else {
        // Spacer for alignment
        NodeID spacer = g_ctx->begin_node("spacer");
        g_ctx->get(spacer).constraint().width = 16.0f;
        g_ctx->end_node();
    }
    
    // Label
    {
        char label_id[64];
        snprintf(label_id, sizeof(label_id), "label_%s", node.label);
        
        NodeID label_node = g_ctx->begin_node(label_id);
        NodeHandle label = g_ctx->get(label_node);
        label.constraint().grow = 1.0f;
        label.render().is_text = true;
        label.render().text = node.label;
        label.input().clickable = true;
        
        bool hovered = is_hovered(label_node);
        if (hovered) {
            label.style().bg = Color::Hex(0x333333FF);
        }
        
        if (is_clicked(label_node) && on_select_) {
            on_select_(&node);
        }
        
        g_ctx->end_node();
    }
    
    g_ctx->end_node();
    
    // Children (if expanded)
    if (has_children && node.expanded) {
        for (auto& child : node.children) {
            build_node(child, depth + 1);
        }
    }
}

} // namespace fanta
