#ifndef UI_CONTEXT_HPP
#define UI_CONTEXT_HPP
#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <stack>
#include <unordered_map>
#include <map>
#include "core/node.hpp" 
#include "style.hpp" 

struct Font; // Forward decl

// Simple FNV-1a hash
static uint32_t hash_str(const char* str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 16777619u;
    }
    return hash;
}

struct UIContext {
    NodeID root_id = 0;
    NodeID current_node = 0;
    std::stack<NodeID> node_stack;
    
    // Core Data
    NodeStore node_store;
    
    // Layout State
    float cursor_x = 0;
    float cursor_y = 0;
    
    // Input State
    int mouse_x = 0;
    int mouse_y = 0;
    bool mouse_down = false;
    float mouse_dx = 0;
    float mouse_dy = 0;
    
    // Input Text Support (Phase 5)
    std::vector<unsigned int> input_chars;
    std::vector<int> input_keys;
    
    
    // Style Stack
    std::vector<Style> style_stack;
    
    // Transform Stack (Phase 1)
    Transform current_transform; // Identity by default
    std::vector<Transform> transform_stack;
    
    // Tooltips
    std::string tooltip_text;
    
    // Overlay Layer (Phase 1 Expand)
    std::vector<NodeID> overlay_list; 
    
    // Missed Members Restore
    std::vector<NodeID> open_popups;
    
    // Menus
    NodeID active_menu_id = 0;
    
    // Layout Persistence
    struct PersistentLayout { float x,y,w,h; };
    std::map<NodeID, PersistentLayout> persistent_layout;

    void open_popup(const char* id);

    // Layout/State
    float frame_scroll_delta = 0.0f;
    NodeID last_id = 0;
    std::vector<NodeID> parent_stack_vec;
    
    // Begin/Next
    LayoutConstraints next_layout;
    Style next_style;
    int next_align = -1;
    NodeID current_id = 0;
    
    NodeID begin_node(const char* name);
    
    // Persistence
    std::map<NodeID, float> persistent_scroll;
    float last_node_w = 0;
    float last_node_h = 0;
    
    // Previous Frame State
    std::unordered_map<NodeID, LayoutData> prev_layout;
    std::unordered_map<NodeID, InputState> prev_input;
    std::unordered_map<NodeID, NodeTree> prev_tree;
    std::unordered_map<NodeID, ScrollState> prev_scroll;
    std::unordered_map<NodeID, Transform> prev_transform;
    std::vector<NodeID> prev_nodes;

    UIContext() {
        current_transform = Transform::Identity();
    }
    
    // Core API
    void begin_frame();
    void end_frame();
    
    void begin_node(NodeID id);
    void end_node(); // Auto-layout
    
    // Phase 5: Z-Order
    void bring_to_front(NodeID id);
    
    void push_style(const Style& s);
    void pop_style();

    NodeID get_id(const std::string& str_id);
    
    NodeHandle get(NodeID id) { return {&node_store, id}; }
    
    // Phase 6: Animations
    struct AnimState {
        float f_val = 0;
        uint32_t c_val = 0;
        bool initialized = false;
    };
    using AnimMap = std::map<int, AnimState>;
    std::map<NodeID, AnimMap> anims;
    
    float animate(NodeID id, int prop_id, float target, float speed, float dt);
    uint32_t animate_color(NodeID id, int prop_id, uint32_t target, float speed, float dt);

    // Transform API
    void push_transform(const Transform& t);
    void pop_transform();
    void apply_transform(float& x, float& y) { current_transform.apply(x, y); }
    
    // Phase 2: Font Access
    struct Font* font = nullptr; 
};

#endif

