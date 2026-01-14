#pragma once

#include "../core/types_internal.hpp"
#include <vector>
#include <algorithm>
#include <optional>

namespace fanta {
namespace internal {

// Grid Track (row or column definition)
struct GridTrack {
    enum class SizeType { Fixed, Fraction, Auto, MinContent, MaxContent };
    
    SizeType type = SizeType::Fraction;
    float value = 1.0f;  // For Fixed: pixels, For Fraction: fr units
    float min_size = 0;
    float max_size = std::numeric_limits<float>::max();
    
    // Computed
    float computed_size = 0;
    float computed_start = 0;
    
    static GridTrack Fixed(float px) { return {SizeType::Fixed, px, px, px}; }
    static GridTrack Fr(float fr) { return {SizeType::Fraction, fr, 0, std::numeric_limits<float>::max()}; }
    static GridTrack Auto() { return {SizeType::Auto, 0, 0, std::numeric_limits<float>::max()}; }
};

// Grid Placement (where an item is placed)
struct GridPlacement {
    int row_start = 0;
    int row_end = 1;    // Span = end - start
    int col_start = 0;
    int col_end = 1;
    
    int row_span() const { return row_end - row_start; }
    int col_span() const { return col_end - col_start; }
};

// Grid State
struct GridState {
    std::vector<GridTrack> rows;
    std::vector<GridTrack> columns;
    float row_gap = 0;
    float col_gap = 0;
    
    // Auto placement
    enum class AutoFlow { Row, Column, RowDense, ColumnDense };
    AutoFlow auto_flow = AutoFlow::Row;
    
    // Alignment
    Align justify_items = Align::Stretch;
    Align align_items = Align::Stretch;
    Align justify_content = Align::Start;
    Align align_content = Align::Start;
    
    void set_columns(const std::vector<GridTrack>& cols) { columns = cols; }
    void set_rows(const std::vector<GridTrack>& r) { rows = r; }
    
    void add_column(GridTrack t) { columns.push_back(t); }
    void add_row(GridTrack t) { rows.push_back(t); }
    
    void solve(float available_width, float available_height);
    Rectangle get_cell_rect(int row, int col) const;
    Rectangle get_area_rect(const GridPlacement& placement) const;
};

// Grid Layout Logic
struct GridLayoutLogic {
    static void SolveTracks(std::vector<GridTrack>& tracks, float available, float gap);
    static void DistributeFraction(std::vector<GridTrack>& tracks, float remaining);
};

} // namespace internal
} // namespace fanta
