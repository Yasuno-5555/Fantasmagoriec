// UI Layer: Interaction Pass
// Per Iron Philosophy XI-XII: Interpretation logic lives HERE, not in InputContext.
// InputContext = Facts. Interaction Pass = Interpretation.

#pragma once

#include "core/contexts_internal.hpp"
#include "view/definitions.hpp"

namespace fanta {
namespace interaction {

    // --- Interpretation Helpers (Pure Functions of Facts) ---
    
    // Edge detection: mouse button pressed this frame?
    inline bool MousePressed() {
        auto& ctx = internal::GetEngineContext();
        return ctx.input.mouse_down && !ctx.input.mouse_was_down;
    }
    
    // Edge detection: mouse button released this frame?
    inline bool MouseReleased() {
        auto& ctx = internal::GetEngineContext();
        return !ctx.input.mouse_down && ctx.input.mouse_was_down;
    }
    
    // Level detection: mouse button currently held?
    inline bool MouseDown() {
        return internal::GetEngineContext().input.mouse_down;
    }

    // --- ID-Based Interaction Queries ---
    
    inline bool IsHot(internal::ID id) {
        auto& ctx = internal::GetEngineContext();
        return ctx.interaction.hot_id == id && !id.is_empty();
    }
    
    inline bool IsActive(internal::ID id) {
        auto& ctx = internal::GetEngineContext();
        return ctx.interaction.active_id == id && !id.is_empty();
    }
    
    // Click = Active (was pressed on this element) + Hot (still on it) + Released
    inline bool IsClicked(internal::ID id) {
        return IsActive(id) && IsHot(id) && MouseReleased();
    }

    // --- Focus Management ---
    
    inline bool IsFocused(internal::ID id) {
        auto& ctx = internal::GetEngineContext();
        return ctx.persistent.focused_id == id && !id.is_empty();
    }
    
    inline void RequestFocus(internal::ID id) {
        internal::GetEngineContext().persistent.focused_id = id;
    }
    
    inline void ClearFocus() {
        internal::GetEngineContext().persistent.focused_id = internal::ID{};
    }

    // --- Hit Test (Updates InteractionState) ---
    // Call this for each interactive element with its last-frame rect
    
    inline void UpdateHitTest(internal::ID id, float rx, float ry, float rw, float rh) {
        auto& ctx = internal::GetEngineContext();
        float mx = ctx.input.mouse_x;
        float my = ctx.input.mouse_y;
        
        bool hovered = (mx >= rx && mx < rx + rw && my >= ry && my < ry + rh);
        
        if (hovered) {
            ctx.interaction.hot_id = id;
            // Phase 10: Hit-Test Visualizer (Magenta Frame)
            // Use internal state directly to avoid circular dependency with debug.hpp
            if (ctx.config.show_debug_overlay) {
                ctx.debug.commands.push_back({internal::DebugOverlayCmd::Rect, "", 0, 0, 0, rx, ry, rw, rh, {1.0f, 0.0f, 1.0f, 0.5f}});
            }
        }
        // Note: hot_id is reset each frame, so no need to clear it
        
        // Capture on press
        if (hovered && MousePressed()) {
            ctx.interaction.active_id = id;
            ctx.persistent.focused_id = id; // Focus follows click
        }
        
        // Release capture
        if (ctx.interaction.active_id == id && MouseReleased()) {
            ctx.interaction.active_id = internal::ID{};
        }
    }
    
    // Global Focus Control
    inline void UpdateFocus() {
        auto& ctx = internal::GetEngineContext();
        // Clear focus if clicking nothing
        if (MousePressed() && ctx.interaction.hot_id.is_empty()) {
            ctx.persistent.focused_id = internal::ID{};
        }
    }
    
    // --- Scroll Interaction ---
    
    inline internal::Vec2 HandleScrollInteraction(ui::ViewHeader* node) {
        auto& ctx = internal::GetEngineContext();
        auto& persist = ctx.persistent;
        
        internal::Vec2 offset = persist.scroll_offsets[node->id.value];
        
        // 1. Mouse Wheel (Facts: mouse_wheel)
        // Note: Wheel is usually vertical. 
        if (IsHot(node->id)) {
            offset.y += ctx.input.mouse_wheel * -20.0f; // scroll speed
        }
        
        // 2. Drag to scroll (Facts: mouse_down, mouse_dx/dy)
        if (IsActive(node->id) && ctx.input.mouse_down) {
            // Drag direction is opposite to scroll offset
            if (node->is_row) offset.x -= ctx.input.mouse_dx;
            else offset.y -= ctx.input.mouse_dy;
        }

        // 3. Clamp to bounds
        float max_x = std::max(0.0f, node->content_size.w - node->measured_size.w);
        float max_y = std::max(0.0f, node->content_size.h - node->measured_size.h);
        
        if (offset.x < 0) offset.x = 0;
        if (offset.x > max_x) offset.x = max_x;
        if (offset.y < 0) offset.y = 0;
        if (offset.y > max_y) offset.y = max_y;
        
        persist.scroll_offsets[node->id.value] = offset;
        return offset;
    }
    
    // --- Frame Management ---
    
    inline void BeginInteractionPass() {
        auto& ctx = internal::GetEngineContext();
        ctx.interaction.reset_frame();
    }

} // namespace interaction
} // namespace fanta
