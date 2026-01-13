// Fantasmagorie v2 - ScrollPanel Widget
#pragma once

#include "widget_base.hpp"
#include "../style/theme.hpp"

namespace fanta {

class ScrollPanelBuilder {
    const char* label_;
    float width_ = 200;
    float height_ = 300;
    float padding_ = 0;
    
public:
    explicit ScrollPanelBuilder(const char* label) : label_(label) {}
    
    // Fluent API
    ScrollPanelBuilder& width(float w) { width_ = w; return *this; }
    ScrollPanelBuilder& height(float h) { height_ = h; return *this; }
    ScrollPanelBuilder& padding(float p) { padding_ = p; return *this; }
    
    // Begin the scroll container
    void begin() {
        if (!g_ctx) return;
        
        NodeID id = g_ctx->begin_node(label_);
        NodeHandle n = g_ctx->get(id);
        
        // Constraints
        n.constraint().width = width_;
        n.constraint().height = height_;
        if (padding_ > 0) n.constraint().padding = padding_;
        
        // Scroll Config
        n.scroll().scrollable = true;
        n.scroll().clip_content = true;
        
        // Input
        n.input().hoverable = true;
        
        // Layout
        n.layout().dir = LayoutDir::Column;
        
        // Style
        auto theme = current_theme();
        n.style().bg = theme->color.surface;
        n.style().corner_radius = theme->spacing.corner_medium;
        
        // DO NOT end_node - user puts content inside
    }
};

inline ScrollPanelBuilder ScrollPanel(const char* label = "scroll") {
    return ScrollPanelBuilder(label);
}

} // namespace fanta
