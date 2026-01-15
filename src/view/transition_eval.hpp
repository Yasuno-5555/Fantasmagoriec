#pragma once
#include "view/definitions.hpp"
#include "core/context.hpp"
#include "core/animation.hpp"

namespace fanta {
namespace ui {

    // Transition Evaluation Helper
    // Checks if an animation exists for this View ID.
    // If so, updates the animation target to match the 'current frame' view property.
    // Then returns the *animated* value to be used for rendering.
    
    // Usage: 
    // internal::ColorF bg = EvaluateTransition(view->id, view->bg_color);
    
    inline internal::ColorF EvaluateTransitionColor(internal::ID id, internal::ColorF target_color) {
        auto& ctx = internal::GetEngineContext();
        auto& anims = ctx.persistent.animations;
        
        // 0. Check if animation entry exists or needs creation?
        // We only care if we want to animate. 
        // For Phase N, let's say ALL buttons animate? Or we need a flag?
        // The Prompt says: "User writes simple code, system interpolates".
        // Let's assume automatic transition for now if the ID is valid.
        
        if (id.value == 0) return target_color; // No ID, no state.
        
        // Phase 10: Animation Kill Switch
        if (ctx.config.disable_animation) return target_color;

        auto it = anims.find(id.value);
        if (it == anims.end()) {
            // First time seeing this ID. Just initialize.
            internal::AnimationState state;
            state.current[0] = target_color.r;
            state.current[1] = target_color.g;
            state.current[2] = target_color.b;
            state.current[3] = target_color.a;
            
            state.target[0] = target_color.r; 
            state.target[1] = target_color.g; 
            state.target[2] = target_color.b; 
            state.target[3] = target_color.a;
            
            anims[id.value] = state;
            return target_color;
        }
        
        // Entry exists. Update target and return current.
        internal::AnimationState& state = it->second;
        state.set_target(target_color.r, target_color.g, target_color.b, target_color.a);
        
        // Note: Update(dt) normally happens once per frame globally.
        // If we call .update() here, we might update multiple times if called multiple times?
        // Ideally, we iterate `anims` at start of frame.
        // But we don't know the *Tasks* (Targets) until we see the View AST.
        // So we must set targets here.
        // The Update step should happen separately.
        
        // For simplicity in this step, let's assume Update() is called by the Engine Loop before RenderUI?
        // No, RenderUI is the main loop.
        // Let's call update() right here? No, that depends on tree traversal order.
        // Correct approach:
        // 1. Traverse View AST (Layout/Render). Update TARGETS in AnimationState.
        // 2. Separate System iterates ALL AnimationStates and steps them by DT.
        // 3. Render uses CURRENT values.
        
        // BUT rendering happens during traversal.
        // So:
        // Frame T:
        //   Render walks tree.
        //   Sets Target = Red.
        //   Reads Current (Blue -> Purple). Draws Purple.
        // End Frame:
        //   AnimationSystem steps all physics (Purple -> Red).
        
        return internal::ColorF{state.current[0], state.current[1], state.current[2], state.current[3]};
    }

}
}
