#pragma once
// UI Layer: View Definitions (The AST)
// Lives in User Land, NOT V5 Core
// All structs must be POD-compatible for FrameArena

#include "core/types_core.hpp"
#include <string_view>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace fanta {
namespace ui {

    // --- View Types ---
    enum class ViewType : uint8_t { 
        Box, 
        Text,
        Button,
        Scroll,
        TextInput,
        Toggle,
        Slider,
        ContextMenu,
        Plot,
        Table,
        TreeNode,
        ColorPicker,
        MenuBar,
        MenuItem,
        DragSource,
        DropTarget,
        TextArea,
        Splitter,
        Bezier
    };

    // --- Common View Header (POD) ---
    // Universal Masquerade: All common properties live here.
    
    enum class Align : uint8_t { Start = 0, Center = 1, End = 2, Stretch = 3 };
    
    struct ViewHeader {
        ViewType type = ViewType::Box;
        internal::ID id{};
        
        // Tree Structure (intrusive)
        ViewHeader* next_sibling = nullptr;
        ViewHeader* first_child = nullptr;
        
        // --- Universal Layout Inputs ---
        float width = 0;   // 0 = Auto
        float height = 0;
        float padding = 0;
        float margin = 0;
        float flex_grow = 0;
        float flex_shrink = 1;  
        bool is_row = false;    
        bool wrap = false;      
        Align align = Align::Stretch;  
        
        // --- Universal Style Inputs ---
        internal::ColorF bg_color = {0, 0, 0, 0}; // Transparent by default
        internal::ColorF fg_color = {1, 1, 1, 1}; // White by default (Text, etc.)
        float border_radius = 0;
        float elevation = 0;
        bool is_squircle = false;
        float backdrop_blur = 0;
        float wobble_x = 0;
        float wobble_y = 0;
        float font_size = 14.0f;
        
        // Layout Outputs (computed by layout engine)
        struct { float w = 0, h = 0; } measured_size;
        struct { float w = 0, h = 0; } content_size;
        struct { float x = 0, y = 0, w = 0, h = 0; } computed_rect;
    };

    // --- BoxView (Container) ---
    struct BoxView : public ViewHeader {
        BoxView() { bg_color = {0.15f, 0.15f, 0.18f, 1.0f}; }
        
        void add(ViewHeader* child) {
            if (!child) return;
            ViewHeader* cur = first_child;
            while (cur) { if (cur == child) return; cur = cur->next_sibling; }
            if (!first_child) first_child = child;
            else { ViewHeader* last = first_child; while (last->next_sibling) last = last->next_sibling; last->next_sibling = child; }
        }
    };

    // --- TextView (Leaf) ---
    struct TextView : public ViewHeader {
        const char* text = nullptr;
    };

    // --- ButtonView (Interactive Leaf) ---
    struct ButtonView : public ViewHeader {
        ButtonView() {
            bg_color = {0.25f, 0.25f, 0.3f, 1.0f};
            elevation = 2.0f;
            border_radius = 6.0f;
        }
        const char* label = nullptr;
        internal::ColorF hover_color = {0.35f, 0.35f, 0.4f, 1.0f};
        internal::ColorF active_color = {0.2f, 0.2f, 0.25f, 1.0f};
    };

    // --- ScrollView ---
    struct ScrollView : public BoxView {
        bool show_scrollbar = true;
        internal::ColorF scrollbar_color = {0.3f, 0.3f, 0.35f, 0.6f};
    };

    // --- TextInputView ---
    struct TextInputView : public ViewHeader {
        char* buffer = nullptr;
        size_t buffer_size = 0;
        const char* placeholder = nullptr;
    };

    // --- TextAreaView ---
    struct TextAreaView : public ViewHeader {
        char* buffer = nullptr;
        size_t buffer_size = 0;
        const char* placeholder = nullptr;
        float line_height = 14.0f * 1.4f; 
    };

    // --- SplitterView ---
    struct SplitterView : public BoxView {
        float* split_ratio = nullptr; 
        bool is_vertical = false; 
        float handle_thickness = 8.0f;
    };

