#include "drag_drop.hpp"
#include "../core/contexts_internal.hpp"

namespace fanta {
namespace internal {

static DragDropState g_drag_drop;

DragDropState& GetDragDrop() {
    return g_drag_drop;
}

void DragDropLogic::Update(InputContext& input, RuntimeState& runtime) {
    auto& dd = GetDragDrop();
    
    if (dd.is_dragging) {
        dd.update_drag(input.mouse_x, input.mouse_y);
        
        // Check for drop (mouse released)
        if (!input.mouse_down) {
            // Find drop target
            for (const auto& [id, layout] : runtime.layout_results) {
                if (input.mouse_x >= layout.x && input.mouse_x < layout.x + layout.w &&
                    input.mouse_y >= layout.y && input.mouse_y < layout.y + layout.h) {
                    dd.drop_target = id;
                    break;
                }
            }
            dd.end_drag();
        }
    }
}

bool DragDropLogic::IsDragActive() {
    return GetDragDrop().is_dragging;
}

const DragPayload* DragDropLogic::GetPayload() {
    auto& dd = GetDragDrop();
    if (dd.is_dragging) {
        return &dd.payload;
    }
    return nullptr;
}

bool DragDropLogic::AcceptsPayload(const std::string& type) {
    auto* payload = GetPayload();
    return payload && payload->type == type;
}

} // namespace internal
} // namespace fanta
