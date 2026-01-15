// Fantasmagorie v2 - UI Context Implementation
#include "core/context.hpp"
#include "style/theme.hpp" // For current_theme()
#include <cmath>
#include <algorithm>

namespace fanta {

void UIContext::begin_frame() {
    scroll_delta = 0;
    
    // Process Scroll (Bubbling)
    if (this->scroll_delta != 0 && hot_id != INVALID_NODE) {
        NodeID curr = hot_id;
        while (curr != INVALID_NODE) {
            if (store.exists(curr) && store.scroll[curr].scrollable) {
                // Apply scroll
                auto& s = store.scroll[curr];
                s.scroll_y -= this->scroll_delta * 20.0f; // Scale factor
                
                // Clamp
                if (s.scroll_y < 0) s.scroll_y = 0;
                if (s.scroll_y > s.max_scroll_y) s.scroll_y = s.max_scroll_y;
                
                // Consumed
                break; 
            }
            
            // Go up
             if (store.tree.find(curr) != store.tree.end()) {
                 curr = store.tree[curr].parent;
             } else {
                 break;
             }
        }
    }
    
    // Process Tab Navigation
    // We usage prev_nodes because current nodes are not yet built
    if (!prev_nodes.empty()) {
        for (int key : input_keys) {
            if (key == 258) { // GLFW_KEY_TAB
                // TODO: Check modifiers for Shift+Tab
                // For now just forward
                bool shift = false; 
                
                int start_idx = -1;
                for (size_t i = 0; i < prev_nodes.size(); ++i) {
                    if (prev_nodes[i] == focused_id) {
                        start_idx = (int)i;
                        break;
                    }
                }
                
                // Find next focusable
                NodeID next_focus = INVALID_NODE;
                int count = (int)prev_nodes.size();
                int offset = shift ? -1 : 1;
                
                for (int i = 1; i < count; ++i) {
                    int idx = (start_idx + i * offset + count) % count;
                    NodeID id = prev_nodes[idx];
                    
                    // Check if focusable (from persistent input state)
                    if (store.input[id].focusable && !store.input[id].disabled) {
                        next_focus = id;
                        break;
                    }
                }
                
                if (next_focus != INVALID_NODE) {
                    focused_id = next_focus;
                }
            }
        }
    }
    
    // Save previous frame state
    prev_layout = store.layout;
    prev_nodes = store.nodes;
    
    // Clear for new frame
    store.clear();
    
    // Reset state
    while (!node_stack.empty()) node_stack.pop();
    current_id = INVALID_NODE;
    cursor_x = cursor_y = 0;
    
    // Process Inputs (simplified)
    if (mouse_down && !prev_mouse_down) mouse_clicked = true;
    else mouse_clicked = false;
    
    if (!mouse_down && prev_mouse_down) mouse_released = true;
    else mouse_released = false;
    
    prev_mouse_down = mouse_down;
    
    hot_id = INVALID_NODE;
}

void UIContext::end_frame() {
    // ...
}

NodeID UIContext::begin_node(const char* id_str) {
    NodeID id = get_id(id_str);
    store.add_node(id);
    
    // Hierarchy
    if (current_id != INVALID_NODE) {
        store.tree[current_id].children.push_back(id);
        store.tree[id].parent = current_id;
    } else {
        if (root_id == INVALID_NODE) root_id = id;
    }
    
    node_stack.push(id);
    current_id = id;
    
    return id;
}

void UIContext::end_node() {
    if (!node_stack.empty()) {
        node_stack.pop();
        if (!node_stack.empty()) current_id = node_stack.top();
        else current_id = INVALID_NODE;
    }
}

NodeID UIContext::get_id(const char* str_id) {
    // Simple hashing for now
    // In production, mix with parent ID to avoid collisions
    uint32_t hash = hash_str(str_id);
    if (!node_stack.empty()) {
        // Mix with parent
        hash ^= (uint32_t)node_stack.top(); 
        hash *= 16777619u;
    }
    return (NodeID)hash;
}

void UIContext::update_visuals(float dt) {
    auto theme = current_theme();
    
    for (auto& [id, vs] : store.visual_state) {
        if (!vs.initialized) continue;
        
        // Determine state
        int state = 0; // Default
        const auto& input = store.input[id];
        
        if (input.disabled) {
            state = 4; // Disabled
        } else if (active_id == id) {
            state = 2; // Active
        } else if (focused_id == id) {
            state = 3; // Focused (New! Priority over Hover)
        } else if (hot_id == id) {
            state = 1; // Hover
        }
        
        vs.current_interaction_state = state;
        const ResolvedStyle& target = vs.get_target(state);
        ResolvedStyle& current = vs.current;
        
        // Asymmetric Animation / Duration Selection
        float dur = theme->motion.duration_base;
        
        // Active state should be punchy (Fast)
        if (state == 2) { 
            dur = theme->motion.duration_fast;
        } 
        // Hover/Default use base duration (Smooth)
        else {
            dur = theme->motion.duration_base;
        }
        
        if (dur <= 0.001f) dur = 0.001f;
        
        // Asymptotic smoothing speed
        // To reach 99% in 'dur' with exp decay: 1 - exp(-speed * dur) = 0.99
        // -speed * dur = ln(0.01) = -4.6
        // speed = 4.6 / dur;
        float speed = 4.6f / dur;
        float alpha = 1.0f - std::exp(-speed * dt);
        if (alpha > 1.0f) alpha = 1.0f;
        
        // Lerp Colors (v3 types)
        current.bg = Color::lerp(current.bg, target.bg, alpha);
        current.text = Color::lerp(current.text, target.text, alpha);
        current.border_color = Color::lerp(current.border_color, target.border_color, alpha);
        current.focus_ring_color = Color::lerp(current.focus_ring_color, target.focus_ring_color, alpha);
        
        // Lerp Floats helper
        auto lerp_f = [&](float& cur, float tgt) {
            cur += (tgt - cur) * alpha;
        };
        
        // Lerp CornerRadius (lerp members)
        lerp_f(current.radius.tl, target.radius.tl);
        lerp_f(current.radius.tr, target.radius.tr);
        lerp_f(current.radius.br, target.radius.br);
        lerp_f(current.radius.bl, target.radius.bl);

        // Lerp ElevationShadow
        auto lerp_layer = [&](ShadowLayer& cur, const ShadowLayer& tgt) {
            cur.color = Color::lerp(cur.color, tgt.color, alpha);
            lerp_f(cur.blur, tgt.blur);
            lerp_f(cur.spread, tgt.spread);
            lerp_f(cur.offset.x, tgt.offset.x);
            lerp_f(cur.offset.y, tgt.offset.y);
        };
        lerp_layer(current.shadow.ambient, target.shadow.ambient);
        lerp_layer(current.shadow.key, target.shadow.key);

        lerp_f(current.border_width, target.border_width);
        lerp_f(current.focus_ring_width, target.focus_ring_width);
        lerp_f(current.scale, target.scale);
        
        // Booleans / Enums (Immediate switch)
        current.show_focus_ring = target.show_focus_ring;
        current.cursor = target.cursor;
    }
}

} // namespace fanta


