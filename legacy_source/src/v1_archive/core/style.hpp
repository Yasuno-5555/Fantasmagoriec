#pragma once
#include <cstdint>

namespace ui {
    struct Color { 
        float r,g,b,a; 
        static Color Hex(uint32_t hex) {
            return { ((hex>>24)&0xFF)/255.0f, ((hex>>16)&0xFF)/255.0f, ((hex>>8)&0xFF)/255.0f, (hex&0xFF)/255.0f };
        }
        uint32_t u32() const {
            return ((uint32_t)(r*255)<<24) | ((uint32_t)(g*255)<<16) | ((uint32_t)(b*255)<<8) | (uint32_t)(a*255);
        }
    };

    struct Style {
        Color bg = {0,0,0,0};
        Color bg_hover = {0,0,0,0};
        Color bg_active = {0,0,0,0};
        Color text_color = {1,1,1,1};
        Color border_color = {0,0,0,0};
        float border_width = 0.0f;
        float radius = 0.0f;
        float padding = 0.0f; 
        
        bool show_focus_ring = false;
        float focus_ring_width = 0.0f;
        Color focus_ring_color = {0,0,0,0};
        Color accent_color = {0.0f, 0.6f, 1.0f, 1.0f}; // Electric Blue
        
        // Phase 6: Shadows
        Color shadow_color = {0,0,0,0.6f};
        float shadow_offset_x = 0.0f; // Center shadow for elevation
        float shadow_offset_y = 4.0f;
        float shadow_radius = 16.0f; // Soft
        
        // Phase 1 Expansion
        Color menu_bg = {0.1f, 0.11f, 0.14f, 0.95f}; // Deep Glass
        Color popup_bg = {0.1f, 0.11f, 0.14f, 0.98f};
        Color splitter_color = {0.1f, 0.1f, 0.1f, 1.0f};
        Color splitter_hover_color = {0.3f, 0.3f, 0.3f, 1.0f};
        Color item_hover_bg = {1.0f, 1.0f, 1.0f, 0.1f}; // White overlay
        
        // Phase Pro: Deep Theme
        float frame_padding = 8.0f;
        float item_spacing = 4.0f;
        float scrollbar_width = 10.0f;
        float scrollbar_rounding = 4.0f;
        
        Color scrollbar_bg = {0.0f, 0.0f, 0.0f, 0.5f};
        Color scrollbar_grab = {0.4f, 0.4f, 0.4f, 1.0f};
        Color scrollbar_grab_hover = {0.6f, 0.6f, 0.6f, 1.0f};
        
        Color tab_unfocused = {0.2f, 0.2f, 0.2f, 1.0f};
        Color tab_focused = {0.4f, 0.4f, 0.4f, 1.0f};
        
        Color separator_color = {0.5f, 0.5f, 0.5f, 1.0f};
        
        // Helpers
        uint32_t bg_u32() const { return col_to_u32(bg); }
        // ...
    private:
        static uint32_t col_to_u32(Color c) {
            uint32_t r = (uint32_t)(c.r * 255);
            uint32_t g = (uint32_t)(c.g * 255);
            uint32_t b = (uint32_t)(c.b * 255);
            uint32_t a = (uint32_t)(c.a * 255);
            return (r << 24) | (g << 16) | (b << 8) | a;
        }
    };
}
using ui::Style; // Global alias for UIContext? Or use ui::Style everywhere.
// ui_context.hpp uses Style. 
// If I use `ui::Style` in ui_context.hpp, I need namespace.
