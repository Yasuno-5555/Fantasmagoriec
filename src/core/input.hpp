#pragma once

namespace fanta {
namespace internal {

struct InputState {
    float mouse_x = 0;
    float mouse_y = 0; // Relative to client area
    bool mouse_down = false; // Left button
    
    // Phase 6.1: Previous frame state for click detection
    bool prev_mouse_down = false;
    
    // Helpers
    bool is_clicked() const { return !mouse_down && prev_mouse_down; } // Trigger on release
    bool is_pressed() const { return mouse_down; }
    bool is_just_pressed() const { return mouse_down && !prev_mouse_down; }
    
    float mouse_wheel = 0; // Per-frame delta
    
    void update_frame() {
        prev_mouse_down = mouse_down;
        mouse_wheel = 0; // Clear for next frame
    }
};

} // namespace internal
} // namespace fanta
