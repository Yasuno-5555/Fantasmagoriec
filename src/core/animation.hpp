#pragma once
#include <cmath>
#include <algorithm>

namespace fanta {
namespace internal {

enum class CurveType { 
    Linear, EaseIn, EaseOut, EaseInOut, 
    ElasticIn, ElasticOut, BounceIn, BounceOut 
};

struct Curves {
    static float apply(CurveType type, float t) {
        switch(type) {
            case CurveType::EaseIn: return t * t;
            case CurveType::EaseOut: return t * (2 - t);
            case CurveType::EaseInOut: return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
            case CurveType::ElasticOut: {
                if (t == 0 || t == 1) return t;
                return std::pow(2.0f, -10.0f * t) * std::sin((t - 0.075f) * (2.0f * 3.14159f) / 0.3f) + 1.0f;
            }
            default: return t;
        }
    }
};

template<typename T>
T Lerp(const T& a, const T& b, float t) {
    return a * (1.0f - t) + b * t;
}

// Specialization for Colors in types_internal could be added later

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
    
    // Bounds for elastic clamping
    bool has_bounds = false;
    float min_val = 0.0f;
    float max_val = 0.0f;
    
    void init(float v) {
        if (!initialized) {
            value = target = v;
            velocity = 0.0f;
            initialized = true;
        }
    }
    
    void set_bounds(float min, float max) {
        has_bounds = true;
        min_val = min;
        max_val = max;
    }
    
    void update(float dt, const SpringConfig& config) {
        if (dt <= 0.0f) return;
        if (dt > 0.1f) dt = 0.1f; // Cap dt for stability

        // Physics step (semi-implicit Euler)
        float displacement = value - target;
        float force = -config.stiffness * displacement - config.damping * velocity;
        
        // Elastic clamping force if outside bounds
        if (has_bounds) {
            if (value < min_val) {
                float dist = min_val - value;
                force += dist * config.stiffness * 2.0f; // Extra stiff pull back
            } else if (value > max_val) {
                float dist = value - max_val;
                force -= dist * config.stiffness * 2.0f;
            }
        }

        velocity += force * dt;
        value += velocity * dt;
        
        // Snap to target if very close and inside bounds
        bool inside = !has_bounds || (value >= min_val && value <= max_val);
        if (inside && std::abs(value - target) < 0.01f && std::abs(velocity) < 0.01f) {
            value = target;
            velocity = 0.0f;
        }
    }
};

enum class ConstraintType { Distance, MinDistance, MaxDistance, StayTop, StayBottom };

struct PhysicsConstraint {
    AnimState* a;
    AnimState* b;
    ConstraintType type;
    float param = 0.0f;
    
    void solve() {
        if (!a || !b) return;
        switch(type) {
            case ConstraintType::Distance:
                b->target = a->value + param;
                break;
            case ConstraintType::MinDistance:
                if (b->value < a->value + param) b->target = a->value + param;
                break;
            case ConstraintType::MaxDistance:
                if (b->value > a->value + param) b->target = a->value + param;
                break;
            case ConstraintType::StayTop:
                if (b->value < param) b->target = param;
                break;
            case ConstraintType::StayBottom:
                if (b->value > param) b->target = param;
                break;
        }
    }
};

} // internal
} // fanta
