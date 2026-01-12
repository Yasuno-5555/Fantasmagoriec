// Fantasmagorie v2 - Widget Base
// Common widget infrastructure
#pragma once

#include "../core/context.hpp"
#include "../render/draw_list.hpp"

namespace fanta {

// Global context pointer (set by application)
inline UIContext* g_ctx = nullptr;
inline DrawList* g_draw = nullptr;

// Set global context
inline void set_context(UIContext* ctx, DrawList* draw = nullptr) {
    g_ctx = ctx;
    g_draw = draw;
}

// ============================================================================
// Input Result
// ============================================================================

enum class InputResult {
    None,
    Clicked,
    Changed,
    Submitted
};

// ============================================================================
// Hit Testing
// ============================================================================

inline bool hit_test(NodeID id) {
    if (!g_ctx) return false;
    auto& layout = g_ctx->store.layout[id];
    return Rect{layout.x, layout.y, layout.w, layout.h}
        .contains(g_ctx->mouse_pos);
}

inline bool is_hovered(NodeID id) {
    return hit_test(id);
}

inline bool is_clicked(NodeID id) {
    return is_hovered(id) && g_ctx->mouse_down;
}

} // namespace fanta
