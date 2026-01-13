// Fantasmagorie v2 - Node Store (SoA Architecture)
// "Node is Dead" - Components stored separately for cache efficiency
#pragma once

#include "types.hpp"
#include "types.hpp"
#include <vector>
#include <string>
#include <map>

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
    float min_width = 0.0f;
    float min_height = 0.0f;
    float grow = 0.0f;
    float shrink = 1.0f;
    float padding = 0.0f;
    float gap = 0.0f;      // Space between children
    
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

// Visual Primitives
struct Shadow {
    Color color = Color::Transparent();
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float blur_radius = 0.0f; // Gaussian blur radius
    float spread = 0.0f;      // Expansion of the shadow rect
};

struct Border {
    Color color = Color::Transparent();
    Color color_light = Color::Transparent(); // Top/Left (if alpha=0, uses color)
    Color color_dark = Color::Transparent();  // Bottom/Right (if alpha=0, uses color)
    float width = 0.0f;
};

enum class Easing {
    Linear,
    EaseOut,   // Cubic Out
    EaseInOut  // Cubic InOut
};

struct AnimationConfig {
    float duration = 0.12f; // Seconds
    Easing easing = Easing::EaseOut;
};

enum class InteractionState {
    Default = 0,
    Hover = 1,
    Active = 2,
    Focused = 3,
    Disabled = 4
};

// Visual Style
struct ResolvedStyle {
    Color bg = Color::Transparent();
    Color bg_hover = Color::Transparent(); // Deprecated: implicit animations prefer explicit state overrides
    Color bg_active = Color::Transparent(); // Deprecated
    Color text = Color::White();
    
    Border border;
    Shadow shadow;
    
    float corner_radius = 0.0f;
    float scale = 1.0f; // Visual scale transformation
    
    // Focus
    bool show_focus_ring = false;
    Color focus_ring_color = Color::Hex(0x4A90D9FF);
    float focus_ring_width = 2.0f;
    
    CursorType cursor = CursorType::Arrow;
    
    // Animation settings for transitions TO this style
    AnimationConfig animation;
};

// Implicit Animation Target
struct VisualState {
    ResolvedStyle current; // Tweened value shown on screen
    
    // Target styles for states
    // 0: Default, 1: Hover, 2: Active, 3: Disabled, 4: Focused
    // Target styles for states
    // 0: Default, 1: Hover, 2: Active, 3: Disabled, 4: Focused
    std::map<int, ResolvedStyle> targets;
    
    int current_interaction_state = 0; // The state we are currently animating TOWARDS
    
    const ResolvedStyle& get_target(int state) const {
        auto it = targets.find(state);
        if (it != targets.end()) return it->second;
        // Fallback to default (state 0), or create default from current if not found?
        // It's expected that state 0 is always populated by the builder base
        auto def = targets.find(0);
        if (def != targets.end()) return def->second;
        
        static ResolvedStyle empty;
        return empty;
    }
    
    bool initialized = false;
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
    
    // Icon
    bool is_icon = false;
    IconType icon = IconType::Check;
    uint32_t icon_color = 0xFFFFFFFF;
};

// ============================================================================
// Node Store (Structure of Arrays)
// ============================================================================

class NodeStore {
public:
    std::vector<NodeID> nodes;  // Creation order
    
    std::map<NodeID, NodeTree> tree;
    std::map<NodeID, LayoutConstraints> constraints;
    std::map<NodeID, LayoutData> layout;
    std::map<NodeID, ResolvedStyle> style;      // The "Target" style for the current frame/state
    std::map<NodeID, VisualState> visual_state; // The "Tweened" style (implicit animation)
    std::map<NodeID, InputState> input;
    std::map<NodeID, ScrollState> scroll;
    std::map<NodeID, RenderData> render;
    std::map<NodeID, Transform> transform;
    
    void clear() {
        nodes.clear();
        tree.clear();
        constraints.clear();
        layout.clear();
        style.clear();
        // visual_state is persistent, do NOT clear it!
        input.clear();
        // scroll is persistent
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
            // visual_state[id] = {}; // Created on demand or preserved
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
