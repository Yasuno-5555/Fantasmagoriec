// Fantasmagorie v2 - Procedural Icon Renderer
#include "draw_list.hpp"
#include <cmath>

namespace fanta {

void DrawList::add_icon(float x, float y, float size, IconType icon, uint32_t color) {
    // Center point
    float cx = x + size * 0.5f;
    float cy = y + size * 0.5f;
    float r = size * 0.5f;
    
    // Helper for relative drawing
    auto line = [&](float px1, float py1, float px2, float py2) {
        add_line(x + px1 * size, y + py1 * size, 
                 x + px2 * size, y + py2 * size, color, 2.0f);
    };
    
    switch (icon) {
        case IconType::Check: {
            // Check mark (✔)
            line(0.2f, 0.5f, 0.4f, 0.7f);
            line(0.4f, 0.7f, 0.8f, 0.3f);
            break;
        }
        case IconType::Close: {
            // X mark
            float p = 0.2f;
            line(p, p, 1.0f - p, 1.0f - p);
            line(1.0f - p, p, p, 1.0f - p);
            break;
        }
        case IconType::Menu: {
            // Hamburger
            line(0.1f, 0.2f, 0.9f, 0.2f);
            line(0.1f, 0.5f, 0.9f, 0.5f);
            line(0.1f, 0.8f, 0.9f, 0.8f);
            break;
        }
        case IconType::ChevronRight: {
            // >
            line(0.3f, 0.2f, 0.7f, 0.5f);
            line(0.7f, 0.5f, 0.3f, 0.8f);
            break;
        }
        case IconType::ChevronDown: {
            // V
            line(0.2f, 0.3f, 0.5f, 0.7f);
            line(0.5f, 0.7f, 0.8f, 0.3f);
            break;
        }
        case IconType::Settings: {
            // Circle + Lines
            add_arc(cx, cy, r * 0.6f, 0, 6.28f, color, 2.0f);
            // Too complex for simple lines? Just a minimal representation
            add_arc(cx, cy, r * 0.3f, 0, 6.28f, color, 2.0f);
            break;
        }
        case IconType::Search: {
            // Magnifier
            float mr = r * 0.7f;
            add_arc(x + mr, y + mr, mr * 0.8f, 0, 6.28f, color, 2.0f);
            line(0.7f, 0.7f, 1.0f, 1.0f);
            break;
        }
        case IconType::User: {
            // Head + Body
            add_arc(cx, y + size * 0.3f, size * 0.2f, 0, 6.28f, color, 2.0f);
            add_arc(cx, y + size * 0.9f, size * 0.4f, 3.14f * 1.2f, 3.14f * 1.8f, color, 2.0f);
            break;
        }
        case IconType::Info: {
            // i
            add_rect(cx - 1, y + size * 0.1f, 2, 2, color);
            line(0.5f, 0.3f, 0.5f, 0.8f);
            break;
        }
        case IconType::Warning: {
            // Triangle + ! (Triangle requires 3 lines)
            line(0.5f, 0.1f, 0.9f, 0.9f);
            line(0.9f, 0.9f, 0.1f, 0.9f);
            line(0.1f, 0.9f, 0.5f, 0.1f);
            // Exclamation
            line(0.5f, 0.3f, 0.5f, 0.6f);
            add_rect(cx - 1, y + size * 0.75f, 2, 2, color);
            break;
        }
    }
}

} // namespace fanta
