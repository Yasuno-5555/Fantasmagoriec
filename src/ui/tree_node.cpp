#include "tree_node.hpp"
#include "../core/contexts_internal.hpp"

namespace fanta {
namespace internal {

void TreeNodeLogic::Update(ElementState& el, InputContext& input, const ComputedLayout& layout) {
    if (!el.is_tree_node) return;
    
    float mx = input.mouse_x;
    float my = input.mouse_y;
    
    // Arrow hitbox (left side)
    float arrow_x = layout.x + GetIndent(el.tree_node_state.depth);
    float arrow_size = GetArrowSize();
    
    bool hit_arrow = (mx >= arrow_x && mx < arrow_x + arrow_size &&
                      my >= layout.y && my < layout.y + layout.h);
    
    // Full row hitbox
    bool hit_row = (mx >= layout.x && mx < layout.x + layout.w &&
                    my >= layout.y && my < layout.y + layout.h);
    
    el.is_hovered = hit_row;
    
    if (hit_arrow && input.state.is_just_pressed() && el.tree_node_state.has_children) {
        el.tree_node_state.toggle();
    } else if (hit_row && input.state.is_just_pressed()) {
        input.focused_id = el.id;
        
        // Selection logic
        if (!IsKeyDown(0x10)) { // Shift
            for (auto& store_el : GetStore().elements) {
                store_el.is_selected = false;
            }
        }
        el.is_selected = true;
    }
    
    el.is_focused = (input.focused_id == el.id);
}

} // namespace internal
} // namespace fanta
