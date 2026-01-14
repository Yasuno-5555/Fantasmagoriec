#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "animation.hpp" // Phase 6.2

// Internal Core Types. Hidden from user.
// Included only by fanta.cpp and backend implementations.

namespace fanta {
namespace internal {

    using ID = uint64_t;

    struct Vec2 { float x=0, y=0; };
    struct Rectangle { float x=0, y=0, w=0, h=0; };
    struct ColorF { float r=0, g=0, b=0, a=1; };

    enum class LayoutMode { Column, Row };
    enum class Align { Start, Center, End, Stretch };
    enum class Justify { Start, Center, End, SpaceBetween, SpaceAround };

    struct Theme {
        ColorF background;
        ColorF surface;
        ColorF primary;
        ColorF accent;
        ColorF text;
        ColorF text_muted;
        ColorF border;
    };

    // The "Real" properties of an element
    struct ElementState {
        ID id;
        std::string label;
        
        // Layout
        float w=0, h=0; // 0 = auto
        float p=0; // padding
        LayoutMode layout_mode = LayoutMode::Column; // Row or Column
        float gap = 0; // Gap between children
        Align align = Align::Start; // Cross-axis alignment
        Justify justify = Justify::Start; // Main-axis justification
        
        // Style
        ColorF bg_color = {0,0,0,0};
        ColorF text_color = {1,1,1,1};
        float font_size = 16.0f; // Default size 16px
        float corner_radius = 0;
        
        // Effects
        float blur_radius = 0;
        bool backdrop_blur = false;
        
        // Depth
        float elevation = 0;
        
        // Primitive
        bool is_line = false;
        
        // Phase 7: Node Graph
        bool is_canvas = false;
        Vec2 canvas_pan = {0,0};
        float canvas_zoom = 1.0f;
        
        bool is_node = false;
        Vec2 node_pos = {0,0};
        
        bool is_wire = false;
        Vec2 wire_p0={0,0}, wire_p1={0,0}, wire_p2={0,0}, wire_p3={0,0};
        float wire_thickness = 1.0f;
        
        // Phase 8: Advanced Widgets
        bool is_scrollable_v = false;
        bool is_scrollable_h = false;
        Vec2 scroll_pos = {0,0};
        bool is_popup = false;
        
        // Tree
        ID parent = 0;
        std::vector<ID> children;
    };

    // Per-Element Persistent State (retained across frames)
    struct PersistentData {
        AnimState elevation;
        AnimState scale;
        AnimState scroll_y; // Phase 8
        AnimState scroll_x; // Phase 8
    };

    // The Global Store (Flat Data)
    struct StateStore {
        std::vector<ElementState> elements;
        std::vector<ElementState> frame_elements;
        
        // Phase 6.2: Persistent Animation State
        std::map<ID, PersistentData> persistent_states;

        void clear_frame() { frame_elements.clear(); }
    };

} // namespace internal
} // namespace fanta
