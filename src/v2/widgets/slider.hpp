// Fantasmagorie v2 - Slider Widget
#pragma once

#include "builder_base.hpp"

namespace fanta {

class SliderBuilder : public BuilderBase<SliderBuilder> {
public:
    explicit SliderBuilder(const char* label) : label_(label) { width_ = 200; height_ = 24; }
    
    SliderBuilder& range(float min, float max) { min_ = min; max_ = max; return *this; }
    SliderBuilder& bind(float* value) { value_ = value; return *this; }
    
    void build();
    ~SliderBuilder() { if (!built_) build(); }
    
private:
    const char* label_;
    float* value_ = nullptr;
    float min_ = 0, max_ = 1;
};

inline SliderBuilder Slider(const char* label) {
    return SliderBuilder(label);
}

} // namespace fanta
