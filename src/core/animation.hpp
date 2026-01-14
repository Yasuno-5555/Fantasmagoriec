#pragma once
#include <cmath>
#include <algorithm>

namespace fanta {
namespace internal {

struct SpringConfig {
    float stiffness;
    float damping;
    
    static SpringConfig Default() { return { 150.0f, 15.0f }; }
    static SpringConfig Bouncy() { return { 120.0f, 8.0f }; }
    static SpringConfig Stiff() { return { 300.0f, 25.0f }; }
};

struct AnimState {
    float value = 0.0f;
    float velocity = 0.0f;
    float target = 0.0f;
    bool initialized = false;
    
    void init(float v) {
        if (!initialized) {
            value = target = v;
            velocity = 0.0f;
            initialized = true;
        }
    }
    
    void update(float dt, const SpringConfig& config) {
        // Physics step
        // F = -k*x - c*v
        float displacement = value - target;
        float force = -config.stiffness * displacement - config.damping * velocity;
        
        velocity += force * dt;
        value += velocity * dt;
        
        // Snap to target if very close
        if (std::abs(displacement) < 0.01f && std::abs(velocity) < 0.01f) {
            value = target;
            velocity = 0.0f;
        }
    }
};

} // internal
} // fanta
