// Fantasmagorie v2 - Playground
#pragma once

#include "../core/context.hpp"
#include "../render/draw_list.hpp"

namespace fanta {

class Playground {
public:
    // Main Entry Points
    static void render_editor(UIContext& ctx);
    static void render_preview(UIContext& ctx);
    
    // Debug Visualizer
    static void draw_debug_overlay(UIContext& ctx, DrawList& list);

private:
    static void render_inspector(UIContext& ctx);
    static void render_toolbar(UIContext& ctx);
};

} // namespace fanta
