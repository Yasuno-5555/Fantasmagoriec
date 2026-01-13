// Fantasmagorie v2 - Checkbox Widget
#pragma once

#include "widget_base.hpp"

namespace fanta {

class CheckboxBuilder {
public:
    explicit CheckboxBuilder(const char* label) : label_(label) {}
    
    // Composition helpers
    CheckboxBuilder& width(float w) { common.set_width(w); return *this; }
    CheckboxBuilder& height(float h) { common.set_height(h); return *this; }
    
    // Widget specific
    CheckboxBuilder& bind(bool* value) { value_ = value; return *this; }
    
    // Animation
    CheckboxBuilder& animate_on(InteractionState state, std::function<void(ResolvedStyle&)> fn) {
        common.add_modifier(state, fn);
        return *this;
    }
    
    void build();
    ~CheckboxBuilder() { if (!built_) build(); }
    
private:
    const char* label_;
    WidgetCommon common;
    
    bool* value_ = nullptr;
    bool built_ = false;
};

inline CheckboxBuilder Checkbox(const char* label) {
    return CheckboxBuilder(label);
}

} // namespace fanta
