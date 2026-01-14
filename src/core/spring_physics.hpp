#pragma once
#include <cmath>
#include <algorithm>

namespace fanta {
namespace internal {

struct SpringConfig {
    float stiffness = 170.0f;
    float damping = 26.0f;
    float mass = 1.0f;
    float precision = 0.001f;
};

struct SpringState {
    float value = 0.0f;
    float velocity = 0.0f;
    bool is_resting = true;
};

class SpringSolver {
public:
    static bool Solve(float target, SpringState& state, const SpringConfig& config, float dt) {
        if (state.is_resting && std::abs(state.value - target) < config.precision) {
            state.value = target;
            state.velocity = 0.0f;
            return false;
        }

        // F = -k(x - target) - c(v)
        float x = state.value;
        float v = state.velocity;
        
        // Multi-step integration for stability at high stiffness
        const int steps = 4;
        float h = dt / steps;
        
        for (int i = 0; i < steps; ++i) {
            float f_spring = -config.stiffness * (x - target);
            float f_damping = -config.damping * v;
            float a = (f_spring + f_damping) / config.mass;
            
            v += a * h;
            x += v * h;
        }

        state.value = x;
        state.velocity = v;

        if (std::abs(v) < config.precision && std::abs(x - target) < config.precision) {
            state.value = target;
            state.velocity = 0.0f;
            state.is_resting = true;
        } else {
            state.is_resting = false;
        }

        return !state.is_resting;
    }
};

} // namespace internal
} // namespace fanta
