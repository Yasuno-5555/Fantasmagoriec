#pragma once

#include "core/types_internal.hpp"
#include <vector>
#include <map>

namespace fanta {
namespace internal {

// Computed layout result (temporary during layout pass)
struct ComputedLayout {
    float x = 0, y = 0, w = 0, h = 0;
};

// Layout Engine: Flexbox-based layout solver
class LayoutEngine {
public:
    // Solve layout for entire tree
    // Returns map of element ID -> computed position/size
    // Solve layout for entire tree
    // Returns map of element ID -> computed position/size
    std::map<::fanta::internal::ID, ComputedLayout> solve(
        const std::vector<::fanta::internal::ElementState>& elements,
        const std::map<::fanta::internal::ID, size_t>& id_map,
        float root_width,
        float root_height
    );

private:
    // Measure element's intrinsic size (if auto)
    void measure_element(
        const ::fanta::internal::ElementState& el,
        float available_w,
        float available_h,
        float& out_w,
        float& out_h
    );

    // Solve layout for a single node and its children
    ComputedLayout solve_node(
        const ::fanta::internal::ElementState& el,
        const std::vector<::fanta::internal::ElementState>& elements,
        const std::map<::fanta::internal::ID, size_t>& id_map,
        float parent_x,
        float parent_y,
        float available_w,
        float available_h,
        std::map<::fanta::internal::ID, ComputedLayout>& results
    );

    // Layout children in flexbox manner
    Vec2 layout_children(
        const ::fanta::internal::ElementState& parent,
        const std::vector<::fanta::internal::ID>& child_ids,
        const std::vector<::fanta::internal::ElementState>& elements,
        const std::map<::fanta::internal::ID, size_t>& id_map,
        float content_x,
        float content_y,
        float content_w,
        float content_h,
        std::map<::fanta::internal::ID, ComputedLayout>& results
    );
};

} // namespace internal
} // namespace fanta
