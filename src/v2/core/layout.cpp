// Fantasmagorie v2 - Layout Engine Implementation
#include "layout.hpp"

namespace fanta {

void LayoutEngine::solve(NodeStore& store, NodeID root, float width, float height) {
    if (!store.exists(root)) return;
    
    // Set root geometry
    auto& layout = store.layout[root];
    layout.x = 0;
    layout.y = 0;
    layout.w = width;
    layout.h = height;
    
    solve_node(store, root, 0, 0, width, height);
}

void LayoutEngine::solve_node(NodeStore& store, NodeID id, float x, float y, float w, float h) {
    auto& tree = store.tree[id];
    auto& layout = store.layout[id];
    auto& constraints = store.constraints[id];
    
    // Update this node's position
    layout.x = x;
    layout.y = y;
    layout.w = w;
    layout.h = h;
    
    if (tree.children.empty()) return;
    
    float padding = constraints.padding;
    float content_x = x + padding;
    float content_y = y + padding;
    float content_w = w - padding * 2;
    float content_h = h - padding * 2;
    
    bool is_row = (layout.dir == LayoutDir::Row);
    float main_axis = is_row ? content_w : content_h;
    float cross_axis = is_row ? content_h : content_w;
    
    // Phase 1: Measure fixed sizes and collect grow factors
    float total_fixed = 0;
    float total_grow = 0;
    
    for (NodeID child_id : tree.children) {
        auto& child_c = store.constraints[child_id];
        float fixed = is_row ? child_c.width : child_c.height;
        
        if (fixed >= 0) {
            total_fixed += fixed;
        }
        total_grow += child_c.grow;
    }
    
    // Phase 2: Distribute remaining space
    float remaining = main_axis - total_fixed;
    float cursor = is_row ? content_x : content_y;
    
    for (NodeID child_id : tree.children) {
        auto& child_c = store.constraints[child_id];
        
        // Main axis size
        float main_size;
        float fixed = is_row ? child_c.width : child_c.height;
        if (fixed >= 0) {
            main_size = fixed;
        } else if (total_grow > 0 && child_c.grow > 0) {
            main_size = remaining * (child_c.grow / total_grow);
        } else {
            main_size = 0;
        }
        
        // Cross axis size
        float cross_fixed = is_row ? child_c.height : child_c.width;
        float cross_size = (cross_fixed >= 0) ? cross_fixed : cross_axis;
        
        // Position and recurse
        float child_x, child_y, child_w, child_h;
        if (is_row) {
            child_x = cursor;
            child_y = content_y;
            child_w = main_size;
            child_h = cross_size;
            cursor += main_size;
        } else {
            child_x = content_x;
            child_y = cursor;
            child_w = cross_size;
            child_h = main_size;
            cursor += main_size;
        }
        
        solve_node(store, child_id, child_x, child_y, child_w, child_h);
    }
}

void LayoutEngine::measure_children(NodeStore& store, NodeID id) {
    // TODO: Implement content-based sizing for auto-sized nodes
}

} // namespace fanta
