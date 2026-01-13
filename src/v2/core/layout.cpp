// Fantasmagorie v2 - Layout Engine Implementation
#include "layout.hpp"

namespace fanta {

void LayoutEngine::solve(NodeStore& store, NodeID root, float width, float height) {
    if (!store.exists(root)) return;
    
    // Phase 0: Measure content sizes recursively
    measure_children(store, root);
    
    // Set root geometry
    auto& target_c = store.constraints[root];
    float root_w = (target_c.width >= 0) ? target_c.width : width;
    float root_h = (target_c.height >= 0) ? target_c.height : height;

    solve_node(store, root, 0, 0, root_w, root_h);
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
        } else if (child_c.grow <= 0) {
            // "Shrink-to-content": Use measured size as fixed if not growing
            auto& child_layout = store.layout[child_id];
            total_fixed += is_row ? child_layout.w : child_layout.h;
        }
        total_grow += child_c.grow;
    }
    
// Gap Support Implementation
    float gap = constraints.gap;
    
    // Phase 2: Distribute remaining space
    float remaining = main_axis - total_fixed;
    
    // Subtract gaps from remaining space
    if (gap > 0 && tree.children.size() > 1) {
        remaining -= gap * (tree.children.size() - 1);
    }
    
    float cursor = is_row ? content_x : content_y;
    
    for (size_t i = 0; i < tree.children.size(); ++i) {
        NodeID child_id = tree.children[i];
        
        auto& child_c = store.constraints[child_id];
        auto& child_layout_state = store.layout[child_id]; // Already has measured w,h
        
        // Main axis size
        float main_size;
        float fixed = is_row ? child_c.width : child_c.height;
        float min_main = is_row ? child_c.min_width : child_c.min_height;
        
        if (fixed >= 0) {
            main_size = std::max(fixed, min_main);
        } else if (child_c.grow > 0 && total_grow > 0) {
            main_size = remaining * (child_c.grow / total_grow);
            main_size = std::max(main_size, min_main);
        } else {
            // Shrink-to-content
            main_size = is_row ? child_layout_state.w : child_layout_state.h;
            main_size = std::max(main_size, min_main);
        }
        
        // Cross axis size
        float cross_fixed = is_row ? child_c.height : child_c.width;
        float min_cross = is_row ? child_c.min_height : child_c.min_width;
        float cross_size;
        
        if (cross_fixed >= 0) {
            cross_size = std::max(cross_fixed, min_cross);
        } else {
            // Stretch or measured? 
            // Default behavior: Stretch to container if not auto
            cross_size = std::max(cross_axis, min_cross); 
        }
        
        // Position and recurse
        // Position with Scroll Offset
        float scroll_x = 0;
        float scroll_y = 0;
        if (store.scroll[id].scrollable) {
            scroll_x = store.scroll[id].scroll_x;
            scroll_y = store.scroll[id].scroll_y;
        }

        float child_x, child_y, child_w, child_h;
        if (is_row) {
            child_x = cursor - scroll_x;
            child_y = content_y - scroll_y;
            child_w = main_size;
            child_h = cross_size;
            cursor += main_size + gap; // Add gap
        } else {
            child_x = content_x - scroll_x;
            child_y = cursor - scroll_y;
            child_w = cross_size;
            child_h = main_size;
            cursor += main_size + gap; // Add gap
        }
        
        solve_node(store, child_id, child_x, child_y, child_w, child_h);
    }
    
    // Update Max Scroll
    if (store.scroll[id].scrollable) {
        float content_used = cursor - (is_row ? content_x : content_y);
        // Correct for last gap? No, cursor adds gap after item.
        // If we have children, cursor is past the last item by 'gap'.
        if (!tree.children.empty() && gap > 0) {
            content_used -= gap;
        }
        
        float viewport = is_row ? content_w : content_h;
        float max_scroll = std::max(0.0f, content_used - viewport);
        
        if (is_row) store.scroll[id].max_scroll_x = max_scroll;
        else        store.scroll[id].max_scroll_y = max_scroll;
    }
}

void LayoutEngine::measure_children(NodeStore& store, NodeID id) {
    auto& tree = store.tree[id];
    auto& constraints = store.constraints[id];
    auto& layout = store.layout[id];
    auto& render = store.render[id];

    // Base case: Text measurement
    if (render.is_text && !render.text.empty()) {
        // Simple heuristic: 8px per character if no font engine
        // In a real app, this would call font->measure(render.text)
        layout.w = (float)render.text.length() * 8.0f;
        layout.h = 20.0f; // Default line height
    }

    if (tree.children.empty()) return;

    // Recursively measure children first
    float content_w = 0;
    float content_h = 0;

    bool is_row = (layout.dir == LayoutDir::Row);

    for (NodeID child_id : tree.children) {
        measure_children(store, child_id);
        
        auto& child_c = store.constraints[child_id];
        auto& child_l = store.layout[child_id];

        float cw = (child_c.width >= 0) ? child_c.width : child_l.w;
        float ch = (child_c.height >= 0) ? child_c.height : child_l.h;

        if (is_row) {
            content_w += cw;
            content_h = std::max(content_h, ch);
        } else {
            content_w = std::max(content_w, cw);
            content_h += ch;
        }
    }

    // Wrap with padding
    content_w += constraints.padding * 2;
    content_h += constraints.padding * 2;
    
    // Apply min constraints
    content_w = std::max(content_w, constraints.min_width);
    content_h = std::max(content_h, constraints.min_height);

    // Only update if auto-sized
    if (constraints.width < 0) layout.w = content_w;
    if (constraints.height < 0) layout.h = content_h;
}

} // namespace fanta
