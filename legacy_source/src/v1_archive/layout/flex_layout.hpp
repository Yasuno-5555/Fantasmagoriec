#pragma once
#include "../core/ui_context.hpp"

// A very minimal flex layout solver.
// Only supports:
// - Row / Column
// - Fixed Size vs Auto
// - Flex Grow (basic)
// - Alignment (Start, Center, End, Stretch)
// No wrapping, no shrinkage (for now), no min/max constraints validation.

namespace Layout {

    void solve(UIContext& ctx, NodeID root_id, float window_width, float window_height);

} // namespace Layout
