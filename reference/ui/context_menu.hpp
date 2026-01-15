#pragma once

#include "../core/types_internal.hpp"
#include <string>
#include <vector>
#include <functional>

namespace fanta {
namespace internal {

// Menu Item
struct MenuItem {
    std::string label;
    std::string shortcut;      // e.g., "Ctrl+C"
    std::function<void()> callback;
    bool enabled = true;
    bool checked = false;
    bool is_separator = false;
    std::vector<MenuItem> submenu;  // For nested menus
    
    static MenuItem Separator() {
        MenuItem item;
        item.is_separator = true;
        return item;
    }
};

// Context Menu State
struct ContextMenuState {
    bool is_open = false;
    Vec2 position = {0, 0};
    ID target_id{};
    std::vector<MenuItem> items;
    int hovered_index = -1;
    int submenu_open = -1;  // Which submenu is open
    
    void open(float x, float y, ID target) {
        is_open = true;
        position = {x, y};
        target_id = target;
        hovered_index = -1;
        submenu_open = -1;
    }
    
    void close() {
        is_open = false;
        items.clear();
        target_id = ID{};
    }
    
    void add_item(const std::string& label, std::function<void()> callback, const std::string& shortcut = "") {
        items.push_back({label, shortcut, callback, true, false, false, {}});
    }
    
    void add_separator() {
        items.push_back(MenuItem::Separator());
    }
};

// Context Menu Logic
struct ContextMenuLogic {
    static void Update(InputContext& input, RuntimeState& runtime);
    static void Render(ContextMenuState& menu, DrawList& dl);
    static bool HandleClick(ContextMenuState& menu, InputContext& input);
};

// Global context menu accessor
ContextMenuState& GetContextMenu();

} // namespace internal
} // namespace fanta
