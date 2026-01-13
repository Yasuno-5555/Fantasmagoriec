// Fantasmagorie v2 - Theme Editor Widget
#pragma once

#include "widget_base.hpp"
#include "button.hpp"
#include "slider.hpp"
#include "label.hpp"
#include "../style/theme.hpp"

namespace fanta {

class ThemeEditorBuilder {
    const char* label_ = "Theme Editor";
    float width_ = 300;
    bool visible_ = true;
    
public:
    ThemeEditorBuilder() = default;
    
    ThemeEditorBuilder& width(float w) { width_ = w; return *this; }
    ThemeEditorBuilder& visible(bool v) { visible_ = v; return *this; }
    
    void build() {
        if (!g_ctx || !visible_) return;
        
        auto theme = current_theme();
        if (!theme) return;
        
        NodeID id = g_ctx->begin_node(label_);
        NodeHandle n = g_ctx->get(id);
        
        // Constraints
        n.constraint().width = width_;
        n.constraint().padding = 12;
        
        // Style
        n.style().bg = Color::Hex(0x252525F0);
        n.style().corner_radius = 8;
        n.style().border.width = 1;
        n.style().border.color = Color::Hex(0x404040FF);
        
        // Layout
        n.layout().dir = LayoutDir::Column;
        n.constraint().gap = 8;
        
        // Header
        Label("Theme Editor").bold().color(0xFFD700FF).height(30).build();
        
        // Motion Settings
        Label("Animation Duration").height(20).build();
        
        static float duration_base = theme->motion.duration_base * 1000; // ms
        Slider("dur_base")
            .range(0, 500)
            .bind(&duration_base)
            .width(width_ - 24)
            .build();
        theme->motion.duration_base = duration_base / 1000.0f;
        
        Label("Fast Duration").height(20).build();
        
        static float duration_fast = theme->motion.duration_fast * 1000;
        Slider("dur_fast")
            .range(0, 200)
            .bind(&duration_fast)
            .width(width_ - 24)
            .build();
        theme->motion.duration_fast = duration_fast / 1000.0f;
        
        // Spacing
        Label("Corner Radius").height(20).build();
        
        static float corner = theme->spacing.corner_medium;
        Slider("corner")
            .range(0, 20)
            .bind(&corner)
            .width(width_ - 24)
            .build();
        theme->spacing.corner_medium = corner;
        theme->spacing.corner_large = corner * 1.5f;
        theme->spacing.corner_small = corner * 0.5f;
        
        g_ctx->end_node();
    }
};

inline ThemeEditorBuilder ThemeEditor() {
    return ThemeEditorBuilder();
}

} // namespace fanta
