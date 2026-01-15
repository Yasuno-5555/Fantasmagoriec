#pragma once

#include "../core/types_internal.hpp"
#include <vector>
#include <cmath>
#include <chrono>

namespace fanta {
namespace internal {

// Touch Point
struct TouchPoint {
    int id;
    Vec2 position;
    Vec2 start_position;
    Vec2 velocity;
    bool active = false;
    std::chrono::steady_clock::time_point start_time;
};

// Gesture Types
enum class GestureType {
    None,
    Tap,
    DoubleTap,
    LongPress,
    Pan,
    Pinch,
    Rotate,
    Swipe
};

enum class SwipeDirection {
    None, Left, Right, Up, Down
};

// Gesture Event
struct GestureEvent {
    GestureType type = GestureType::None;
    Vec2 position = {0, 0};
    Vec2 delta = {0, 0};
    float scale = 1.0f;          // For pinch
    float rotation = 0.0f;       // For rotate (radians)
    float velocity = 0.0f;       // For swipe
    SwipeDirection swipe_dir = SwipeDirection::None;
    int touch_count = 0;
};

// Gesture Recognizer State
struct GestureState {
    std::vector<TouchPoint> touches;
    GestureEvent current_gesture;
    GestureType active_gesture = GestureType::None;
    
    // Pinch state
    float initial_distance = 0;
    float current_distance = 0;
    
    // Rotate state
    float initial_angle = 0;
    float current_angle = 0;
    
    // Timing thresholds
    float tap_max_duration = 0.3f;       // seconds
    float long_press_duration = 0.5f;
    float double_tap_interval = 0.3f;
    float swipe_min_velocity = 500.0f;   // pixels/sec
    
    // Last tap info for double-tap
    std::chrono::steady_clock::time_point last_tap_time;
    Vec2 last_tap_position = {0, 0};
    
    void add_touch(int id, float x, float y) {
        for (auto& t : touches) {
            if (t.id == id && !t.active) {
                t.position = {x, y};
                t.start_position = {x, y};
                t.velocity = {0, 0};
                t.active = true;
                t.start_time = std::chrono::steady_clock::now();
                update_gesture_state();
                return;
            }
        }
        touches.push_back({id, {x, y}, {x, y}, {0, 0}, true, std::chrono::steady_clock::now()});
        update_gesture_state();
    }
    
    void update_touch(int id, float x, float y) {
        for (auto& t : touches) {
            if (t.id == id && t.active) {
                Vec2 old_pos = t.position;
                t.position = {x, y};
                t.velocity = {x - old_pos.x, y - old_pos.y};  // Per frame
                break;
            }
        }
        update_gesture_state();
    }
    
    void remove_touch(int id) {
        for (auto& t : touches) {
            if (t.id == id) {
                t.active = false;
                break;
            }
        }
        resolve_gesture();
        update_gesture_state();
    }
    
    int active_touch_count() const {
        int count = 0;
        for (const auto& t : touches) {
            if (t.active) count++;
        }
        return count;
    }
    
    void update_gesture_state() {
        int count = active_touch_count();
        current_gesture.touch_count = count;
        
        if (count == 2) {
            // Calculate pinch distance
            std::vector<TouchPoint*> active;
            for (auto& t : touches) {
                if (t.active) active.push_back(&t);
            }
            
            if (active.size() >= 2) {
                float dx = active[1]->position.x - active[0]->position.x;
                float dy = active[1]->position.y - active[0]->position.y;
                current_distance = std::sqrt(dx * dx + dy * dy);
                current_angle = std::atan2(dy, dx);
                
                if (active_gesture == GestureType::None) {
                    initial_distance = current_distance;
                    initial_angle = current_angle;
                    active_gesture = GestureType::Pinch;  // Or Rotate
                }
                
                current_gesture.scale = current_distance / initial_distance;
                current_gesture.rotation = current_angle - initial_angle;
            }
        } else if (count == 1) {
            for (auto& t : touches) {
                if (t.active) {
                    current_gesture.position = t.position;
                    current_gesture.delta = {
                        t.position.x - t.start_position.x,
                        t.position.y - t.start_position.y
                    };
                    break;
                }
            }
        }
    }
    
    void resolve_gesture() {
        // Called when touch ends
        if (active_touch_count() == 0 && !touches.empty()) {
            auto& last = touches.back();
            auto now = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(now - last.start_time).count();
            
            float dx = last.position.x - last.start_position.x;
            float dy = last.position.y - last.start_position.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (duration < tap_max_duration && dist < 20.0f) {
                // Check for double tap
                float tap_interval = std::chrono::duration<float>(now - last_tap_time).count();
                if (tap_interval < double_tap_interval) {
                    current_gesture.type = GestureType::DoubleTap;
                } else {
                    current_gesture.type = GestureType::Tap;
                }
                last_tap_time = now;
                last_tap_position = last.position;
            } else if (std::sqrt(last.velocity.x * last.velocity.x + last.velocity.y * last.velocity.y) * 60.0f > swipe_min_velocity) {
                // Swipe
                current_gesture.type = GestureType::Swipe;
                if (std::abs(last.velocity.x) > std::abs(last.velocity.y)) {
                    current_gesture.swipe_dir = last.velocity.x > 0 ? SwipeDirection::Right : SwipeDirection::Left;
                } else {
                    current_gesture.swipe_dir = last.velocity.y > 0 ? SwipeDirection::Down : SwipeDirection::Up;
                }
            }
            
            active_gesture = GestureType::None;
            touches.clear();
        }
    }
};

// Gesture Recognizer Logic
struct GestureLogic {
    static void Update(GestureState& state, float dt);
    static bool IsPinching();
    static bool IsSwiping();
    static GestureEvent GetCurrentGesture();
};

// Global accessor
GestureState& GetGestures();

} // namespace internal
} // namespace fanta
