#pragma once
#include "types_internal.hpp"
#include "input.hpp"
#include <string>
#include <vector>
#include <memory>

namespace fanta {
namespace internal {

    // Phase 6.1: Input Context
    struct InputContext {
        InputState state;
        float mouse_x = 0;
        float mouse_y = 0;
        bool mouse_down = false;
        float mouse_wheel = 0;
    };

    // Phase 9: Theme Context
    struct ThemeContext {
        Theme current;
    };

    // Framework Runtime State
    struct RuntimeState {
        int width = 0;
        int height = 0;
        float dt = 0.016f;
        
        bool screenshot_pending = false;
        std::string screenshot_filename;

        std::vector<ID> overlay_ids;
    };

    // Internal Accessors (implemented in fanta.cpp)
    StateStore& GetStore();
    RuntimeState& GetRuntime();
    InputContext& GetInput();
    ThemeContext& GetThemeCtx();

} // namespace internal
} // namespace fanta
