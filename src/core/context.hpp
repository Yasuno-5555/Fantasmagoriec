#pragma once
#include "types_internal.hpp"
#include "memory.hpp"
#include "input.hpp"
#include "../backend/backend.hpp"
#include <vector>
#include <map>
#include <memory>

namespace fanta {
namespace internal {

    // Forward decls
    struct StateStore;  // Defined in types_internal.hpp (Legacy for now)
    
    // V5: Persistent World State
    // Lives for the entire application duration.
    struct WorldState {
        // Theme config
        Theme current_theme;
        
        // Persistent Widget State (Animation, Scroll, Text Input)
        // Currently defined in StateStore (types_internal.hpp) which we will likely move here or wrap.
        // For Phase 2, we just hold a reference or pointer to the legacy Store if we can't move it yet.
        // Actually, let's move the ownership of Backend here.
        std::unique_ptr<fanta::Backend> backend;
        
        // Global Assets (Fonts, Textures) - Managed by singletons currently (FontManager), strictly should be here.
        
        // Window Configuration
        int window_width = 0;
        int window_height = 0;
    };

    // V5: Per-Frame Transient State
    // Reset at the beginning of every frame.
    struct FrameState {
        // Linear Memory Allocator
        FrameArena arena;
        
        // Frame Time
        float dt = 0.016f;
        double time = 0.0;
        
        // ID Stack for the current frame build
        std::vector<ID> id_stack;
        
        // Layout Results for the current frame (Computed in Layout Phase)
        std::map<ID, ComputedLayout> layout_results;
        
        // Render Commands (DrawList is currently separate, but conceptually here)
        
        void reset() {
            arena.reset();
            id_stack.clear();
            // Note: layout_results are technically "this frame's result", 
            // but in Immediate Mode we often need "last frame's result" for the current frame's interactions.
            // This is the classic IMGUI lag.
            // In V5, we might separate "LayoutResultCache" (Last Frame) from "CurrentLayout" (This Frame).
            // For now, keep as is (cleared? No, RuntimeState didn't clear it explicitly in BeginFrame, only overlays).
            // Actually fanta.cpp clears elements but layout results persist? 
            // Let's check fanta.cpp. It calculates layouts in EndFrame and stores them.
            // So they are valid for the NEXT frame's "GetLayout"? No, "GetLayout" usually returns last frame.
            // EndFrame solves layout for *that* frame to render.
        }
    };
    
    // V5: Thread-Local Runtime Context
    // Access point for the currently active World and Frame.
    struct RuntimeContext {
        WorldState* world = nullptr;
        FrameState* frame = nullptr;
        
        // Legacy Compatibility bridging
        StateStore* store = nullptr; // Pointer to legacy store
        InputContext* input = nullptr;
    };

    // TLS Accessor
    RuntimeContext& GetContext();

} // namespace internal
} // namespace fanta
