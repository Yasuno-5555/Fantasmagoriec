// Fantasmagorie v2 - Label Widget
#pragma once

#include "widget_base.hpp"

namespace fanta {

// ============================================================================
// Label Builder
// ============================================================================

class LabelBuilder {
public:
    explicit LabelBuilder(const char* text) : text_(text) {}
    
    LabelBuilder& color(uint32_t c) { color_ = c; return *this; }
    LabelBuilder& bold() { bold_ = true; return *this; }
    
    void build();
    ~LabelBuilder() { if (!built_) build(); }
    
private:
    const char* text_;
    uint32_t color_ = 0xFFFFFFFF;
    bool bold_ = false;
    bool built_ = false;
};

// Factory
inline LabelBuilder Label(const char* text) {
    return LabelBuilder(text);
}

// Simple text helper
inline void Text(const char* text) {
    Label(text).build();
}

} // namespace fanta
