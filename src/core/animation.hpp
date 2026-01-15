#pragma once
#include <cmath>
#include <algorithm>
#include "core/types_core.hpp"

namespace fanta {
namespace internal {

    // Stateless Animation: ID -> State
    struct AnimationState {
        // We support animating up to 4 float values per ID (e.g. RGBA, Rect, or just one float)
        // For simplicity in Phase N, let's target RGBA Color transitions.
        
        float current[4] = {0,0,0,0};
        float target[4] = {0,0,0,0};
        float velocity[4] = {0,0,0,0}; // For spring
        
        bool active = false; // Is currently animating?
        
        // Configuration
        float stiffness = 150.0f;
        float damping = 20.0f;
        float mass = 1.0f;
        
        // Helper to set target (wakes up animation if changed)
        void set_target(float v0, float v1, float v2, float v3) {
            float eps = 0.001f;
            if (std::abs(target[0] - v0) > eps || 
                std::abs(target[1] - v1) > eps || 
                std::abs(target[2] - v2) > eps || 
                std::abs(target[3] - v3) > eps) {
                
                target[0] = v0; target[1] = v1; target[2] = v2; target[3] = v3;
                active = true;
            }
        }
        
        // Update Step (Spring Physics)
        void update(float dt) {
            if (!active) return;
            
            bool still_moving = false;
            float eps = 0.001f;
            
            for (int i = 0; i < 4; ++i) {
                // Spring force: F = -k(x - target)
                float displacement = current[i] - target[i];
                float force = -stiffness * displacement;
                
                // Damping force: F_d = -c * velocity
                float damp = -damping * velocity[i];
                
                // Acceleration: a = F / m
                float accel = (force + damp) / mass;
                
                // Euler integration
                velocity[i] += accel * dt;
                current[i] += velocity[i] * dt;
                
                // Check if settled
                if (std::abs(displacement) > eps || std::abs(velocity[i]) > eps) {
                    still_moving = true;
                } else {
                    // Snap to target if very close/slow to stop micro-jitter
                    // But be careful not to snap while others are moving? 
                    // Individual snapping is generally okay.
                }
            }
            
            active = still_moving;
            if (!active) {
                for(int i=0; i<4; ++i) { current[i] = target[i]; velocity[i] = 0; }
            }
        }
    };

} // namespace internal
} // namespace fanta
