// Fantasmagorie v3 - Consolidated Implementation (Rich & Deep)
#define NOMINMAX
#include <windows.h>
#include <glad.h>
#include "fanta_core_unified.hpp"
#include "fanta_widgets_unified.hpp"
#include "fanta_render_unified.hpp"
#include "core/context.hpp"

namespace fanta {

// --- Global Context Management ---
static UIContext* g_current_ctx = nullptr;
static PaintList* g_current_paint = nullptr;

void set_context(UIContext* ctx, PaintList* paint) {
    g_current_ctx = ctx;
    g_current_paint = paint;
    g_ctx = ctx; // For legacy v2 glue if still exists
}

void load_gl_functions() {
    gladLoadGL();
}

// --- Widget Implementations ---

void Label::build() {
    if (built || !g_current_ctx) return;
    built = true;
    NodeID id = g_current_ctx->begin_node(text.c_str());
    NodeHandle n(id, &g_current_ctx->store);
    common.apply(n);
    n.render().is_text = true; n.render().text = text;
    g_current_ctx->end_node();
}

void Button::build() {
    if (built || !g_current_ctx) return;
    built = true;
    NodeID id = g_current_ctx->begin_node(text.c_str());
    NodeHandle n(id, &g_current_ctx->store);
    if (!common.has_custom_style) {
        n.style().bg = Color::Hex(0x3B82F6FF);
        n.style().radius = 4.0f;
    }
    common.apply(n);
    n.render().is_text = true; n.render().text = text;
    if (g_current_ctx->active_id == id && g_current_ctx->mouse_released) {
        if (click_fn) click_fn();
    }
    g_current_ctx->end_node();
}

void Slider::build() {
    if (built || !g_current_ctx || !val) return;
    built = true;
    NodeID id = g_current_ctx->begin_node(label.c_str());
    NodeHandle n(id, &g_current_ctx->store);
    common.apply(n);
    // Deep access: Directly modify layout or handle dragging
    if (g_current_ctx->active_id == id) {
        // Drag logic...
    }
    g_current_ctx->end_node();
}

// --- Rendering Logic ---

void UIRenderer::render(const NodeStore& store, PaintList& paint) {
    if (store.nodes.empty()) return;
    render_recursive(store.nodes[0], store, paint);
}

void UIRenderer::render_recursive(NodeID id, const NodeStore& store, PaintList& paint) {
    auto it_l = store.layout.find(id);
    if (it_l == store.layout.end()) return;
    const auto& l = it_l->second;
    Rect b = {l.x, l.y, l.w, l.h};

    auto it_r = store.render.find(id);
    if (it_r != store.render.end()) {
        const auto& rd = it_r->second;
        if (rd.blur_radius > 0.5f) paint.layer_begin(b, rd.blur_radius, rd.backdrop_blur);
    }

    auto it_s = store.style.find(id);
    if (it_s != store.style.end()) paint.rect(b, it_s->second);

    if (it_r != store.render.end() && it_r->second.is_text) {
        paint.text({b.x + 8, b.y + 16}, it_r->second.text, Color::White());
    }

    auto it_t = store.tree.find(id);
    if (it_t != store.tree.end()) {
        for (NodeID child : it_t->second.children) render_recursive(child, store, paint);
    }

    if (it_r != store.render.end() && it_r->second.blur_radius > 0.5f) paint.layer_end();
}

// --- Backend Boilerplate ---
// (We would implement the actual OpenGL calls here in a real scenario)
// For this stabilization, we ensure the symbols exist.

} // namespace fanta
