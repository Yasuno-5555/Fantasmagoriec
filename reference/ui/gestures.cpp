#include "gestures.hpp"

namespace fanta {
namespace internal {

static GestureState g_gestures;

GestureState& GetGestures() {
    return g_gestures;
}

void GestureLogic::Update(GestureState& state, float dt) {
    // Check for long press
    if (state.active_touch_count() == 1) {
        for (auto& t : state.touches) {
            if (t.active) {
                auto now = std::chrono::steady_clock::now();
                float duration = std::chrono::duration<float>(now - t.start_time).count();
                
                float dx = t.position.x - t.start_position.x;
                float dy = t.position.y - t.start_position.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                
                if (duration > state.long_press_duration && dist < 20.0f && 
                    state.active_gesture == GestureType::None) {
                    state.current_gesture.type = GestureType::LongPress;
                    state.current_gesture.position = t.position;
                    state.active_gesture = GestureType::LongPress;
                } else if (dist > 20.0f && state.active_gesture == GestureType::None) {
                    state.active_gesture = GestureType::Pan;
                    state.current_gesture.type = GestureType::Pan;
                }
                break;
            }
        }
    }
    
    (void)dt;
}

bool GestureLogic::IsPinching() {
    return GetGestures().active_gesture == GestureType::Pinch;
}

bool GestureLogic::IsSwiping() {
    return GetGestures().current_gesture.type == GestureType::Swipe;
}

GestureEvent GestureLogic::GetCurrentGesture() {
    return GetGestures().current_gesture;
}

} // namespace internal
} // namespace fanta
