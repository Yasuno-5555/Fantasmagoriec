// Fantasmagorie v3 - Animation System
// Tween, Spring physics, and per-widget state
#pragma once

#include "core/types.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <cmath>

namespace fanta {

// ============================================================================
// Animation Value Types
// ============================================================================

struct AnimatedFloat {
    float current = 0;
    float target = 0;
    float velocity = 0;
    
    bool is_animating(float threshold = 0.001f) const {
        return std::abs(target - current) > threshold || std::abs(velocity) > threshold;
    }
};

struct AnimatedVec2 {
    Vec2 current = {0, 0};
    Vec2 target = {0, 0};
    Vec2 velocity = {0, 0};
    
    bool is_animating(float threshold = 0.001f) const {
        return (target - current).length() > threshold || velocity.length() > threshold;
    }
};

struct AnimatedColor {
    Color current = Color::Clear();
    Color target = Color::Clear();
    
    bool is_animating(float threshold = 0.001f) const {
        return std::abs(target.r - current.r) > threshold ||
               std::abs(target.g - current.g) > threshold ||
               std::abs(target.b - current.b) > threshold ||
               std::abs(target.a - current.a) > threshold;
    }
};

// ============================================================================
// Tween (Time-based interpolation)
// ============================================================================

class Tween {
public:
    using EasingFn = std::function<float(float)>;
    
    Tween() = default;
    
    Tween& duration(float seconds) { duration_ = seconds; return *this; }
    Tween& delay(float seconds) { delay_ = seconds; return *this; }
    Tween& easing(EasingFn fn) { easing_ = fn; return *this; }
    Tween& on_complete(std::function<void()> cb) { on_complete_ = cb; return *this; }
    
    // Start the tween
    void start() {
        elapsed_ = -delay_;
        active_ = true;
    }
    
    // Update and return normalized progress (0-1)
    float update(float dt) {
        if (!active_) return finished_ ? 1.0f : 0.0f;
        
        elapsed_ += dt;
        if (elapsed_ < 0) return 0.0f; // Still in delay
        
        float t = std::min(elapsed_ / duration_, 1.0f);
        float eased = easing_ ? easing_(t) : t;
        
        if (t >= 1.0f) {
            active_ = false;
            finished_ = true;
            if (on_complete_) on_complete_();
        }
        
        return eased;
    }
    
    bool is_active() const { return active_; }
    bool is_finished() const { return finished_; }
    void reset() { elapsed_ = 0; active_ = false; finished_ = false; }
    
    // Interpolate value
    template<typename T>
    static T lerp(const T& from, const T& to, float t);
    
private:
    float duration_ = 0.3f;
    float delay_ = 0;
    float elapsed_ = 0;
    EasingFn easing_ = easing::ease_out_cubic;
    std::function<void()> on_complete_;
    bool active_ = false;
    bool finished_ = false;
};

// Lerp specializations
template<>
inline float Tween::lerp(const float& from, const float& to, float t) {
    return from + (to - from) * t;
}

template<>
inline Vec2 Tween::lerp(const Vec2& from, const Vec2& to, float t) {
    return {from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t};
}

template<>
inline Color Tween::lerp(const Color& from, const Color& to, float t) {
    return Color::lerp(from, to, t);
}

// ============================================================================
// Spring Physics
// ============================================================================

class Spring {
public:
    float stiffness = 170.0f;  // Higher = faster
    float damping = 26.0f;     // Higher = less bouncy
    float mass = 1.0f;
    
    // Presets
    static Spring gentle() { return {120, 14, 1}; }
    static Spring wobbly() { return {180, 12, 1}; }
    static Spring stiff() { return {210, 20, 1}; }
    static Spring slow() { return {280, 60, 1}; }
    static Spring molasses() { return {280, 120, 1}; }
    
    // Update spring for float value
    void update(AnimatedFloat& v, float dt) const {
        float spring_force = -stiffness * (v.current - v.target);
        float damping_force = -damping * v.velocity;
        float acceleration = (spring_force + damping_force) / mass;
        
        v.velocity += acceleration * dt;
        v.current += v.velocity * dt;
    }
    
    // Update spring for Vec2
    void update(AnimatedVec2& v, float dt) const {
        Vec2 spring_force = (v.target - v.current) * stiffness;
        Vec2 damping_force = v.velocity * (-damping);
        Vec2 acceleration = (spring_force + damping_force) * (1.0f / mass);
        
        v.velocity = v.velocity + acceleration * dt;
        v.current = v.current + v.velocity * dt;
    }
    
    // Update spring for Color
    void update(AnimatedColor& v, float dt) const {
        AnimatedFloat r = {v.current.r, v.target.r, 0};
        AnimatedFloat g = {v.current.g, v.target.g, 0};
        AnimatedFloat b = {v.current.b, v.target.b, 0};
        AnimatedFloat a = {v.current.a, v.target.a, 0};
        
        update(r, dt); update(g, dt); update(b, dt); update(a, dt);
        
        v.current = Color(r.current, g.current, b.current, a.current);
    }
};

// ============================================================================
// Widget Animation State
// ============================================================================

struct WidgetAnimState {
    // Common UI states (0.0 to 1.0)
    AnimatedFloat hover_t;
    AnimatedFloat active_t;
    AnimatedFloat focus_t;
    AnimatedFloat disabled_t;
    
