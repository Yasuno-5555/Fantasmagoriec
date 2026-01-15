#pragma once

#include "../core/types_internal.hpp"
#include <string>
#include <algorithm>

namespace fanta {
namespace internal {

// TextInput State (stored in ElementState)
struct TextInputState {
    std::string buffer;
    int cursor_pos = 0;
    int selection_start = -1;  // -1 = no selection
    int selection_end = -1;
    bool ime_composing = false;
    std::string ime_composition;
    
    void insert_char(char c) {
        if (selection_start >= 0) delete_selection();
        buffer.insert(buffer.begin() + cursor_pos, c);
        cursor_pos++;
    }
    
    void insert_string(const std::string& s) {
        if (selection_start >= 0) delete_selection();
        buffer.insert(cursor_pos, s);
        cursor_pos += static_cast<int>(s.length());
    }
    
    void delete_char_before() {
        if (selection_start >= 0) {
            delete_selection();
        } else if (cursor_pos > 0) {
            buffer.erase(cursor_pos - 1, 1);
            cursor_pos--;
        }
    }
    
    void delete_char_after() {
        if (selection_start >= 0) {
            delete_selection();
        } else if (cursor_pos < static_cast<int>(buffer.length())) {
            buffer.erase(cursor_pos, 1);
        }
    }
    
    void delete_selection() {
        if (selection_start < 0) return;
        int start = std::min(selection_start, selection_end);
        int end = std::max(selection_start, selection_end);
        buffer.erase(start, end - start);
        cursor_pos = start;
        selection_start = -1;
        selection_end = -1;
    }
    
    void move_cursor(int delta, bool shift_held) {
        int new_pos = std::clamp(cursor_pos + delta, 0, static_cast<int>(buffer.length()));
        
        if (shift_held) {
            if (selection_start < 0) {
                selection_start = cursor_pos;
            }
            selection_end = new_pos;
        } else {
            selection_start = -1;
            selection_end = -1;
        }
        cursor_pos = new_pos;
    }
    
    void select_all() {
        selection_start = 0;
        selection_end = static_cast<int>(buffer.length());
        cursor_pos = selection_end;
    }
    
    std::string get_selected_text() const {
        if (selection_start < 0) return "";
        int start = std::min(selection_start, selection_end);
        int end = std::max(selection_start, selection_end);
        return buffer.substr(start, end - start);
    }
    
    bool has_selection() const {
        return selection_start >= 0 && selection_start != selection_end;
    }
};

// TextInput Logic (update per frame)
struct TextInputLogic {
    static void Update(ElementState& el, InputContext& input, const ComputedLayout& layout);
    static void HandleCharInput(ElementState& el, unsigned int codepoint);
    static void HandleKeyInput(ElementState& el, int key, bool ctrl, bool shift);
};

} // namespace internal
} // namespace fanta
