// Fantasmagorie v2 - Checkbox Widget
#pragma once

#include "widget_base.hpp"

namespace fanta {

class CheckboxBuilder {
public:
    explicit CheckboxBuilder(const char* label) : label_(label) {}
    
    CheckboxBuilder& bind(bool* value) { value_ = value; return *this; }
    
    void build();
    ~CheckboxBuilder() { if (!built_) build(); }
    
private:
    const char* label_;
    bool* value_ = nullptr;
    bool built_ = false;
};

inline CheckboxBuilder Checkbox(const char* label) {
    return CheckboxBuilder(label);
}

} // namespace fanta
