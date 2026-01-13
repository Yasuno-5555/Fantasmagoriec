// Fantasmagorie v2 - Icon Widget
#pragma once

#include "widget_base.hpp"
#include "../render/draw_list.hpp" // For IconType

namespace fanta {

class Icon {
public:
    Icon(IconType type) : type_(type) {}
    
    // Fluent composition
    Icon& size(float s) { common.width = s; common.height = s; return *this; }
    Icon& color(uint32_t c) { color_ = c; return *this; }
    Icon& color(Color c) { color_ = c.u32(); return *this; }
    
    // Spacing
    Icon& padding(float p) { common.padding = p; return *this; }
    
    void build();

private:
    IconType type_;
    WidgetCommon common; // Composition
    uint32_t color_ = 0xFFFFFFFF;
};

} // namespace fanta
