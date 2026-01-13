// Fantasmagorie v2 - Slider Widget
#pragma once

#include "widget_base.hpp"

namespace fanta {

class SliderBuilder {
public:
    explicit SliderBuilder(const char* label) : label_(label) { common.width = 200; common.height = 24; }
    
    // Composition helpers
    SliderBuilder& width(float w) { common.set_width(w); return *this; }
    SliderBuilder& height(float h) { common.set_height(h); return *this; }
    SliderBuilder& padding(float p) { common.set_padding(p); return *this; }
    
    // Widget specific
    SliderBuilder& range(float min, float max) { min_ = min; max_ = max; return *this; }
    SliderBuilder& bind(float* value) { value_ = value; return *this; }
    SliderBuilder& undoable() { undoable_ = true; return *this; }
    
    // Animation
    SliderBuilder& animate_on(InteractionState state, std::function<void(ResolvedStyle&)> fn) {
        common.add_modifier(state, fn);
        return *this;
    }
    
    void build();
    ~SliderBuilder() { if (!built_) build(); }
    
private:
    const char* label_;
    WidgetCommon common;
    
    float* value_ = nullptr;
    float min_ = 0, max_ = 1;
    bool built_ = false;
    bool undoable_ = false;
};

inline SliderBuilder Slider(const char* label) {
    return SliderBuilder(label);
}

} // namespace fanta
