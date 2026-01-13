// Fantasmagorie v2 - Button Widget
#pragma once

#include "widget_base.hpp"
#include <functional>

namespace fanta {

// ============================================================================
// Button Builder
// ============================================================================

class ButtonBuilder {
public:
    explicit ButtonBuilder(const char* label) : label_(label) {}
    
    // Composition helpers
    ButtonBuilder& width(float w) { common.set_width(w); return *this; }
    ButtonBuilder& height(float h) { common.set_height(h); return *this; }
    ButtonBuilder& padding(float p) { common.set_padding(p); return *this; }
    
    // Widget specific
    ButtonBuilder& on_click(std::function<void()> fn) { on_click_ = fn; return *this; }
    ButtonBuilder& primary() { is_primary_ = true; return *this; }
    ButtonBuilder& danger() { is_danger_ = true; return *this; }
    
    // Animation
    ButtonBuilder& animate_on(InteractionState state, std::function<void(ResolvedStyle&)> fn) {
        common.add_modifier(state, fn);
        return *this;
    }
    
    // Immediate mode helper
    bool build_clicked() {
        bool clicked = false;
        on_click_ = [&]() { clicked = true; };
        build();
        return clicked;
    }
    
    void build();
    ~ButtonBuilder() { if (!built_) build(); }
    
private:
    const char* label_;
    WidgetCommon common;
    
    std::function<void()> on_click_;
    bool is_primary_ = false;
    bool is_danger_ = false;
    bool built_ = false;
};

// ============================================================================
// Factory Function
// ============================================================================

inline ButtonBuilder Button(const char* label) {
    return ButtonBuilder(label);
}

} // namespace fanta
