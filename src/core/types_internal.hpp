#pragma once
#include "fanta.h"
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <optional>
#include <array>
#include "animation.hpp" 
#include "spring_animation.hpp"

// Internal Core Types. Hidden from user.
// Included only by fanta.cpp and backend implementations.

namespace fanta {
namespace internal {

    using ID = fanta::ID;

    // FNV-1a Hash (Legacy/Helper - maybe remove if ID constructor covers it?)
    // Converting to ID type directly.
    inline ID Hash(const char* str) { return ID(str); }

    struct Vec2 {
        float x=0, y=0;
        Vec2 operator*(float f) const { return {x*f, y*f}; }
        Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
        bool operator==(const Vec2& o) const { return x==o.x && y==o.y; }
    };
    struct Rectangle { float x=0, y=0, w=0, h=0; };
    struct ColorF {
        float r=0, g=0, b=0, a=1.0f; // Linear float for HDR support (Phase 25)

        bool operator==(const ColorF& other) const { return r==other.r && g==other.g && b==other.b && a==other.a; }
        ColorF operator*(float t) const { return {r*t, g*t, b*t, a*t}; }
        ColorF operator+(const ColorF& o) const { return {r+o.r, g+o.g, b+o.b, a+o.a}; }
    };

    // Phase 25: High-Precision Paths
    enum class PathVerb { MoveTo, LineTo, QuadTo, CubicTo, Close };
    struct PathPoint {
        PathVerb verb;
        Vec2 p0, p1, p2;
    };
    struct Path {
        std::vector<PathPoint> points;
        float stroke_width = 1.0f;
        bool is_fill = false;
    };

    // Forward declarations for widget states
    struct TextInputState;
    struct TreeNodeState;



    using ::fanta::LayoutMode;
    using ::fanta::Align;
    using ::fanta::Justify;
    using ::fanta::Wrap;
    using ::fanta::AccessibilityRole;

    // Mirroring public API token
    // V5 Refactor: Use public API token
    // Mirroring public API token
    enum class ColorToken : uint8_t {
        None = 0,
        SystemBackground, SecondarySystemBackground, TertiarySystemBackground,
        Label, SecondaryLabel, TertiaryLabel, QuaternaryLabel,
        Fill, SecondaryFill, Separator, Accent, Destructive,
        Count
    };

    // Phase 12.7: Typography Tokens (HIG)
    // V5 Refactor: Use public API token
    // Phase 12.7: Typography Tokens (HIG)
    enum class TextToken : uint8_t {
        None = 0,
        LargeTitle, Title1, Title2, Title3,
        Headline, Body, Callout, Subhead, Footnote, 
        Caption1, Caption2,
        Count
    };

    // Phase 40: God-level Typography
    struct TypographyRule {
        float size = 13.0f;
        float weight = 400.0f;
        float tracking = 0.0f;
        float line_height = 1.2f;
    };

    struct Theme {
        // Internal theme stores floats for fast rendering
        std::array<ColorF, static_cast<size_t>(ColorToken::Count)> palette;
        std::array<TypographyRule, static_cast<size_t>(TextToken::Count)> typography; 
        
        ColorF Resolve(uint32_t rgba, ColorToken token, bool is_semantic) const {
            if (is_semantic) {
                return palette[static_cast<size_t>(token)];
            }
            // Convert RGBA (uint32) to ColorF
            float a = (rgba & 0xFF) / 255.0f;
            float b = ((rgba >> 8) & 0xFF) / 255.0f;
            float g = ((rgba >> 16) & 0xFF) / 255.0f;
            float r = ((rgba >> 24) & 0xFF) / 255.0f;
            return {r, g, b, a};
        }

        TypographyRule ResolveFont(float size, TextToken token) const {
            if (token != TextToken::None) {
                return typography[static_cast<size_t>(token)];
            }
            TypographyRule custom;
            custom.size = size;
            return custom;
        }
    };

    // Phase 10: State-Dependent Styling
    // Now stores Intent (Token+RGBA) instead of resolved ColorF
    struct StateColor {
        uint32_t rgba = 0;
        ColorToken token = ColorToken::None;
        bool is_semantic = false;
        
        // Helper to construct from public Color
        // (This would typically happen in the builder)
    };

    struct StateStyle {
        std::optional<StateColor> bg;
        std::optional<StateColor> text;
        float elevation_delta = 0.0f;
        float scale_delta = 0.0f;
    };

#include "animated_state.hpp"
// ... 
    // AccessibilityRole is defined in fanta.h

    // Per-Element Persistent State (retained across frames)
    struct PersistentData {
        SpringPropertyAnimation<float> elevation;
        SpringPropertyAnimation<float> scale;
        
        // Phase 40: Spring-First Motion
        SpringPropertyAnimation<ColorF> color_anim;
        SpringPropertyAnimation<float> opacity_anim;
        SpringPropertyAnimation<Vec2> pos_anim;
        SpringPropertyAnimation<Vec2> size_anim;
        
        SpringPropertyAnimation<float> scroll_y; 
        SpringPropertyAnimation<float> scroll_x; 
        
        // Helper for wobble (Liquid Glass)
        SpringPropertyAnimation<Vec2> wobble;
        
        Vec2 drag_offset; 
        
        bool is_selected = false; 

        // Phase 32: Undo/Redo support for widgets
        std::string text_buffer;
        int cursor_pos = 0;
        int selection_start = -1;
        int selection_end = -1;
        float slider_val = 0.0f;
        bool toggle_val = false;

