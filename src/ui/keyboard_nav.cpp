#include "keyboard_nav.hpp"
#include "../core/contexts_internal.hpp"

// GLFW key codes
#define GLFW_KEY_TAB 258
#define GLFW_KEY_ENTER 257
#define GLFW_KEY_ESCAPE 256
#define GLFW_KEY_LEFT_SHIFT 340
#define GLFW_KEY_RIGHT_SHIFT 344

namespace fanta {
namespace internal {

static KeyboardNavState g_keyboard_nav;

KeyboardNavState& GetKeyboardNav() {
    return g_keyboard_nav;
}

void KeyboardNavState::build_chain(const std::vector<ElementState>& elements) {
    focus_chain.clear();
    for (const auto& el : elements) {
        if (el.is_focusable && !el.is_disabled) {
            focus_chain.push_back(el.id);
        }
    }
}

ID KeyboardNavState::get_focused_id() const {
    if (focus_index >= 0 && focus_index < static_cast<int>(focus_chain.size())) {
        return focus_chain[focus_index];
    }
    return ID{};
}

void KeyboardNavState::focus_next() {
    if (focus_chain.empty()) return;
    focus_index = (focus_index + 1) % static_cast<int>(focus_chain.size());
}

void KeyboardNavState::focus_prev() {
    if (focus_chain.empty()) return;
    focus_index = (focus_index - 1 + static_cast<int>(focus_chain.size())) % static_cast<int>(focus_chain.size());
}

void KeyboardNavState::focus_first() {
    focus_index = focus_chain.empty() ? -1 : 0;
}

void KeyboardNavState::focus_last() {
    focus_index = focus_chain.empty() ? -1 : static_cast<int>(focus_chain.size()) - 1;
}

void KeyboardNavLogic::Update(InputContext& input, const std::vector<ElementState>& elements) {
    auto& nav = GetKeyboardNav();
    nav.build_chain(elements);
    
    // Sync with input context
    if (nav.focus_index >= 0 && nav.focus_index < static_cast<int>(nav.focus_chain.size())) {
        input.focused_id = nav.focus_chain[nav.focus_index];
    }
}

bool KeyboardNavLogic::HandleTabKey(KeyboardNavState& nav, bool shift) {
    if (shift) {
        nav.focus_prev();
    } else {
        nav.focus_next();
    }
    return true;
}

bool KeyboardNavLogic::HandleEnterKey(KeyboardNavState& nav, InputContext& input) {
    // Simulate click on focused element
    ID focused = nav.get_focused_id();
    if (focused.valid()) {
        // The actual click handling happens in element interaction
        return true;
    }
    return false;
}

bool KeyboardNavLogic::HandleEscapeKey(KeyboardNavState& nav, InputContext& input) {
    // Clear focus or close popup
    if (input.focused_id.valid()) {
        input.focused_id = ID{};
        nav.focus_index = -1;
        return true;
    }
    return false;
}

} // namespace internal
} // namespace fanta
