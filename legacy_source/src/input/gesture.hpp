// Fantasmagorie v2 - Gesture System
// Touch gesture recognition for mobile platforms
#pragma once

#include "core/types.hpp"
#include <vector>
#include <chrono>

namespace fanta {
namespace gesture {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

// ============================================================================
// Touch Point
// ============================================================================

struct TouchPoint {
    int id = 0;
    Vec2 pos;
    Vec2 start_pos;
    Vec2 prev_pos;
    TimePoint start_time;
    bool active = false;
};

// ============================================================================
// Gesture Types
// ============================================================================

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

enum class SwipeDirection { None, Left, Right, Up, Down };

struct GestureEvent {
    GestureType type = GestureType::None;
    Vec2 position;          // Center position
    Vec2 delta;             // Pan delta
    float scale = 1.0f;     // Pinch scale
    float rotation = 0.0f;  // Rotate angle (radians)
    SwipeDirection swipe = SwipeDirection::None;
    int touch_count = 0;
};

// ============================================================================
// Gesture Recognizer
// ============================================================================

class GestureRecognizer {
public:
    static GestureRecognizer& instance() {
        static GestureRecognizer r;
        return r;
    }
    
    // Configuration
    float tap_max_distance = 10.0f;      // Max movement for tap
    float long_press_time_ms = 500.0f;   // Long press threshold
    float double_tap_time_ms = 300.0f;   // Double tap window
    float swipe_min_velocity = 500.0f;   // Pixels per second
    
    // Touch input
    void touch_begin(int id, float x, float y) {
        TouchPoint& t = get_or_create(id);
        t.id = id;
        t.pos = {x, y};
        t.start_pos = {x, y};
        t.prev_pos = {x, y};
        t.start_time = Clock::now();
        t.active = true;
    }
    
    void touch_move(int id, float x, float y) {
        TouchPoint* t = get(id);
        if (t && t->active) {
            t->prev_pos = t->pos;
            t->pos = {x, y};
        }
    }
    
    void touch_end(int id, float x, float y) {
        TouchPoint* t = get(id);
        if (t && t->active) {
            t->pos = {x, y};
            t->active = false;
            process_touch_end(*t);
        }
    }
    
    // Get current gesture
    GestureEvent current_gesture() const { return current_; }
    
    // Check specific gestures
    bool is_tap() const { return current_.type == GestureType::Tap; }
    bool is_double_tap() const { return current_.type == GestureType::DoubleTap; }
    bool is_long_press() const { return current_.type == GestureType::LongPress; }
    bool is_panning() const { return current_.type == GestureType::Pan; }
    bool is_pinching() const { return current_.type == GestureType::Pinch; }
    
    Vec2 pan_delta() const { return current_.delta; }
    float pinch_scale() const { return current_.scale; }
    float rotation() const { return current_.rotation; }
    
    // Update (call each frame)
    void update() {
        current_ = GestureEvent();
        
        int active_count = 0;
        for (const auto& t : touches_) {
            if (t.active) active_count++;
        }
        
        current_.touch_count = active_count;
        
        if (active_count == 1) {
            // Single finger - check for pan
            TouchPoint* t = get_active();
            if (t) {
                current_.position = t->pos;
                current_.delta = t->pos - t->prev_pos;
                
                float dist = distance(t->pos, t->start_pos);
                if (dist > tap_max_distance) {
                    current_.type = GestureType::Pan;
                }
                
                // Check for long press
                auto now = Clock::now();
                auto elapsed = std::chrono::duration<float, std::milli>(now - t->start_time).count();
                if (elapsed > long_press_time_ms && dist < tap_max_distance) {
                    current_.type = GestureType::LongPress;
                }
            }
        } else if (active_count == 2) {
            // Two fingers - check for pinch/rotate
            std::vector<TouchPoint*> active;
            for (auto& t : touches_) {
                if (t.active) active.push_back(&t);
            }
            
            if (active.size() == 2) {
                Vec2 p1 = active[0]->pos;
                Vec2 p2 = active[1]->pos;
                Vec2 prev1 = active[0]->prev_pos;
                Vec2 prev2 = active[1]->prev_pos;
                
                float curr_dist = distance(p1, p2);
                float prev_dist = distance(prev1, prev2);
                
                if (prev_dist > 0) {
                    current_.scale = curr_dist / prev_dist;
                    current_.type = GestureType::Pinch;
                }
                
                current_.position = (p1 + p2) * 0.5f;
            }
        }
    }
    
    void clear() {
        touches_.clear();
        current_ = GestureEvent();
    }
    
private:
    std::vector<TouchPoint> touches_;
    GestureEvent current_;
    TimePoint last_tap_time_;
    Vec2 last_tap_pos_;
    
    TouchPoint& get_or_create(int id) {
        for (auto& t : touches_) {
            if (t.id == id) return t;
        }
        touches_.push_back(TouchPoint());
        return touches_.back();
    }
    
    TouchPoint* get(int id) {
        for (auto& t : touches_) {
            if (t.id == id) return &t;
        }
        return nullptr;
    }
    
    TouchPoint* get_active() {
        for (auto& t : touches_) {
            if (t.active) return &t;
        }
        return nullptr;
    }
    
    float distance(Vec2 a, Vec2 b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx*dx + dy*dy);
    }
    
    void process_touch_end(const TouchPoint& t) {
        float dist = distance(t.pos, t.start_pos);
        auto now = Clock::now();
        auto elapsed = std::chrono::duration<float, std::milli>(now - t.start_time).count();
        
        if (dist < tap_max_distance && elapsed < 200) {
            // Check for double tap
            auto since_last = std::chrono::duration<float, std::milli>(now - last_tap_time_).count();
            float tap_dist = distance(t.pos, last_tap_pos_);
            
            if (since_last < double_tap_time_ms && tap_dist < tap_max_distance * 2) {
                current_.type = GestureType::DoubleTap;
            } else {
                current_.type = GestureType::Tap;
            }
            
            current_.position = t.pos;
            last_tap_time_ = now;
            last_tap_pos_ = t.pos;
        } else if (elapsed < 300) {
            // Check for swipe
            Vec2 delta = t.pos - t.start_pos;
            float velocity = dist / (elapsed / 1000.0f);
            
            if (velocity > swipe_min_velocity) {
                current_.type = GestureType::Swipe;
                current_.position = t.pos;
                
                if (std::abs(delta.x) > std::abs(delta.y)) {
                    current_.swipe = delta.x > 0 ? SwipeDirection::Right : SwipeDirection::Left;
                } else {
                    current_.swipe = delta.y > 0 ? SwipeDirection::Down : SwipeDirection::Up;
                }
            }
        }
    }
};

// ============================================================================
// Convenience
// ============================================================================

inline void touch_begin(int id, float x, float y) { GestureRecognizer::instance().touch_begin(id, x, y); }
inline void touch_move(int id, float x, float y) { GestureRecognizer::instance().touch_move(id, x, y); }
inline void touch_end(int id, float x, float y) { GestureRecognizer::instance().touch_end(id, x, y); }
inline void update() { GestureRecognizer::instance().update(); }

inline bool is_tap() { return GestureRecognizer::instance().is_tap(); }
inline bool is_double_tap() { return GestureRecognizer::instance().is_double_tap(); }
inline bool is_long_press() { return GestureRecognizer::instance().is_long_press(); }
inline Vec2 pan_delta() { return GestureRecognizer::instance().pan_delta(); }
inline float pinch_scale() { return GestureRecognizer::instance().pinch_scale(); }

} // namespace gesture
} // namespace fanta

