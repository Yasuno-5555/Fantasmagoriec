#include "context_menu.hpp"
#include "../core/contexts_internal.hpp"

// Right mouse button (GLFW)
#define GLFW_MOUSE_BUTTON_RIGHT 1

namespace fanta {
namespace internal {

static ContextMenuState g_context_menu;

ContextMenuState& GetContextMenu() {
    return g_context_menu;
}

void ContextMenuLogic::Update(InputContext& input, RuntimeState& runtime) {
    auto& menu = GetContextMenu();
    
    // Check for right-click to open context menu
    // (Assuming we have right-click detection in input)
    
    if (menu.is_open) {
        // Update hovered item
        float mx = input.mouse_x;
        float my = input.mouse_y;
        
        float item_height = 24.0f;
        float menu_width = 200.0f;
        
        menu.hovered_index = -1;
        
        float y = menu.position.y;
        for (int i = 0; i < static_cast<int>(menu.items.size()); i++) {
            if (menu.items[i].is_separator) {
                y += 8.0f;
                continue;
            }
            
            if (mx >= menu.position.x && mx < menu.position.x + menu_width &&
                my >= y && my < y + item_height) {
                menu.hovered_index = i;
            }
            
            y += item_height;
        }
        
        // Close on click outside
        float menu_height = y - menu.position.y;
        bool outside = mx < menu.position.x || mx > menu.position.x + menu_width ||
                       my < menu.position.y || my > menu.position.y + menu_height;
        
        if (outside && input.state.is_just_pressed()) {
            menu.close();
        }
    }
}

void ContextMenuLogic::Render(ContextMenuState& menu, DrawList& dl) {
    if (!menu.is_open) return;
    
    float item_height = 24.0f;
    float menu_width = 200.0f;
    float padding = 4.0f;
    
    // Calculate menu height
    float menu_height = padding * 2;
    for (const auto& item : menu.items) {
        menu_height += item.is_separator ? 8.0f : item_height;
    }
    
    // Background
    ColorF bg = {0.2f, 0.2f, 0.2f, 0.95f};
    ColorF hover_bg = {0.3f, 0.3f, 0.5f, 1.0f};
    ColorF text = {1.0f, 1.0f, 1.0f, 1.0f};
    ColorF disabled_text = {0.5f, 0.5f, 0.5f, 1.0f};
    
    dl.add_rounded_rect(
        {menu.position.x, menu.position.y},
        {menu_width, menu_height},
        4.0f, bg, 8.0f // elevation
    );
    
    // Items
    float y = menu.position.y + padding;
    for (int i = 0; i < static_cast<int>(menu.items.size()); i++) {
        const auto& item = menu.items[i];
        
        if (item.is_separator) {
            // Draw separator line
            dl.add_line(
                {menu.position.x + padding, y + 4.0f},
                {menu.position.x + menu_width - padding, y + 4.0f},
                1.0f, {0.4f, 0.4f, 0.4f, 1.0f}
            );
            y += 8.0f;
            continue;
        }
        
        // Hover highlight
        if (i == menu.hovered_index && item.enabled) {
            dl.add_rect(
                {menu.position.x + padding, y},
                {menu_width - padding * 2, item_height},
                hover_bg, 0
            );
        }
        
        // TODO: Add text rendering for item.label
        
        y += item_height;
    }
}

bool ContextMenuLogic::HandleClick(ContextMenuState& menu, InputContext& input) {
    if (!menu.is_open) return false;
    
    if (menu.hovered_index >= 0 && menu.hovered_index < static_cast<int>(menu.items.size())) {
        auto& item = menu.items[menu.hovered_index];
        if (item.enabled && !item.is_separator && item.callback) {
            item.callback();
            menu.close();
            return true;
        }
    }
    
    return false;
}

} // namespace internal
} // namespace fanta
