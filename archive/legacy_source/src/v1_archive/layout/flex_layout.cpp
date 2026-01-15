#include "flex_layout.hpp"
#include <algorithm>

namespace Layout {

    // Helper for traversal
    static void compute_sizes(UIContext& ctx, NodeID id);
    static void compute_positions(UIContext& ctx, NodeID id, float x, float y, float w, float h);

    void solve(UIContext& ctx, NodeID root_id, float window_width, float window_height) {
        if (!ctx.node_store.exists(root_id)) return;
        
        NodeHandle root = ctx.get(root_id);
        
        compute_sizes(ctx, root_id);
        
        // Root overrides (Must be after compute_sizes to prevent overwrite)
        root.layout().computed_x = 0;
        root.layout().computed_y = 0;
        root.layout().computed_w = window_width;
        root.layout().computed_h = window_height;

        compute_positions(ctx, root_id, 0, 0, window_width, window_height);
        
        // Phase 13-A: Solve Overlays
        for (NodeID ov : ctx.overlay_list) {
            compute_sizes(ctx, ov);
            NodeHandle oh = ctx.get(ov);
            float ox = 0, oy = 0;
            if (oh.constraint().position_type == 1) { // Absolute
                ox = oh.constraint().x;
                oy = oh.constraint().y;
            }
            compute_positions(ctx, ov, ox, oy, window_width, window_height);
        }
    }


    // Pass 1: Intrinsic Sizes (Bottom-Up)
    static void compute_sizes(UIContext& ctx, NodeID id) {
        NodeHandle n = ctx.get(id);
        if(!n.is_valid()) return;

        // Children first
        for (NodeID child_id : n.tree().children) {
            compute_sizes(ctx, child_id);
        }
        
        // Determine my size based on constraints or children
        float w = 0, h = 0;
        auto& c = n.constraint();
        auto& l = n.layout();
        
        // Fixed size overrides everything
        if (c.width >= 0) w = c.width;
        if (c.height >= 0) h = c.height;
        
        // If Auto, sum children (Excluding Absolute)
        if (c.width < 0 || c.height < 0) {
            float content_w = 0, content_h = 0;
            // LayoutDir 1=Col, 0=Row
            bool is_col = (l.dir == 1); 
            
            for (NodeID child_id : n.tree().children) {
                NodeHandle child = ctx.get(child_id);
                // Ignore Absolute items for size calculation
                if (child.constraint().position_type == 1) continue;

                if (is_col) {
                    content_h += child.layout().computed_h;
                    content_w = std::max(content_w, child.layout().computed_w);
                } else { // Row
                    content_w += child.layout().computed_w;
                    content_h = std::max(content_h, child.layout().computed_h);
                }
            }
            
            // Add padding
            content_w += c.padding * 2;
            content_h += c.padding * 2;
            
            if (c.width < 0) w = content_w;
            if (c.height < 0) h = content_h;
        }
        
        l.computed_w = w;
        l.computed_h = h;
    }
    
    // Pass 2: Positions & Growth (Top-Down)
    static void compute_positions(UIContext& ctx, NodeID id, float x, float y, float w, float h) {
        NodeHandle n = ctx.get(id);
        if(!n.is_valid()) return;
        
        auto& l = n.layout();
        auto& c = n.constraint();
        
        l.computed_x = x;
        l.computed_y = y;
        
        // If we are a flex container, we distribute space to children
        if (n.tree().children.empty()) return;

        bool is_col = (l.dir == 1);
        
        float current_main = is_col ? y : x;
        float ptr_cross = is_col ? x : y;
        float available_w = w - (c.padding * 2); 
        float available_h = h - (c.padding * 2);
        
        // Add padding offset
        current_main += c.padding;
        ptr_cross += c.padding;

        // 1. Calculate space used by fixed items (Exclude Absolute)
        float used_main = 0;
        float total_grow = 0;
        
        for (NodeID child_id : n.tree().children) {
             NodeHandle child = ctx.get(child_id);
             if (child.constraint().position_type == 1) continue;
             
             auto& cl = child.layout();
             float size = is_col ? cl.computed_h : cl.computed_w;
             used_main += size;
             total_grow += child.constraint().grow;
        }
        
        // 2. Distribute remaining space
        float total_space = is_col ? available_h : available_w;
        float remaining = total_space - used_main;
        if (remaining < 0) remaining = 0;
        
        // 3. Layout Children
        float content_size_main = 0.0f;
        auto& scroll = ctx.node_store.scroll[id];
        
        float scroll_off_x = scroll.scroll_x;
        float scroll_off_y = scroll.scroll_y;

        for (NodeID child_id : n.tree().children) {
             NodeHandle child = ctx.get(child_id);
             auto& cc = child.constraint();
             auto& cl = child.layout();
             
             // Absolute Positioning
             if (cc.position_type == 1) {
                 // Absolute children should scroll with container
                 cl.computed_x = x + cc.x - scroll_off_x;
                 cl.computed_y = y + cc.y - scroll_off_y;
                 
                 // Recurse
                 compute_positions(ctx, child_id, cl.computed_x, cl.computed_y, cl.computed_w, cl.computed_h);
                 continue; 
             }
             
             // Relative Flow
             float size_main = is_col ? cl.computed_h : cl.computed_w;
             float size_cross = is_col ? cl.computed_w : cl.computed_h;
             
             // Grow?
             if (total_grow > 0 && cc.grow > 0) {
                 float extra = remaining * (cc.grow / total_grow);
                 size_main += extra;
             }
             
             // Stretch Cross?
             if (cl.align == 3) {
                 size_cross = is_col ? available_w : available_h;
             }
             
             // Commit computed size
             if(is_col) {
                 cl.computed_h = size_main;
                 cl.computed_w = size_cross;
             } else {
                 cl.computed_w = size_main;
                 cl.computed_h = size_cross;
             }
             
             // Recurse
             float child_x = is_col ? ptr_cross : current_main;
             float child_y = is_col ? current_main : ptr_cross;
             
             // Apply Scroll Offset to position
             child_x -= scroll_off_x;
             child_y -= scroll_off_y;

             compute_positions(ctx, child_id, child_x, child_y, cl.computed_w, cl.computed_h);
             
             current_main += size_main;
             content_size_main += size_main;
        }
        
        // Update Scroll State
        if (is_col) {
            float visible_h = available_h; 
            float total_h = content_size_main; 
            if (total_h > visible_h) {
                scroll.max_scroll_y = total_h - visible_h;
                scroll.scrollable = true;
            } else {
                scroll.max_scroll_y = 0;
                scroll.scrollable = (scroll.max_scroll_x > 0); // Keep scrollable if X is scrollable?
                scroll.scroll_y = 0; 
            }
            // What about X scroll in Column? Only if constraints overflow?
            // For now, assume Row = X Scroll, Column = Y Scroll dominant.
            // But we might want X scroll in Column if items correspond to "Wide Content".
            // Let's implement X scroll calculation too.
            // Need to track max width of children?
            // "ptr_cross" logic implies alignment. 
            // This basic flex layout doesn't track cross-axis overflow sum easily without iterating.
            // Let's stick to Main Axis scrolling for now based on Dir.
        } else {
            // Row
            float visible_w = available_w;
            float total_w = content_size_main;
            if (total_w > visible_w) {
                scroll.max_scroll_x = total_w - visible_w;
                scroll.scrollable = true;
            } else {
                scroll.max_scroll_x = 0;
                scroll.scrollable = (scroll.max_scroll_y > 0);
                scroll.scroll_x = 0;
            }
        }
    }

}