    // Position/size
    AnimatedVec2 position;
    AnimatedFloat scale;
    AnimatedFloat rotation;
    AnimatedFloat opacity;
    
    // Colors
    AnimatedColor bg_color;
    AnimatedColor fg_color;
    AnimatedColor border_color;
    
    WidgetAnimState() {
        scale.current = scale.target = 1.0f;
        opacity.current = opacity.target = 1.0f;
    }
    
    // Update all springs
    void update(const Spring& spring, float dt) {
        spring.update(hover_t, dt);
        spring.update(active_t, dt);
        spring.update(focus_t, dt);
        spring.update(disabled_t, dt);
        spring.update(position, dt);
        spring.update(scale, dt);
        spring.update(rotation, dt);
        spring.update(opacity, dt);
        spring.update(bg_color, dt);
        spring.update(fg_color, dt);
        spring.update(border_color, dt);
    }
    
    bool is_animating() const {
        return hover_t.is_animating() || active_t.is_animating() ||
               focus_t.is_animating() || disabled_t.is_animating() ||
               position.is_animating() || scale.is_animating() ||
               rotation.is_animating() || opacity.is_animating() ||
               bg_color.is_animating() || fg_color.is_animating() ||
               border_color.is_animating();
    }
};

// ============================================================================
// Animation Controller (manages all animations)
// ============================================================================

class AnimationController {
public:
    // Get or create widget animation state
    WidgetAnimState& get(const std::string& id) {
        return states_[id];
    }
    
    WidgetAnimState& get(uint64_t id) {
        return states_[std::to_string(id)];
    }
    
    // Update all animations
    void update(float dt) {
        for (auto& [id, state] : states_) {
            state.update(spring_, dt);
        }
        
        // Update tweens
        auto it = tweens_.begin();
        while (it != tweens_.end()) {
            it->second.update(dt);
            if (it->second.is_finished()) {
                it = tweens_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Set spring preset
    void set_spring(const Spring& s) { spring_ = s; }
    const Spring& spring() const { return spring_; }
    
    // Create a named tween
    Tween& tween(const std::string& name) {
        auto& t = tweens_[name];
        t.reset();
        return t;
    }
    
    Tween* get_tween(const std::string& name) {
        auto it = tweens_.find(name);
        return it != tweens_.end() ? &it->second : nullptr;
    }
    
    // Clear all state
    void clear() {
        states_.clear();
        tweens_.clear();
    }
    
    // Stats
    size_t active_animations() const {
        size_t count = 0;
        for (const auto& [id, state] : states_) {
            if (state.is_animating()) ++count;
        }
        return count + tweens_.size();
    }
    
private:
    std::unordered_map<std::string, WidgetAnimState> states_;
    std::unordered_map<std::string, Tween> tweens_;
    Spring spring_ = Spring::gentle();
};

// ============================================================================
// Staggered Animation Helper
// ============================================================================

class StaggeredAnimation {
public:
    StaggeredAnimation& count(int n) { count_ = n; return *this; }
    StaggeredAnimation& stagger(float delay) { stagger_ = delay; return *this; }
    StaggeredAnimation& duration(float d) { duration_ = d; return *this; }
    StaggeredAnimation& easing(Tween::EasingFn fn) { easing_ = fn; return *this; }
    
    void start() { elapsed_ = 0; active_ = true; }
    
    void update(float dt) {
        if (active_) elapsed_ += dt;
    }
    
    // Get progress for item at index (0-1)
    float progress(int index) const {
        float start_time = index * stagger_;
        float local_elapsed = elapsed_ - start_time;
        if (local_elapsed < 0) return 0.0f;
        float t = std::min(local_elapsed / duration_, 1.0f);
        return easing_ ? easing_(t) : t;
    }
    
    bool is_complete() const {
        return elapsed_ >= (count_ - 1) * stagger_ + duration_;
    }
    
private:
    int count_ = 1;
    float stagger_ = 0.05f;
    float duration_ = 0.3f;
    float elapsed_ = 0;
    Tween::EasingFn easing_ = easing::ease_out_cubic;
    bool active_ = false;
};


// ============================================================================
// Convenience: Animated Float (simplified, no template issues)
// ============================================================================

class AnimatedValue {
public:
    AnimatedValue() = default;
    AnimatedValue(float initial) : current_(initial), target_(initial) {}
    
    // Set target (starts animation)
    void set(float value) { target_ = value; }
    
    // Set immediately (no animation)
    void set_immediate(float value) { current_ = target_ = value; velocity_ = 0; }
    
    // Get current animated value
    float get() const { return current_; }
    
    // Get target value
    float target() const { return target_; }
    
    // Update with spring
    void update(const Spring& spring, float dt) {
        float spring_force = -spring.stiffness * (current_ - target_);
        float damping_force = -spring.damping * velocity_;
        float acceleration = (spring_force + damping_force) / spring.mass;
        
        velocity_ += acceleration * dt;
        current_ += velocity_ * dt;
    }
    
    bool is_animating(float threshold = 0.001f) const {
        return std::abs(target_ - current_) > threshold || std::abs(velocity_) > threshold;
    }
    
    // Implicit conversion
    operator float() const { return current_; }
    
private:
    float current_ = 0;
    float target_ = 0;
    float velocity_ = 0;
};

// Type alias for backward compatibility
template<typename T>
using Animated = AnimatedValue;

} // namespace fanta



