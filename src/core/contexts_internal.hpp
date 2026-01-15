#pragma once
// L5: Engine Context (Internal Aggregator)
// MINIMAL V5 VERSION - No legacy fields

#include "core/types_core.hpp"
#include "core/context.hpp"
#include "backend/backend.hpp"
#include "backend/drawlist.hpp"
#include <string>
#include <vector>
#include <map>
#include <array>
#include <variant>

namespace fanta {
namespace internal {

    // --- Theme (Minimal) ---
    struct TypographyRule {
        float size = 16.0f;
    };

    struct Theme {
        std::array<ColorF, 16> palette;
        std::array<TypographyRule, 16> typography;
    };

    // --- Element State (MINIMAL) ---
    struct ElementState {
        ID id{};
        ID parent{};
        ComputedLayout layout;
        ColorF color = {0.2f, 0.2f, 0.22f, 1.0f};
        float corner_radius = 0;
        float elevation = 0;
        bool is_text = false;
        std::string label;
    };

    // --- State Store (MINIMAL) ---
    struct StateStore {
        std::vector<ElementState> frame_elements;
        void clear_frame() { frame_elements.clear(); }
    };

    // --- Debugging Infrastructure (Phase 10) ---
    struct DebugOverlayCmd {
        enum Type { Text, Bar, Rect };
        Type type;
        std::string label;
        float value = 0; 
        float min = 0;
        float max = 1.0f;
        float x=0, y=0, w=0, h=0; // Phase 10: Rects
        ColorF color = {1,0,1,1};
    };

    struct DebugOverlayState {
        std::vector<DebugOverlayCmd> commands;
        void clear() { commands.clear(); }
    };

    struct UIConfig {
        bool show_debug_overlay = false;
        bool disable_animation = false;
        bool deterministic_mode = false;
        float fixed_dt = 1.0f / 60.0f;
        
        // Phase 10: Step Mode
        bool is_paused = false;
        bool step_requested = false;
    };

    // --- Engine Context ---
    struct EngineContext {
        WorldState world;
        FrameState frame;
        RuntimeState runtime;
        InputContext input;           // Facts only
        InteractionState interaction; // Ephemeral (hot_id, active_id, focus_id)
        PersistentState persistent;   // Survives frames (last_frame_rects, scroll_offsets)
        StateStore store;
        std::vector<ID> node_stack;
        Theme current_theme;
        UIConfig config;              // Phase 10
        DebugOverlayState debug;      // Phase 10
    };

    // --- Global Accessors ---
    EngineContext& GetEngineContext();
    StateStore& GetStore();
    void Log(const std::string& msg);
    uint64_t& GetGlobalIDCounter();

} // namespace internal
} // namespace fanta
