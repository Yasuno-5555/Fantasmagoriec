#pragma once
#include "spring_physics.hpp"
#include <algorithm>

namespace fanta {
namespace internal {

template<typename T>
struct SpringPropertyAnimation {
    T current_value{};
    T velocity{};
    T target_value{};
    SpringConfig config;
    bool active = false;
    bool initialized = false;

    void init(const T& v) {
        if (!initialized) {
            current_value = target_value = v;
            velocity = T{};
            initialized = true;
        }
    }

    void update(float dt) {
        if (!active && initialized) return;
        if (dt <= 0.0f) return;
        if (dt > 0.05f) dt = 0.05f;

        // F = -k(x - target) - c(v)
        T displacement = current_value + (target_value * -1.0f);
        T force = (displacement * -config.stiffness) + (velocity * -config.damping);
        
        velocity = velocity + (force * (dt / config.mass));
        current_value = current_value + (velocity * dt);

        // Check for rest
        // We need a way to check if T is roughly at target
        // For float/vec/color we can use a generic epsilon check if we assume they support a 'Length' or similar
        // For simplicity, let's just keep active if velocity is significant
        bool at_rest = true; 
        // Heuristic: sum of absolute differences
        // (This works for float, Vec2, ColorF if they have operator[] or we specialize)
        // For now, let's just use constant active for a bit
        active = true; 
    }

    void animate_to(const T& target) {
        target_value = target;
        active = true;
    }

    void set_immediate(const T& v) {
        current_value = target_value = v;
        velocity = T{};
        active = false;
        initialized = true;
    }
};

// Specialized for float for easier epsilon checks
template<>
struct SpringPropertyAnimation<float> {
    SpringState state;
    float target = 0.0f;
    SpringConfig config;

    void init(float v) {
        if (!state.is_resting) return;
        state.value = target = v;
        state.velocity = 0.0f;
    }

    void update(float dt) {
        SpringSolver::Solve(target, state, config, dt);
    }

    void animate_to(float v) {
        target = v;
        state.is_resting = false;
    }

    float value() const { return state.value; }
};

} // internal
} // fanta
