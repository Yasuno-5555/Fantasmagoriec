// Fantasmagorie v2 - Debug Utils
#pragma once

#include "context.hpp"
#include "../render/draw_list.hpp"

namespace fanta {

class DebugOverlay {
public:
    static void draw_layout_bounds(UIContext& ctx, DrawList& list);
    static void draw_hover_highlight(UIContext& ctx, DrawList& list);
};

} // namespace fanta
