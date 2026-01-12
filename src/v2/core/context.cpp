// Fantasmagorie v2 - UI Context Implementation
#include "context.hpp"

namespace fanta {

void UIContext::begin_frame() {
    // Save previous frame state
    prev_layout = store.layout;
    
    // Clear for new frame
    store.clear();
    
    // Reset state
    while (!node_stack.empty()) node_stack.pop();
    current_id = INVALID_NODE;
    cursor_x = cursor_y = 0;
    input_chars.clear();
    input_keys.clear();
    scroll_delta = 0;
}

void UIContext::end_frame() {
    // Layout solve happens in application
    last_node_w = 0;
    last_node_h = 0;
    mouse_delta = {0, 0};
}

NodeID UIContext::get_id(const char* str_id) {
    uint32_t base = hash_str(str_id);
    
    // Combine with parent stack for unique IDs
    if (!node_stack.empty()) {
        base ^= (uint32_t)node_stack.top() * 16777619u;
    }
    
    return (NodeID)base;
}

NodeID UIContext::begin_node(const char* id) {
    NodeID nid = get_id(id);
    store.add_node(nid);
    
    // Set parent relationship
    if (!node_stack.empty()) {
        NodeID parent = node_stack.top();
        store.tree[nid].parent = parent;
        store.tree[parent].children.push_back(nid);
    } else {
        root_id = nid;
    }
    
    node_stack.push(nid);
    current_id = nid;
    
    return nid;
}

void UIContext::end_node() {
    if (!node_stack.empty()) {
        NodeID id = node_stack.top();
        auto& layout = store.layout[id];
        last_node_w = layout.w;
        last_node_h = layout.h;
        
        node_stack.pop();
        current_id = node_stack.empty() ? INVALID_NODE : node_stack.top();
    }
}

void UIContext::push_transform(const Transform& t) {
    transform_stack.push_back(current_transform);
    current_transform = current_transform * t;
}

void UIContext::pop_transform() {
    if (!transform_stack.empty()) {
        current_transform = transform_stack.back();
        transform_stack.pop_back();
    }
}

float UIContext::animate(NodeID id, int prop, float target, float speed, float dt) {
    auto& state = anims[id][prop];
    if (!state.initialized) {
        state.value = target;
        state.initialized = true;
    }
    
    float diff = target - state.value;
    state.value += diff * speed * dt * 60.0f;  // Normalize to 60fps
    
    return state.value;
}

uint32_t UIContext::animate_color(NodeID id, int prop, uint32_t target, float speed, float dt) {
    // Simple per-channel lerp
    auto lerp_channel = [&](int shift) -> uint8_t {
        float current = (float)((anims[id][prop + shift].initialized ? 
            (uint32_t)anims[id][prop + shift].value : target) >> shift & 0xFF);
        float tgt = (float)((target >> shift) & 0xFF);
        
        float result = current + (tgt - current) * speed * dt * 60.0f;
        anims[id][prop + shift].value = result;
        anims[id][prop + shift].initialized = true;
        
        return (uint8_t)result;
    };
    
    return (lerp_channel(24) << 24) | (lerp_channel(16) << 16) | 
           (lerp_channel(8) << 8) | lerp_channel(0);
}

} // namespace fanta
