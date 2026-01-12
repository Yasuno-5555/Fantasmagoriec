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
    
    ButtonBuilder& width(float w) { width_ = w; return *this; }
    ButtonBuilder& height(float h) { height_ = h; return *this; }
    ButtonBuilder& on_click(std::function<void()> fn) { on_click_ = fn; return *this; }
    ButtonBuilder& primary() { is_primary_ = true; return *this; }
    ButtonBuilder& danger() { is_danger_ = true; return *this; }
    
    void build();
    ~ButtonBuilder() { if (!built_) build(); }
    
private:
    const char* label_;
    float width_ = 0;
    float height_ = 0;
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
