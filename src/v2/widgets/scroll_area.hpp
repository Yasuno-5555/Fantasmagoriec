// Fantasmagorie v2 - ScrollArea Widget
// Scrollable container with clipping
#pragma once

#include "builder_base.hpp"

namespace fanta {

class ScrollAreaBuilder : public ContainerBuilder<ScrollAreaBuilder> {
public:
    explicit ScrollAreaBuilder(const char* id) : id_(id) {}
    
    ScrollAreaBuilder& horizontal(bool h = true) { horizontal_ = h; return *this; }
    ScrollAreaBuilder& vertical(bool v = true) { vertical_ = v; return *this; }
    ScrollAreaBuilder& show_scrollbar(bool s = true) { show_scrollbar_ = s; return *this; }
    
    void build();
    ~ScrollAreaBuilder() { if (!built_) build(); }
    
private:
    const char* id_;
    bool horizontal_ = false;
    bool vertical_ = true;
    bool show_scrollbar_ = true;
};

inline ScrollAreaBuilder ScrollArea(const char* id) {
    return ScrollAreaBuilder(id);
}

} // namespace fanta
