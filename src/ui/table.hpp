#pragma once

#include "../core/types_internal.hpp"
#include <string>
#include <vector>
#include <functional>

namespace fanta {
namespace internal {

// Table Column Definition
struct TableColumnDef {
    std::string header;
    float width = 100.0f;
    float min_width = 30.0f;
    bool resizable = true;
    bool sortable = true;
    
    // Sort callback (optional)
    std::function<bool(int, int)> sort_func;
};

// Table State
struct TableState {
    std::vector<TableColumnDef> columns;
    int sort_column = -1;        // -1 = no sort
    bool sort_ascending = true;
    
    // Scroll position for virtualization
    float scroll_y = 0;
    int visible_row_start = 0;
    int visible_row_count = 0;
    int total_rows = 0;
    
    // Resize state
    int resizing_column = -1;
    float resize_start_width = 0;
    float resize_start_mouse_x = 0;
    
    // Row height
    float row_height = 24.0f;
    float header_height = 28.0f;
    
    // Selection
    int selected_row = -1;
    std::vector<int> multi_select;  // For Ctrl+click
    
    void begin_resize(int col, float mouse_x) {
        resizing_column = col;
        resize_start_width = columns[col].width;
        resize_start_mouse_x = mouse_x;
    }
    
    void update_resize(float mouse_x) {
        if (resizing_column < 0) return;
        float delta = mouse_x - resize_start_mouse_x;
        float new_width = resize_start_width + delta;
        columns[resizing_column].width = std::max(columns[resizing_column].min_width, new_width);
    }
    
    void end_resize() {
        resizing_column = -1;
    }
    
    void toggle_sort(int col) {
        if (sort_column == col) {
            sort_ascending = !sort_ascending;
        } else {
            sort_column = col;
            sort_ascending = true;
        }
    }
    
    // Calculate visible rows based on scroll position
    void update_visible_rows(float container_height) {
        int max_visible = static_cast<int>((container_height - header_height) / row_height) + 2;
        visible_row_start = static_cast<int>(scroll_y / row_height);
        visible_row_count = std::min(max_visible, total_rows - visible_row_start);
        visible_row_count = std::max(0, visible_row_count);
    }
};

// Table Logic
struct TableLogic {
    static void Update(ElementState& el, InputContext& input, const ComputedLayout& layout);
    static void HandleHeaderClick(TableState& table, int column, bool ctrl);
    static void HandleRowClick(TableState& table, int row, bool ctrl, bool shift);
    static void HandleScroll(TableState& table, float delta, float container_height);
};

} // namespace internal
} // namespace fanta
