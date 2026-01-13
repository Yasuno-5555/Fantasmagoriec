// Fantasmagorie v2 - Widget Base
// Common widget infrastructure
#pragma once

#include "../core/context.hpp"
#include "../render/draw_list.hpp"
#include "../core/types.hpp"
#include "../core/node.hpp"
#include <cassert>
#include <functional>
#include <map>

#include <cassert>

namespace fanta {

// Global context pointer (set by application)
inline UIContext* g_ctx = nullptr;
inline DrawList* g_draw = nullptr;

#define FANTA_ASSERT_CONTEXT() assert(g_ctx && "FANTA ERROR: UIContext is not set! Did you call fanta::set_context()?")
#define FANTA_ASSERT_DRAW() assert(g_draw && "FANTA ERROR: DrawList is not set! check your fanta::set_context() call.")

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
    
    // Use prev_layout for hit testing because current frame layout is solved after widget code
    auto it = g_ctx->prev_layout.find(id);
    if (it == g_ctx->prev_layout.end()) return false;
    
    auto& layout = it->second;
    return Rect{layout.x, layout.y, layout.w, layout.h}
        .contains(g_ctx->mouse_pos);
}

inline bool is_active(NodeID id) {
    return g_ctx && g_ctx->active_id == id;
}

inline void set_active(NodeID id) {
    if (g_ctx) g_ctx->active_id = id;
}

inline void clear_active() {
    if (g_ctx) g_ctx->active_id = INVALID_NODE;
}

inline bool is_hovered(NodeID id) {
    bool hit = hit_test(id);
    if (hit && g_ctx) g_ctx->hot_id = id;
    return hit;
}

inline bool is_clicked(NodeID id) {
    if (is_hovered(id) && g_ctx->mouse_clicked) {
        set_active(id);
        return true;
    }
    return false;
}

inline bool is_dragging(NodeID id) {
    if (is_active(id)) {
        if (!g_ctx->mouse_down) {
            clear_active();
            return false;
        }
        return true;
    }
    return false;
}

} // namespace fanta

namespace fanta {

struct WidgetCommon {
    float width = 0;
    float height = 0;
    float grow = 0;
    float padding = 0;
    float gap = 0;
    bool focusable = false;
    const char* custom_id = nullptr;
    
    // Visual modifiers for implicit animation
    std::map<InteractionState, std::function<void(ResolvedStyle&)>> visual_modifiers;
    
    // Fluent setters
    void set_width(float w) { width = w; }
    void set_height(float h) { height = h; }
    void set_grow(float g) { grow = g; }
    void set_padding(float p) { padding = p; }
    void set_gap(float g) { gap = g; }
    void set_id(const char* id) { custom_id = id; }
    
    void add_modifier(InteractionState state, std::function<void(ResolvedStyle&)> fn) {
        visual_modifiers[state] = fn;
    }
    
    // Apply common properties to a node
    void apply(NodeHandle& n);
};

// ============================================================================
// Scope Guard (RAII)
// ============================================================================

class PanelScope {
public:
    PanelScope(const char* label) {
        if (g_ctx) {
            id_ = g_ctx->begin_node(label);
            // Default panel logic? 
            // Minimal default: just a container.
            // But user might want standard panel styling.
            // Let's keep it minimal for now, maybe set meaningful defaults if needed.
        }
    }

    // Extended constructor for ID + composition
    PanelScope(const char* label, std::function<void(WidgetCommon&)> config) : PanelScope(label) {
        if (config && g_ctx) {
            WidgetCommon common;
            config(common);
            NodeHandle n = g_ctx->get(id_);
            common.apply(n);
        }
    }
    
    ~PanelScope() {
        if (g_ctx) g_ctx->end_node();
    }
    
    // Non-copyable
    PanelScope(const PanelScope&) = delete;
    PanelScope& operator=(const PanelScope&) = delete;
    
    NodeID id() const { return id_; }
    
private:
    NodeID id_ = INVALID_NODE;
};

} // namespace fanta
