// Fantasmagorie v2 - UI Context
// Central state management for the UI framework
#pragma once

#include "node.hpp"
#include <stack>
#include <map>

namespace fanta {

// Forward declarations
struct Font;
struct Style;

// ============================================================================
// UI Context
// ============================================================================

class UIContext {
public:
    // Node Management
    NodeStore store;
    NodeID root_id = INVALID_NODE;
    NodeID current_id = INVALID_NODE;
    std::stack<NodeID> node_stack;
    
    // Layout Cursor
    float cursor_x = 0, cursor_y = 0;
    
    // Input State
    Vec2 mouse_pos;
    Vec2 mouse_delta;
    bool mouse_down = false;
    bool mouse_clicked = false;  // True on frame mouse went down
    bool mouse_released = false; // True on frame mouse went up
    
    NodeID hot_id = INVALID_NODE;    // Node under mouse
    NodeID active_id = INVALID_NODE; // Node currently capturing input
    NodeID focused_id = INVALID_NODE; // Node receiving keyboard input
    bool prev_mouse_down = false;
    
    float scroll_delta = 0;
    std::vector<unsigned int> input_chars;
    std::vector<int> input_keys;
    
    // Transform Stack
    Transform current_transform;
    std::vector<Transform> transform_stack;
    
    // Previous Frame (for animations/persistence)
    std::map<NodeID, LayoutData> prev_layout;
    std::vector<NodeID> prev_nodes; // Creation order from last frame
    
    // Animation State
    struct AnimState { float value = 0; bool initialized = false; };
    std::map<NodeID, std::map<int, AnimState>> anims;
    
    // Font
    Font* font = nullptr;

    // Debug State
    NodeID debug_hover_id = INVALID_NODE; // Node currently hovered in Inspector
    bool show_layout_bounds = false;      // Toggle for layout visualization
    
    // ========================================================================
    // API
    // ========================================================================
    
    UIContext() {
        current_transform = Transform::Identity();
    }
    
    void begin_frame();
    void end_frame();
    
    // Process implicit animations
    void update_visuals(float dt);
    
    NodeID begin_node(const char* id);
    void end_node();
    
    NodeHandle get(NodeID id) { return NodeHandle(&store, id); }
    
    // ID Generation
    NodeID get_id(const char* str_id);
    
    // Transform
    void push_transform(const Transform& t);
    void pop_transform();
    void apply_transform(float& x, float& y) {
        current_transform.apply(x, y);
    }
    
    // Animation
    float animate(NodeID id, int prop, float target, float speed, float dt);
    uint32_t animate_color(NodeID id, int prop, uint32_t target, float speed, float dt);
    
    // Last node size (for connections etc)
    float last_node_w = 0, last_node_h = 0;
};

// ============================================================================
// Hash Function
// ============================================================================

inline uint32_t hash_str(const char* str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 16777619u;
    }
    return hash;
}

} // namespace fanta