    // --- ToggleView ---
    struct ToggleView : public ViewHeader {
        bool* value = nullptr;
        const char* label = nullptr;
    };

    // --- SliderView ---
    struct SliderView : public ViewHeader {
        float* value = nullptr;
        float min = 0;
        float max = 1.0f;
        const char* label = nullptr;
    };

    // --- ContextMenuView ---
    struct ContextMenuItem {
        const char* label = nullptr;
        bool* triggered = nullptr;  
    };
    
    struct ContextMenuView : public BoxView {
        ContextMenuItem* items = nullptr;
        size_t item_count = 0;
        bool* is_open = nullptr;
        float anchor_x = 0, anchor_y = 0;
    };

    // --- PlotView ---
    enum class PlotType : uint8_t { Line, Bar, Scatter };
    
    struct PlotView : public ViewHeader {
        const float* data = nullptr;
        size_t data_count = 0;
        PlotType plot_type = PlotType::Line;
        internal::ColorF line_color = {0.3f, 0.6f, 1.0f, 1.0f};
        float min_val = 0, max_val = 1.0f;
    };

    // --- TableView ---
    using RowBuilderFn = void(*)(int row_index, void* user_data);
    
    struct TableView : public BoxView {
        int row_count = 0;
        float row_height = 24.0f;
        RowBuilderFn row_builder = nullptr;
        void* user_data = nullptr;
    };

    // --- TreeNodeView ---
    struct TreeNodeView : public BoxView {
        const char* label = nullptr;
        bool* expanded = nullptr;  
        int depth = 0;
        float indent_per_level = 20.0f;
    };

    // --- HSV etc ---
    struct HSV {
        float h = 0, s = 0, v = 0, a = 1;
        static HSV FromRGB(float r, float g, float b, float alpha = 1.0f) {
            HSV hsv; hsv.a = alpha;
            float mx = std::max({r, g, b}), mn = std::min({r, g, b}), d = mx - mn;
            hsv.v = mx; hsv.s = (mx > 0) ? d / mx : 0;
            if (d < 0.00001f) hsv.h = 0;
            else if (mx == r) hsv.h = 60.0f * std::fmod((g - b) / d, 6.0f);
            else if (mx == g) hsv.h = 60.0f * ((b - r) / d + 2.0f);
            else hsv.h = 60.0f * ((r - g) / d + 4.0f);
            if (hsv.h < 0) hsv.h += 360.0f;
            return hsv;
        }
        internal::ColorF ToRGB() const {
            float c = v * s, x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f)), m = v - c;
            float r = 0, g = 0, b = 0;
            if (h < 60) { r = c; g = x; }
            else if (h < 120) { r = x; g = c; }
            else if (h < 180) { g = c; b = x; }
            else if (h < 240) { g = x; b = c; }
            else if (h < 300) { r = x; b = c; }
            else { r = c; b = x; }
            return {r + m, g + m, b + m, a};
        }
    };

    struct ColorPickerView : public ViewHeader {
        internal::ColorF* color = nullptr;
        HSV hsv;
        float sv_size = 150.0f;
        float hue_bar_width = 20.0f;
    };

    struct MenuBarView : public BoxView {
        float bar_height = 26.0f;
        int active_menu = -1;
    };

    struct MenuItemView : public ViewHeader {
        const char* label = nullptr;
        const char* shortcut = nullptr;
        bool* triggered = nullptr;
        bool is_separator = false;
    };

    struct DragSourceView : public ViewHeader {
        const char* payload_type = nullptr;
        void* payload_data = nullptr;
        size_t payload_size = 0;
    };

    struct DropTargetView : public ViewHeader {
        const char* accept_type = nullptr;
        bool* dropped = nullptr;
        void** received_data = nullptr;
    };

    struct BezierView : public ViewHeader {
        float p0[2], p1[2], p2[2], p3[2];
        float thickness;
    };

} // namespace ui
} // namespace fanta
