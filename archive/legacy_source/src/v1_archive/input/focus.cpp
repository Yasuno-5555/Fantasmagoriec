#include "focus.hpp"
#include <algorithm>
#include <vector> 

NodeID FocusManager::hit_test(UIContext& ctx, NodeID root, float x, float y) {
    // Use PREVIOUS FRAME data (Flat list)
    // Reverse iterate to find top-most
    auto& nodes = ctx.prev_nodes;
    NodeID hit_id = 0;
    
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        NodeID id = *it;
            // Audit: Apply Inverse Transform to map Screen Mouse -> Local Space
            float local_x = x;
            float local_y = y;
            if (ctx.prev_transform.count(id)) {
                ctx.prev_transform[id].apply_inverse(local_x, local_y);
            }

            // 1. Basic Rect Test (In Local Space)
            // Note: FlexLayout 'computed_x/y' are usually absolute if not for the Draw Transform.
            // If the node is transformed via PushTransform, the computed coordinates ARE the local bounds.
            if (local_x >= c.computed_x && local_x <= c.computed_x + c.computed_w &&
                local_y >= c.computed_y && local_y <= c.computed_y + c.computed_h) {
                
                // 2. Clipping Test (Ancestor Traversal)
                bool clipped_out = false;
                NodeID curr = id;
                // Climb up
                while (ctx.prev_tree.count(curr) && ctx.prev_tree[curr].parent != 0) {
                     curr = ctx.prev_tree[curr].parent;
                     
                     // Check if ancestor clips content
                     if (ctx.prev_scroll.count(curr) && ctx.prev_scroll[curr].clip_content) {
                         if (ctx.prev_layout.count(curr)) {
                             auto& pc = ctx.prev_layout[curr];
                             // Check intersection
                             if (x < pc.computed_x || x > pc.computed_x + pc.computed_w ||
                                 y < pc.computed_y || y > pc.computed_y + pc.computed_h) {
                                 clipped_out = true;
                                 break;
                             }
                         }
                     }
                }
                
                if (clipped_out) continue; // Skip this node
                
                // Check input state
                if (ctx.prev_input[id].clickable) { // Only clickable for now
                     return id;
                }
            }
        }
    }
    return 0;
}

void FocusManager::update(UIContext& ctx, float mx, float my, bool mouse_down, float scroll_delta) {
    drag_dx = mx - last_mx;
    drag_dy = my - last_my;
    last_mx = mx;
    last_my = my;

    hover_id = hit_test(ctx, 0, mx, my); // Sets hover_id

    // Scroll Logic
    if (scroll_delta != 0.0f && hover_id != 0) {
        NodeID curr = hover_id;
        while (curr != 0) {
            if (ctx.node_store.scroll.count(curr)) { // Check if scroll data exists for curr
                auto& s = ctx.node_store.scroll[curr];
                if (s.scrollable) {
                    s.scroll_y -= scroll_delta * 30.0f; // Speed
                    if (s.scroll_y < 0) s.scroll_y = 0;
                    if (s.scroll_y > s.max_scroll_y) s.scroll_y = s.max_scroll_y;
                    
                    // Audit: Persist immediately so it survives the next frame's clear
                    ctx.persistent_scroll[curr] = s.scroll_y;
                    break; // Handled
                }
            }
            if (ctx.node_store.tree.count(curr)) { // Check if tree data exists for curr
                curr = ctx.node_store.tree[curr].parent;
            } else {
                break; // No parent, stop
            }
        }
    }
    
    bool was_clicked = mouse_down && !mouse_was_down;
    clicked_id = 0; // Reset
    
    if (was_clicked) {
        if (hover_id != 0) {
             focus_id = hover_id; // Simple focus-follows-click
             clicked_id = hover_id;
        } else {
            focus_id = 0;
        }
    }
    mouse_was_down = mouse_down;
}

void FocusManager::build_focus_list(UIContext& ctx, std::vector<NodeID>& list) {
    // Iterate prev_nodes (Draw Order)
    // This usually matches logical definition order for immediate mode.
    for(NodeID id : ctx.prev_nodes) {
        if(ctx.prev_input.count(id) && ctx.prev_input[id].focusable) {
            list.push_back(id);
        }
    }
}

void FocusManager::tab_next(UIContext& ctx) {
    std::vector<NodeID> list;
    build_focus_list(ctx, list);
    if(list.empty()) return;
    
    if(focus_id == 0) {
        focus_id = list[0];
        return;
    }
    
    for(size_t i=0; i<list.size(); ++i) {
        if(list[i] == focus_id) {
            // Found current
            if(i + 1 < list.size()) focus_id = list[i+1];
            else focus_id = list[0]; // Wrap
            return;
        }
    }
    
    // Current focus not in list (maybe destroyed), reset
    focus_id = list[0];
}

void FocusManager::tab_prev(UIContext& ctx) {
    std::vector<NodeID> list;
    build_focus_list(ctx, list);
    if(list.empty()) return;

    if(focus_id == 0) {
        focus_id = list.back();
        return;
    }
    
    for(size_t i=0; i<list.size(); ++i) {
        if(list[i] == focus_id) {
            if(i > 0) focus_id = list[i-1];
            else focus_id = list.back(); // Wrap
            return;
        }
    }
    focus_id = list.back();
}
