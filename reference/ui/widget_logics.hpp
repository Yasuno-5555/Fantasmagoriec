#pragma once
#include <fanta.h>
#include "../core/types_internal.hpp"
#include "../core/contexts_internal.hpp"

namespace fanta {
namespace internal {

struct SliderLogic {
    static void Update(ElementState& s, InputContext& input, const ComputedLayout& rect) {
        // V2-D: Get Coordinate-Corrected Mouse Pos
        Vec2 mouse_pos = input.mouse_screen; // Fallback
        auto& runtime = GetRuntime();
        if (runtime.element_mouse_pos.count(s.id)) {
            mouse_pos = runtime.element_mouse_pos.at(s.id);
        }

        // Use Global Capture (V2-B)
        bool is_active_slider = false;

        if (IsCaptured(s.id)) {
            // Already captured -> drag maintained
            is_active_slider = true;
            if (!input.mouse_down) {
                Release();
                is_active_slider = false;
            }
        } else if (s.is_hovered && input.state.is_just_pressed() && !IsAnyCaptured()) {
            Capture(s.id);
            is_active_slider = true;
        }
        
        if (is_active_slider) {
            float t = (mouse_pos.x - rect.x) / rect.w;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            
            float new_val = s.slider_min + t * (s.slider_max - s.slider_min);
            
            // Sync to PersistentData
            auto& p_data = GetStore().persistent_states[s.id];
            p_data.slider_val = new_val;
            s.slider_val = new_val; // Visual sync for this frame
        } else {
            // Pull from PersistentData if available
            auto it = GetStore().persistent_states.find(s.id);
            if (it != GetStore().persistent_states.end()) {
                s.slider_val = it->second.slider_val;
            }
        }
    }
};

struct ToggleLogic {
    static void Update(ElementState& s, InputContext& input) {
        auto& p_data = GetStore().persistent_states[s.id];
        if (IsClicked(s.id) && !IsAnyCaptured()) {
            p_data.toggle_val = !p_data.toggle_val;
        }
        s.toggle_val = p_data.toggle_val; // Visual sync
    }
};

} // namespace internal
} // namespace fanta
