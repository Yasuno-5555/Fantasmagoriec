#include "menu_bar.hpp"
#include "context_menu.hpp"  // Reuse MenuItem
#include "../core/contexts_internal.hpp"

namespace fanta {
namespace internal {

static MenuBarState g_menu_bar;

MenuBarState& GetMenuBar() {
    return g_menu_bar;
}

void MenuBarLogic::Update(MenuBarState& state, InputContext& input, float screen_width) {
    float mx = input.mouse_x;
    float my = input.mouse_y;
    
    // Check if mouse is in menu bar area
    bool in_bar = my >= 0 && my < state.bar_height;
    
    if (in_bar) {
        // Find which menu is hovered
        float x = 0;
        float menu_padding = 16.0f;
        
        for (int i = 0; i < static_cast<int>(state.menus.size()); i++) {
            float label_width = state.menus[i].first.length() * 8.0f;  // Approximate
            float menu_width = label_width + menu_padding * 2;
            
            if (mx >= x && mx < x + menu_width) {
                // Mouse over this menu
                if (input.state.is_just_pressed() || state.is_open()) {
                    state.active_menu = i;
                }
            }
            
            x += menu_width;
        }
    }
    
    // Update hovered item in dropdown
    if (state.is_open()) {
        // TODO: Calculate dropdown bounds and find hovered item
    }
    
    // Close on click outside
    if (input.state.is_just_pressed() && !in_bar && !state.is_open()) {
        state.close();
    }
    
    (void)screen_width;
}

void MenuBarLogic::Render(const MenuBarState& state, DrawList& dl, float screen_width) {
    // Menu bar background
    ColorF bar_bg = {0.15f, 0.15f, 0.15f, 1.0f};
    ColorF hover_bg = {0.25f, 0.25f, 0.25f, 1.0f};
    ColorF text_color = {1.0f, 1.0f, 1.0f, 1.0f};
    
    dl.add_rect({0, 0}, {screen_width, state.bar_height}, bar_bg, 0);
    
    // Menu items
    float x = 0;
    float menu_padding = 16.0f;
    float text_y = (state.bar_height - 14.0f) / 2.0f;  // Centered text
    
    for (int i = 0; i < static_cast<int>(state.menus.size()); i++) {
        const auto& menu = state.menus[i];
        float label_width = menu.first.length() * 8.0f;
        float menu_width = label_width + menu_padding * 2;
        
        // Highlight active menu
        if (i == state.active_menu) {
            dl.add_rect({x, 0}, {menu_width, state.bar_height}, hover_bg, 0);
        }
        
        // TODO: Render menu label text
        
        // If this menu is open, render dropdown
        if (i == state.active_menu && !menu.second.empty()) {
            float dropdown_width = 200.0f;
            float item_height = 24.0f;
            float dropdown_height = menu.second.size() * item_height + 8.0f;
            
            ColorF dropdown_bg = {0.2f, 0.2f, 0.2f, 0.98f};
            dl.add_rounded_rect(
                {x, state.bar_height},
                {dropdown_width, dropdown_height},
                4.0f, dropdown_bg, 8.0f
            );
            
            // TODO: Render dropdown items
        }
        
        x += menu_width;
    }
}

bool MenuBarLogic::HandleClick(MenuBarState& state, InputContext& input) {
    if (!state.is_open()) return false;
    
    // Check if clicking on a menu item
    if (state.hovered_item >= 0) {
        const auto& menu = state.menus[state.active_menu];
        if (state.hovered_item < static_cast<int>(menu.second.size())) {
            const auto& item = menu.second[state.hovered_item];
            if (item.enabled && !item.is_separator && item.callback) {
                item.callback();
                state.close();
                return true;
            }
        }
    }
    
    return false;
}

} // namespace internal
} // namespace fanta
