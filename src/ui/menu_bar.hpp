#pragma once

#include "../core/types_internal.hpp"
#include <string>
#include <vector>
#include <functional>

namespace fanta {
namespace internal {

// Forward declaration
struct MenuItem;

// Menu Bar State
struct MenuBarState {
    std::vector<std::pair<std::string, std::vector<MenuItem>>> menus;
    int active_menu = -1;          // Currently open menu (-1 = none)
    int hovered_item = -1;         // Hovered item in active menu
    float bar_height = 22.0f;
    
    void add_menu(const std::string& label) {
        menus.push_back({label, {}});
    }
    
    void add_item(const std::string& menu_label, const MenuItem& item) {
        for (auto& menu : menus) {
            if (menu.first == menu_label) {
                menu.second.push_back(item);
                return;
            }
        }
    }
    
    void close() {
        active_menu = -1;
        hovered_item = -1;
    }
    
    bool is_open() const { return active_menu >= 0; }
};

// Menu Bar Logic
struct MenuBarLogic {
    static void Update(MenuBarState& state, InputContext& input, float screen_width);
    static void Render(const MenuBarState& state, DrawList& dl, float screen_width);
    static bool HandleClick(MenuBarState& state, InputContext& input);
};

// Global menu bar accessor
MenuBarState& GetMenuBar();

} // namespace internal
} // namespace fanta
