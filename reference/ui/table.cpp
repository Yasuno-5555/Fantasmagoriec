#include "table.hpp"
#include "../core/contexts_internal.hpp"
#include <algorithm>

namespace fanta {
namespace internal {

void TableLogic::Update(ElementState& el, InputContext& input, const ComputedLayout& layout) {
    if (!el.is_table) return;
    
    // TODO: Implement full table interaction
    float mx = input.mouse_x;
    float my = input.mouse_y;
    
    bool inside = (mx >= layout.x && mx < layout.x + layout.w &&
                   my >= layout.y && my < layout.y + layout.h);
    
    el.is_hovered = inside;
}

void TableLogic::HandleHeaderClick(TableState& table, int column, bool ctrl) {
    (void)ctrl;
    if (column >= 0 && column < static_cast<int>(table.columns.size())) {
        if (table.columns[column].sortable) {
            table.toggle_sort(column);
        }
    }
}

void TableLogic::HandleRowClick(TableState& table, int row, bool ctrl, bool shift) {
    if (row < 0 || row >= table.total_rows) return;
    
    if (ctrl) {
        // Toggle selection
        auto it = std::find(table.multi_select.begin(), table.multi_select.end(), row);
        if (it != table.multi_select.end()) {
            table.multi_select.erase(it);
        } else {
            table.multi_select.push_back(row);
        }
    } else if (shift && table.selected_row >= 0) {
        // Range selection
        int start = std::min(table.selected_row, row);
        int end = std::max(table.selected_row, row);
        table.multi_select.clear();
        for (int i = start; i <= end; i++) {
            table.multi_select.push_back(i);
        }
    } else {
        // Single selection
        table.selected_row = row;
        table.multi_select.clear();
        table.multi_select.push_back(row);
    }
}

void TableLogic::HandleScroll(TableState& table, float delta, float container_height) {
    float max_scroll = std::max(0.0f, table.total_rows * table.row_height - container_height + table.header_height);
    table.scroll_y = std::clamp(table.scroll_y + delta, 0.0f, max_scroll);
    table.update_visible_rows(container_height);
}

} // namespace internal
} // namespace fanta
