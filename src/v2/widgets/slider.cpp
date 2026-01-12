// Fantasmagorie v2 - Slider Implementation
#include "slider.hpp"
#include <algorithm>

namespace fanta {

void SliderBuilder::build() {
    if (built_ || !g_ctx || !value_) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(label_);
    NodeHandle n = g_ctx->get(id);
    
    apply_constraints(n);
    
    n.style().bg = Color::Hex(0x333333FF);
    n.style().corner_radius = 4.0f;
    
    n.input().clickable = true;
    n.input().hoverable = true;
    
    // Calculate normalized position
    float range = max_ - min_;
    float normalized = (range > 0) ? (*value_ - min_) / range : 0;
    normalized = std::clamp(normalized, 0.0f, 1.0f);
    
    // Handle drag with undo support
    if (is_clicked(id)) {
        auto& layout = n.layout();
        float local_x = g_ctx->mouse_pos.x - layout.x;
        float new_norm = std::clamp(local_x / layout.w, 0.0f, 1.0f);
        float new_value = min_ + new_norm * range;
        
        if (undoable_ && new_value != *value_) {
            auto cmd = std::make_unique<ValueChangeCommand<float>>(
                value_, *value_, new_value, label_
            );
            CommandStack::instance().push(std::move(cmd));
        } else {
            *value_ = new_value;
        }
    }
    
    n.render().text = std::to_string(int(*value_));
    
    g_ctx->end_node();
}

} // namespace fanta
