#pragma once

#include "../core/types_internal.hpp"
#include <vector>

namespace fanta {
namespace internal {

// Keyboard Navigation State
struct KeyboardNavState {
    std::vector<ID> focus_chain;  // Ordered list of focusable elements
    int focus_index = -1;         // Current focused element index
    
    void build_chain(const std::vector<ElementState>& elements);
    ID get_focused_id() const;
    void focus_next();
    void focus_prev();
    void focus_first();
    void focus_last();
};

// Keyboard Navigation Logic
struct KeyboardNavLogic {
    static void Update(InputContext& input, const std::vector<ElementState>& elements);
    static bool HandleTabKey(KeyboardNavState& nav, bool shift);
    static bool HandleEnterKey(KeyboardNavState& nav, InputContext& input);
    static bool HandleEscapeKey(KeyboardNavState& nav, InputContext& input);
};

// Global keyboard nav state accessor
KeyboardNavState& GetKeyboardNav();

} // namespace internal
} // namespace fanta
