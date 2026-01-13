// Fantasmagorie v2 - Widget Base Implementation
#include "widget_base.hpp"
#include "../core/node.hpp"
#include "../style/theme.hpp"
#include <map>

namespace fanta {

void WidgetCommon::apply(NodeHandle& n) {
    FANTA_ASSERT_CONTEXT();
    
    // Constraints
    if (width > 0) n.constraint().width = width;
    if (height > 0) n.constraint().height = height;
    if (grow > 0) n.constraint().grow = grow;
    if (padding > 0) n.constraint().padding = padding;
    if (gap > 0) n.constraint().gap = gap;
    
    // Input
    if (focusable) n.input().focusable = true;
    
    // Implicit Animation (Visual Modifiers)
    if (!visual_modifiers.empty()) {
        if (!g_ctx) return;
        NodeID id = n.id();
        
        // Access visual state (auto-created by map access)
        VisualState& vs = g_ctx->store.visual_state[id];
        
        // Base style
        const ResolvedStyle& base = g_ctx->store.style[id];
        vs.targets[0] = base; // Default
        
        // Generate targets
        for (auto& [state, func] : visual_modifiers) {
            ResolvedStyle target = base;
            func(target);
            vs.targets[(int)state] = target;
        }
        
        if (!vs.initialized) {
            vs.current = base;
            vs.initialized = true;
        }
    }
}

} // namespace fanta
