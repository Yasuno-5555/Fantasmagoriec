#include "text_input.hpp"
#include "../core/contexts_internal.hpp"

// GLFW key codes (subset)
#define GLFW_KEY_BACKSPACE 259
#define GLFW_KEY_DELETE 261
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#define GLFW_KEY_HOME 268
#define GLFW_KEY_END 269
#define GLFW_KEY_A 65
#define GLFW_KEY_C 67
#define GLFW_KEY_V 86
#define GLFW_KEY_X 88

namespace fanta {
namespace internal {

void TextInputLogic::Update(ElementState& el, InputContext& input, const ComputedLayout& layout) {
    if (!el.is_text_input) return;
    
    // Click to focus
    float mx = input.mouse_x;
    float my = input.mouse_y;
    bool inside = (mx >= layout.x && mx < layout.x + layout.w &&
                   my >= layout.y && my < layout.y + layout.h);
    
    if (inside && input.state.is_just_pressed()) {
        input.focused_id = el.id;
        
        auto& p_data = GetStore().persistent_states[el.id];
        p_data.cursor_pos = static_cast<int>(p_data.text_buffer.length());
        p_data.selection_start = -1;
        p_data.selection_end = -1;
    }
    
    el.is_focused = (input.focused_id == el.id);
}

void TextInputLogic::HandleCharInput(ElementState& el, unsigned int codepoint) {
    if (!el.is_text_input || !el.is_focused) return;
    if (codepoint < 32) return; 
    
    auto& p_data = GetStore().persistent_states[el.id];
    if (codepoint < 128) {
        if (p_data.selection_start >= 0) {
            int s = std::min(p_data.selection_start, p_data.selection_end);
            int e = std::max(p_data.selection_start, p_data.selection_end);
            p_data.text_buffer.erase(s, e - s);
            p_data.cursor_pos = s;
            p_data.selection_start = -1;
            p_data.selection_end = -1;
        }
        p_data.text_buffer.insert(p_data.text_buffer.begin() + p_data.cursor_pos, static_cast<char>(codepoint));
        p_data.cursor_pos++;
    }
}

void TextInputLogic::HandleKeyInput(ElementState& el, int key, bool ctrl, bool shift) {
    if (!el.is_text_input || !el.is_focused) return;
    
    auto& p_data = GetStore().persistent_states[el.id];
    auto& buffer = p_data.text_buffer;
    auto& cursor = p_data.cursor_pos;

    switch (key) {
        case GLFW_KEY_BACKSPACE:
            if (cursor > 0) { buffer.erase(cursor - 1, 1); cursor--; }
            break;
        case GLFW_KEY_DELETE:
            if (cursor < (int)buffer.length()) buffer.erase(cursor, 1);
            break;
        case GLFW_KEY_LEFT:
            if (cursor > 0) cursor--;
            break;
        case GLFW_KEY_RIGHT:
            if (cursor < (int)buffer.length()) cursor++;
            break;
        case GLFW_KEY_A:
            if (ctrl) { p_data.selection_start = 0; p_data.selection_end = (int)buffer.length(); cursor = p_data.selection_end; }
            break;
    }
}

} // namespace internal
} // namespace fanta
