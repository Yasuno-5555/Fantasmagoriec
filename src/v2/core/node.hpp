// Fantasmagorie v2 - Node Store (SoA Architecture)
// "Node is Dead" - Components stored separately for cache efficiency
#pragma once

#include "types.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace fanta {

// ============================================================================
// Node Components (Stored Separately)
// ============================================================================

// Tree Structure
struct NodeTree {
    NodeID parent = INVALID_NODE;
    std::vector<NodeID> children;
};

// Layout Constraints (Input to Layout System)
struct LayoutConstraints {
    float width = -1.0f;   // -1 = Auto
    float height = -1.0f;
    float grow = 0.0f;
    float shrink = 1.0f;
    float padding = 0.0f;
    
    // Positioning
    enum class Position { Relative, Absolute };
    Position position_type = Position::Relative;
    float pos_x = 0.0f;
    float pos_y = 0.0f;
};

// Layout Result (Output from Layout System)
struct LayoutData {
    LayoutDir dir = LayoutDir::Column;
    Align justify = Align::Start;
    Align align = Align::Stretch;
    
    // Computed geometry
    float x = 0, y = 0, w = 0, h = 0;
};

// Visual Style
struct ResolvedStyle {
    Color bg = Color::Transparent();
    Color bg_hover = Color::Transparent();
    Color bg_active = Color::Transparent();
    Color text = Color::White();
    Color border = Color::Transparent();
    
    float corner_radius = 0.0f;
    float border_width = 0.0f;
    
    // Focus
    bool show_focus_ring = false;
    Color focus_ring_color = Color::Hex(0x4A90D9FF);
    float focus_ring_width = 2.0f;
    
    // Shadow (Optional)
    Color shadow = Color::Transparent();
    float shadow_offset_x = 0, shadow_offset_y = 0;
    float shadow_radius = 0;
    
    CursorType cursor = CursorType::Arrow;
};

// Input Handling
struct InputState {
    bool hoverable = false;
    bool clickable = false;
    bool focusable = false;
    bool disabled = false;
    
    std::function<void()> on_click;
    std::function<bool(int key, int action, int mods)> on_key;
};

// Scroll State
struct ScrollState {
    float scroll_x = 0, scroll_y = 0;
    float max_scroll_x = 0, max_scroll_y = 0;
    bool scrollable = false;
    bool clip_content = false;
};

// Render Data
struct RenderData {
    std::string text;
    bool is_text = false;
    bool is_image = false;
    void* texture = nullptr;
    float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
};

// ============================================================================
// Node Store (Structure of Arrays)
// ============================================================================

class NodeStore {
public:
    std::vector<NodeID> nodes;  // Creation order
    
    std::unordered_map<NodeID, NodeTree> tree;
    std::unordered_map<NodeID, LayoutConstraints> constraints;
    std::unordered_map<NodeID, LayoutData> layout;
    std::unordered_map<NodeID, ResolvedStyle> style;
    std::unordered_map<NodeID, InputState> input;
    std::unordered_map<NodeID, ScrollState> scroll;
    std::unordered_map<NodeID, RenderData> render;
    std::unordered_map<NodeID, Transform> transform;
    
    void clear() {
        nodes.clear();
        tree.clear();
        constraints.clear();
        layout.clear();
        style.clear();
        input.clear();
        scroll.clear();
        render.clear();
        transform.clear();
    }
    
    void add_node(NodeID id) {
        if (tree.find(id) == tree.end()) {
            nodes.push_back(id);
            tree[id] = {};
            constraints[id] = {};
            layout[id] = {};
            style[id] = {};
            input[id] = {};
            scroll[id] = {};
            render[id] = {};
        }
    }
    
    bool exists(NodeID id) const {
        return tree.find(id) != tree.end();
    }
};

// ============================================================================
// Node Handle (Ergonomic Accessor)
// ============================================================================

class NodeHandle {
    NodeStore* store_;
    NodeID id_;
    
public:
    NodeHandle(NodeStore* s, NodeID id) : store_(s), id_(id) {}
    
    bool valid() const { return store_ && store_->exists(id_); }
    NodeID id() const { return id_; }
    
    NodeTree& tree() { return store_->tree[id_]; }
    LayoutConstraints& constraint() { return store_->constraints[id_]; }
    LayoutData& layout() { return store_->layout[id_]; }
    ResolvedStyle& style() { return store_->style[id_]; }
    InputState& input() { return store_->input[id_]; }
    ScrollState& scroll() { return store_->scroll[id_]; }
    RenderData& render() { return store_->render[id_]; }
};

} // namespace fanta
