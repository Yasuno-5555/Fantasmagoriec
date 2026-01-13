// Fantasmagorie v2 - Widgets Module
// Include all basic widgets
#pragma once

#include "widget_base.hpp"
#include "button.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "checkbox.hpp"
#include "window.hpp"
#include "scroll_area.hpp"
#include "table.hpp"
#include "tree.hpp"
#include "markdown.hpp"

namespace fanta {

// Layout helpers
inline void Row() {
    FANTA_ASSERT_CONTEXT();
    if (g_ctx->current_id != INVALID_NODE) {
        g_ctx->get(g_ctx->current_id).layout().dir = LayoutDir::Row;
    }
}

inline void Column() {
    FANTA_ASSERT_CONTEXT();
    if (g_ctx->current_id != INVALID_NODE) {
        g_ctx->get(g_ctx->current_id).layout().dir = LayoutDir::Column;
    }
}

inline void Spacer(float size = 0) {
    FANTA_ASSERT_CONTEXT();
    NodeID id = g_ctx->begin_node("spacer");
    NodeHandle n = g_ctx->get(id);
    if (size > 0) {
        n.constraint().width = size;
        n.constraint().height = size;
    } else {
        n.constraint().grow = 1.0f;
    }
    g_ctx->end_node();
}

// Panel/Group
inline NodeID BeginPanel(const char* id) {
    FANTA_ASSERT_CONTEXT();
    NodeID nid = g_ctx->begin_node(id);
    NodeHandle n = g_ctx->get(nid);
    n.style().bg = Color::Hex(0x1A1A1AFF);
    n.style().corner_radius = 8.0f;
    return nid;
}

inline void EndPanel() {
    FANTA_ASSERT_CONTEXT();
    g_ctx->end_node();
}

} // namespace fanta
