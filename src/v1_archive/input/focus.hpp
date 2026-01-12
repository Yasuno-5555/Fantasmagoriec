#pragma once
#include "../core/node.hpp"
#include "../core/ui_context.hpp"

struct FocusManager {
    // State
    NodeID hover_id = 0;
    NodeID active_id = 0;
    NodeID focus_id = 0;
    NodeID clicked_id = 0; // The node clicked THIS frame
    bool mouse_was_down = false;
    
    // Phase 2: Delta Tracking
    float last_mx = 0.0f;
    float last_my = 0.0f;
    float drag_dx = 0.0f;
    float drag_dy = 0.0f;
    
    // Hit test: find topmost node containing point
    NodeID hit_test(UIContext& ctx, NodeID root, float x, float y);
    
    // Core
    void update(UIContext& ctx, float mx, float my, bool mouse_down, float scroll_y = 0.0f);
    // Alias for API compatibility
    void update_input_state(float mx, float my, bool mdown) {
         // Requires context? API passes mx,my,mdown. 
         // But update takes ctx!
         // We can't implement it here without ctx reference.
         // Removing alias. Update api.cpp instead.
    }
    
    // Helpers
    bool is_hovered(NodeID id) const { return hover_id == id; }
    bool is_active(NodeID id) const { return active_id == id; }
    bool is_focused(NodeID id) const { return focus_id == id; }
    bool is_clicked(NodeID id) const { return clicked_id == id; }
    
    void register_node(UIContext*, NodeID) { /* no-op for now */ }
    
    void set_focus(NodeID id) { focus_id = id; }
    void clear_focus() { focus_id = 0; }
    
    // Navigation
    void tab_next(UIContext& ctx);
    void tab_prev(UIContext& ctx);
    
    // Helpers
    void build_focus_list(UIContext& ctx, std::vector<NodeID>& list);
};
