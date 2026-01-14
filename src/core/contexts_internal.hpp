#include "input.hpp"
#include "memory.hpp"
#include "context.hpp" // Phase 2
#include "../element/layout.hpp"
#include "../backend/backend.hpp" // Phase 31: Required for EngineContext
#include <string>
#include <vector>
#include <utility>
#include <memory>

namespace fanta {
namespace internal {
    void Log(const std::string& msg);


    struct InputContext {
        InputState state;
        float mouse_x = 0;
        float mouse_y = 0;
        float mouse_dx = 0; // Phase 5.1: Delta X
        float mouse_dy = 0; // Phase 5.1: Delta Y
        bool mouse_down = false;
        float mouse_wheel = 0;
        
        // V2-D: Coordinate Correction
        Vec2 mouse_screen = {0, 0}; // Raw Hardware
        Vec2 mouse_world = {0, 0};  // Canvas Space
        
        // Legacy (Remove later or alias to mouse_screen)
        // float mouse_x = 0; // Removed duplicate
        // float mouse_y = 0; // Removed duplicate
        
        // Phase 5.2: Wire Dragging
        bool is_dragging_wire = false;
        ID wire_start_id{};
        std::vector<std::pair<ID, ID>> connection_events;
        
        // Phase 11: Focus tracking
        ID focused_id{};
        
        // Phase V2-B: Input Capture
        ID captured_id{};

        // Phase 5.1: Marquee Selection
        bool marquee_active = false;
        bool is_dragging_nodes = false; // Phase 5.1
        Vec2 marquee_start = {0, 0};
        Rectangle marquee_rect = {0, 0, 0, 0};

        // Phase 41: Gestures
        struct {
            bool is_swiping = false;
            Vec2 start_pos = {0,0};
            Vec2 current_delta = {0,0};
            double start_time = 0;
            
            bool is_pinching = false;
            float start_dist = 0;
            float current_scale = 1.0f;
        } gesture;
    };

    // Phase 9: Theme Context
    struct ThemeContext {
        Theme current;
    };

    // Framework Runtime State
    struct RuntimeState {
        // FrameArena frame_arena; // Moved to FrameState (Phase 2)

        int width = 0;
        int height = 0;
        float dt = 0.016f;
        
        bool screenshot_pending = false;
        std::string screenshot_filename;

        std::vector<ID> overlay_ids;
        std::map<ID, ComputedLayout> layout_results;
        std::map<ID, Vec2> element_mouse_pos; // V2-D: Correct coordinate for logic
        bool debug_hit_test = false;
        
        // V2-A: ID Stack
        std::vector<ID> id_stack; 

        // Phase 22: Hero Animations
        std::map<std::string, Rectangle> hero_registry;
        std::map<std::string, Rectangle> prev_hero_registry;
    };



    struct EngineContext {
        // V5 Core Structures
        WorldState world;
        FrameState frame;
        
        // Legacy/Transition Members
        StateStore store; // Still held here for now, or moved to World?
        // Let's keep store here for V4 compatibility, pointed to by RuntimeContext.
        
        // std::unique_ptr<Backend> backend; // Moved to WorldState
        std::vector<ID> node_stack; // Transient, candidate for FrameState
        
        // RuntimeState runtime; // Replaced by FrameState mostly
        // We will keep RuntimeState as a deprecated member or alias if possible, 
        // to minimize changes in fanta.cpp for now.
        // Actually, let's Replace RuntimeState definition with one that references FrameState
        
        RuntimeState runtime; // We need to keep this struct definition for fanta.cpp access
        
        InputContext input_ctx;
        ThemeContext theme_ctx; // Maybe redundant with WorldState.current_theme
        
        EngineContext(); // Constructor to link things up
    };

    // Internal Accessors (implemented in fanta.cpp)
    EngineContext& GetEngineContext(); // Phase 31: Replaces direct g_ctx access
    StateStore& GetStore();
    RuntimeState& GetRuntime();
    InputContext& GetInput();
    ThemeContext& GetThemeCtx();

} // namespace internal
} // namespace fanta
