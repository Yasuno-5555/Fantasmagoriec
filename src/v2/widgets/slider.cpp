#include "slider.hpp"
#include "../style/theme.hpp"
#include "../core/undo.hpp"
#include <algorithm>

namespace fanta {

void SliderBuilder::build() {
    if (built_ || !g_ctx || !value_) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(label_);
    NodeHandle n = g_ctx->get(id);
    
    // Apply common properties
    if (common.width <= 0) common.width = 200;
    if (common.height <= 0) common.height = 24;
    common.focusable = true;
    
    common.apply(n);
    
    auto theme = current_theme();
    n.style().bg = theme->color.surface_variant;
    n.style().corner_radius = theme->spacing.corner_medium;
    
    n.input().clickable = true;
    n.input().hoverable = true;
    
    // Calculate normalized position
    float range = max_ - min_;
    float normalized = (range > 0) ? (*value_ - min_) / range : 0;
    normalized = std::clamp(normalized, 0.0f, 1.0f);
    
    // Handle drag with undo support
    if (is_clicked(id) || is_dragging(id)) {
        auto it = g_ctx->prev_layout.find(id);
        if (it != g_ctx->prev_layout.end()) {
            auto& layout = it->second;
            float local_x = g_ctx->mouse_pos.x - layout.x;
            float new_norm = std::clamp(local_x / layout.w, 0.0f, 1.0f);
            float new_value = min_ + new_norm * range;
            
            if (undoable_ && new_value != *value_ && g_ctx->mouse_released) {
                // Only push to undo stack on release to avoid flooding
                auto cmd = std::make_unique<ValueChangeCommand<float>>(
                    value_, *value_, new_value, label_
                );
                CommandStack::instance().push(std::move(cmd));
            }
            
            
            *value_ = new_value;
        }
    }
    
    // Handle Keyboard Input (Left/Right)
    if (g_ctx->focused_id == id) {
        float step = (max_ - min_) * 0.1f;
        if (step == 0) step = 1.0f;
        
        for (int key : g_ctx->input_keys) {
            if (key == 263) { // Left
                *value_ = std::clamp(*value_ - step, min_, max_);
            } else if (key == 262) { // Right
                *value_ = std::clamp(*value_ + step, min_, max_);
            }
        }
    }
    
    n.render().text = std::to_string(int(*value_));
    
    g_ctx->end_node();
}

} // namespace fanta
