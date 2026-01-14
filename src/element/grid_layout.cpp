#include "grid_layout.hpp"
#include <numeric>

namespace fanta {
namespace internal {

void GridState::solve(float available_width, float available_height) {
    GridLayoutLogic::SolveTracks(columns, available_width, col_gap);
    GridLayoutLogic::SolveTracks(rows, available_height, row_gap);
}

Rectangle GridState::get_cell_rect(int row, int col) const {
    if (row < 0 || row >= static_cast<int>(rows.size()) ||
        col < 0 || col >= static_cast<int>(columns.size())) {
        return {0, 0, 0, 0};
    }
    
    return {
        columns[col].computed_start,
        rows[row].computed_start,
        columns[col].computed_size,
        rows[row].computed_size
    };
}

Rectangle GridState::get_area_rect(const GridPlacement& placement) const {
    if (placement.row_start < 0 || placement.row_end > static_cast<int>(rows.size()) ||
        placement.col_start < 0 || placement.col_end > static_cast<int>(columns.size())) {
        return {0, 0, 0, 0};
    }
    
    float x = columns[placement.col_start].computed_start;
    float y = rows[placement.row_start].computed_start;
    float w = 0, h = 0;
    
    for (int c = placement.col_start; c < placement.col_end; c++) {
        w += columns[c].computed_size;
        if (c > placement.col_start) w += col_gap;
    }
    
    for (int r = placement.row_start; r < placement.row_end; r++) {
        h += rows[r].computed_size;
        if (r > placement.row_start) h += row_gap;
    }
    
    return {x, y, w, h};
}

void GridLayoutLogic::SolveTracks(std::vector<GridTrack>& tracks, float available, float gap) {
    if (tracks.empty()) return;
    
    float total_gap = gap * (tracks.size() - 1);
    float remaining = available - total_gap;
    
    // First pass: fixed sizes
    for (auto& t : tracks) {
        if (t.type == GridTrack::SizeType::Fixed) {
            t.computed_size = t.value;
            remaining -= t.computed_size;
        }
    }
    
    // Second pass: auto sizes (use min_size for now)
    for (auto& t : tracks) {
        if (t.type == GridTrack::SizeType::Auto) {
            t.computed_size = t.min_size;
            remaining -= t.computed_size;
        }
    }
    
    // Third pass: fraction sizes
    DistributeFraction(tracks, remaining);
    
    // Compute positions
    float pos = 0;
    for (auto& t : tracks) {
        t.computed_start = pos;
        pos += t.computed_size + gap;
    }
}

void GridLayoutLogic::DistributeFraction(std::vector<GridTrack>& tracks, float remaining) {
    float total_fr = 0;
    for (const auto& t : tracks) {
        if (t.type == GridTrack::SizeType::Fraction) {
            total_fr += t.value;
        }
    }
    
    if (total_fr <= 0) return;
    
    float fr_unit = remaining / total_fr;
    
    for (auto& t : tracks) {
        if (t.type == GridTrack::SizeType::Fraction) {
            t.computed_size = std::clamp(t.value * fr_unit, t.min_size, t.max_size);
        }
    }
}

} // namespace internal
} // namespace fanta
