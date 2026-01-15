// Fantasmagorie v2 - Debug Utils
#pragma once

#include "core/context.hpp"
#include "render/paint_list.hpp"

namespace fanta {

class DebugOverlay {
public:
    static void draw_layout_bounds(UIContext& ctx, PaintList& list);
    static void draw_hover_highlight(UIContext& ctx, PaintList& list);
};

} // namespace fanta


