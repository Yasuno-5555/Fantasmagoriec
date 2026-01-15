#pragma once
#include <string>
#include <vector>
#include <queue>
#include "../core/ui_context.hpp"
#include "focus.hpp"

enum class EventType {
    KeyDown,
    KeyUp,
    Text,
    MouseBtn,
    MouseMove,
    Scroll
};

struct InputEvent {
    EventType type;
    int key = 0;        // GLFW key code
    int action = 0;     // GLFW action
    int mods = 0;       // GLFW mods
    std::string text;   // UTF-8
    float x = 0;
    float y = 0;
    
    bool consumed = false;
};

struct InputDispatcher {
    std::vector<InputEvent> event_queue;
    std::unordered_map<int, bool> key_state;
    
    void push_key(int key, int action, int mods);
    void push_text(const std::string& text);
    
    bool is_key_down(int key) const {
        auto it = key_state.find(key);
        return it != key_state.end() && it->second;
    }
    
    // Phase 18: Added window handle for Clipboard
    void dispatch(UIContext& ctx, FocusManager& focus, void* window_handle);
};
