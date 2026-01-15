// Fantasmagorie v3 - Context Implementation (Clean)
#include "core/context.hpp"
#include <cstdio>

namespace fanta {

void UIContext::begin_frame() {
    // Reset per-frame state
    node_stack = std::stack<NodeID>();
    
    // Previous frame layout becomes current for first pass? 
    // Usually we swap or clear. For now, simplistic.
    mouse_clicked = mouse_down && !prev_mouse_down;
    mouse_released = !mouse_down && prev_mouse_down;
    
    // Reset usage headers for new frame
    store.nodes.clear();
    // In a real immediate mode, we might clear lists or keep them and reuse IDs.
    // For V3 clean port, let's assume simple clear-and-rebuild.
    // Note: This wipes state if we aren't careful. 
    // True retained-mode mapping requires more complex ID matching.
    // We'll trust the headers are defining the storage.
}

void UIContext::end_frame() {
    prev_mouse_down = mouse_down;
    // Cleanup
}

NodeID UIContext::begin_node(const char* name) {
    // Simple ID generation (hash)
    NodeID id = hash_str(name); 
    // In real app, mix with parent ID to avoid collision
    
    store.nodes.push_back(id);
    node_stack.push(id);
    return id;
}

void UIContext::end_node() {
    if(!node_stack.empty()) node_stack.pop();
}

NodeID UIContext::get_id(const char* str_id) {
    return hash_str(str_id);
}


// Layout placeholder
void UIContext::solve_layout(int w, int h) {
    // Basic Layout Engine
    // Iterate root nodes, apply constraints, compute positions
    // This requires the LayoutEngine logic.
    // For now, stub to allow link.
}

} // namespace fanta