        bool operator==(const PersistentData& o) const {
            return drag_offset == o.drag_offset &&
                   is_selected == o.is_selected &&
                   text_buffer == o.text_buffer &&
                   cursor_pos == o.cursor_pos &&
                   selection_start == o.selection_start &&
                   selection_end == o.selection_end &&
                   slider_val == o.slider_val &&
                   toggle_val == o.toggle_val;
                   // TODO: Compare spring animations if needed for undo/redo consistency
                   // scroll_x == o.scroll_x && scroll_y == o.scroll_y
        }
    };

    // The "Real" properties of an element
    struct ElementState {
        ID id;
        std::string label;
        bool has_visible_label = false; 
        
        // Layout
        float w=0, h=0; 
        float p=0; 
        LayoutMode layout_mode = LayoutMode::Column; 
        Wrap wrap_mode = Wrap::NoWrap; 
        float gap = 0; 
        Align align = Align::Start; 
        Align align_content = Align::Start; 
        Justify justify = Justify::Start; 
        float flex_grow = 0.0f; 
        bool text_wrap = true; 
        
        // Style
        StateColor bg_color; 
        StateColor text_color = {0xFFFFFFFF, ColorToken::None, false}; 
        float font_size = 16.0f; 
        TextToken text_token = TextToken::None; 
        float corner_radius = 0;
        
        // Phase 10: Reactive Styles
        std::optional<StateStyle> hover_style;
        std::optional<StateStyle> active_style;
        std::optional<StateStyle> focus_style;
        bool is_hovered = false; 
        bool is_active = false;  
        bool is_focusable = false; 
        bool is_focused = false;   
        bool is_disabled = false;  
        bool is_selected = false;  

        // Effects
        // Redundant blur fields removed
        
        // Depth
        float elevation = 0;
        
        // Stage 2: Docking
        bool is_dock_space = false;

        // Stage 3: Professional Productivity
        std::string shader_source;
        
        // Phase 35: Accessibility Metadata
        AccessibilityRole accessibility_role = AccessibilityRole::Default;
        std::string accessible_name;
        
        // Positioning (Phase 11: Overlay/Absolute)
        bool is_absolute = false;
        float pos_left = NAN, pos_right = NAN, pos_top = NAN, pos_bottom = NAN;
        
        // Phase 7: Node Graph
        bool is_canvas = false;
        Vec2 canvas_pan = {0,0};
        float canvas_zoom = 1.0f;
        
        bool is_node = false;
        Vec2 node_pos = {0,0};
        
        // Phase 5.2: Wires & Ports
        bool is_wire = false;
        Vec2 wire_p0={0,0}, wire_p1={0,0}, wire_p2={0,0}, wire_p3={0,0};
        float wire_thickness = 1.0f;
        
        bool is_port = false;

        // Phase 7: Interactive Widgets
        bool is_slider = false;
        float slider_min = 0.0f;
        float slider_max = 1.0f;
        float slider_val = 0.0f; // Visual only

        bool is_toggle = false;
        bool toggle_val = false; // Visual only
        
        // Phase 8: Advanced Widgets
        Vec2 scroll_pos = {0,0}; 
        bool is_popup = false;
        
        // Phase 2.3: Virtualized List / Scroll
        bool is_scrollable = false;
        bool scroll_vertical = true;
        bool scroll_horizontal = false;
        
        // Phase NEW: TextInput
        bool is_text_input = false;
        std::string text_buffer;
        int cursor_pos = 0;
        int selection_start = -1;
        int selection_end = -1;
        bool ime_composing = false;
        
        // Phase NEW: TreeNode
        bool is_tree_node = false;
        bool tree_expanded = false;
        int tree_depth = 0;
        bool tree_has_children = false;
        
        // Phase 15: Mobile Support
        bool respect_safe_area = false;
        std::function<void(LifeCycle)> lifecycle_callback;

        // Phase 17: Accessibility Support
        std::string accessibility_label;
        std::string accessibility_hint;


        // Phase 18: RTL Support
        bool is_rtl = false;

        // Phase 20: Flutter-like Implicit Animations
        float anim_duration = 0.0f; // 0 = immediate
        CurveType anim_curve = CurveType::EaseOut;

        // Phase 21: Apple-like Aesthetics
        float backdrop_blur = 0.0f;
        float vibrancy = 0.0f;
        bool is_squircle = false;
        MaterialType material = MaterialType::None;

        // Phase 22: Hero Animations
        std::string hero_tag;

        // Phase 24: Scripting
        std::string script_source;
        
        // Tree
        ID parent{};
        std::vector<ID> children;

        // Persistent State Copy (Phase 5.1)
        PersistentData persistent;
    };

    struct StateDelta {
        std::map<ID, PersistentData> changes;
    };

    // The Global Store (Flat Data)
    struct AccessibilityItem {
        ID id;
        AccessibilityRole role;
        std::string name;
        float x, y, w, h;
    };

    struct AccessibilitySnapshot {
        std::vector<AccessibilityItem> items;
    };

    struct StateStore {
        std::vector<ElementState> elements;
        std::vector<ElementState> frame_elements;
        
        // Phase 6.2: Persistent Animation State
        std::map<ID, PersistentData> persistent_states;

        // Phase 32: Undo/Redo Stacks
        std::vector<StateDelta> undo_stack;
        std::vector<StateDelta> redo_stack;
        std::map<ID, PersistentData> frame_start_snapshot;
        
        // Phase 35: Accessibility Cache
        AccessibilitySnapshot accessibility_snapshot;

        void clear_frame() { frame_elements.clear(); }
    };

} // namespace internal
} // namespace fanta
