#pragma once
#include "draw_types.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

// Forward declarations
struct UIContext;

// IDs
using NodeID = uint64_t;

// --- Node Components ---

struct LayoutConstraints {
    // Layout properties (set by API)
    float width = -1.0f;     // -1 (or <0) = Auto/Shrink
    float height = -1.0f;
    float grow = 0.0f;      
    float shrink = 1.0f;    
    float padding = 0.0f;   // Uniform padding
    
    // Positioning (Phase 13-A)
    int position_type = 0; // 0=Relative, 1=Absolute
    float x = 0.0f;        // Used if Absolute
    float y = 0.0f;
};


// Renamed from NodeLayout to avoid confusion with the component group
struct LayoutData { 
    // Configuration
    int dir = 1; // 0=Row, 1=Column (Enum is LayoutDir in header? Let's keep Enum)
    int justify = 0; // LayoutAlign::Start
    int align = 3;   // LayoutAlign::Stretch
    
    // Computed layout implementation details (Layout Pass results)
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

struct ResolvedStyle {
    // Visual properties (computed or set directly)
    int cursor_type = 0; // 0=Arrow
    
    uint32_t bg_color = 0x00000000; // RGBA8
    uint32_t bg_hover = 0x00000000;
    uint32_t bg_active = 0x00000000;
    
    uint32_t text_color = 0xFFFFFFFF;
    
    float corner_radius = 0.0f;
    float border_width = 0.0f;
    uint32_t border_color = 0x00000000;
    
    // Focus
    bool show_focus_ring = false;
    float focus_ring_width = 0.0f;
    uint32_t focus_ring_color = 0x00000000;
    
    // Shadow
    uint32_t shadow_color = 0;
    float shadow_offset_x = 0;
    float shadow_offset_y = 0;
    float shadow_radius = 0;
};

struct InputState {
    bool hoverable = false;
    bool clickable = false;
    bool focusable = false;
    bool disabled = false; // Skip if true
    
    // Callbacks
    std::function<void()> on_click;
    std::function<bool(const std::string&)> on_text_input; // Return true if consumed
    std::function<std::string()> get_text; // For Copy operations
    std::function<bool(int key, int action, int mods)> on_key; // Return true if consumed
};

struct RenderData {
    std::string text_content; // If text node
    bool is_text = false;
    
    // Phase Visuals: Image Support
    void* texture_id = nullptr;
    float u0=0, v0=0, u1=1, v1=1;
    
    bool is_image = false;
    void* custom_texture_id = nullptr;

#include <deque>

// ...

    // Phase 1: Custom Draw Commands
    std::vector<DrawCmd> custom_draw_cmds;

    // Phase 6: Stable Text Storage for Markdown
    std::deque<std::string> text_segments;
};

struct ScrollState {
    float scroll_y = 0.0f;
    float max_scroll_y = 0.0f;
    
    // Phase 1: Horizontal Scroll
    float scroll_x = 0.0f;
    float max_scroll_x = 0.0f;
    
    bool scrollable = false;
    bool clip_content = false; 
};

struct NodeTree {
    NodeID parent = 0;
    std::vector<NodeID> children;
};

// --- Enums ---
enum class LayoutDir { Row, Column };
enum class LayoutAlign { Start, Center, End, Stretch };

// --- Node Store (SoA) ---
struct NodeStore {
    std::vector<NodeID> nodes; // Order of creation, for iteration if needed
    
    std::unordered_map<NodeID, NodeTree> tree;
    std::unordered_map<NodeID, LayoutConstraints> constraints;
    std::unordered_map<NodeID, LayoutData> layout;
    std::unordered_map<NodeID, ResolvedStyle> style;
    std::unordered_map<NodeID, InputState> input;
    std::unordered_map<NodeID, RenderData> render;
    std::unordered_map<NodeID, ScrollState> scroll;
    std::unordered_map<NodeID, Transform> transform; // Phase 1: Transform Storage
    
    void clear() {
        nodes.clear();
        tree.clear();
        constraints.clear();
        layout.clear();
        style.clear();
        input.clear();
        render.clear();
        scroll.clear();
        transform.clear();
    }
    
    void add_node(NodeID id) {
        if (tree.find(id) == tree.end()) {
            nodes.push_back(id);
            tree[id] = NodeTree();
            constraints[id] = LayoutConstraints();
            layout[id] = LayoutData();
            style[id] = ResolvedStyle();
            input[id] = InputState();
            render[id] = RenderData();
            scroll[id] = ScrollState();
        }
    }
    
    // Legacy helper for checking existence (optional)
    bool exists(NodeID id) const {
        return tree.find(id) != tree.end();
    }
};

// --- Node Handle (API Helper) ---
// Transient handle to access logic. VALID ONLY WITHIN FRAME.
struct NodeHandle {
    NodeStore* store;
    NodeID id;
    
    bool is_valid() const { return store && store->exists(id); }
    
    NodeTree& tree() { return store->tree[id]; }
    LayoutConstraints& constraint() { return store->constraints[id]; }
    LayoutData& layout() { return store->layout[id]; }
    ResolvedStyle& style() { return store->style[id]; }
    InputState& input() { return store->input[id]; }
    RenderData& render() { return store->render[id]; }
    ScrollState& scroll() { return store->scroll[id]; }
};
