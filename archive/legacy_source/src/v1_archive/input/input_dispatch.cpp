#include "input_dispatch.hpp"

void InputDispatcher::push_key(int key, int action, int mods) {
    if (action == 1) key_state[key] = true; // Press
    else if (action == 0) key_state[key] = false; // Release
    // Repeat (2) doesn't change state

    InputEvent e;
    e.type = (action == 1 || action == 2) ? EventType::KeyDown : EventType::KeyUp; // 1=Press, 2=Repeat
    e.key = key;
    e.action = action;
    e.mods = mods;
    event_queue.push_back(e);
}

void InputDispatcher::push_text(const std::string& text) {
    InputEvent e;
    e.type = EventType::Text;
    e.text = text;
    event_queue.push_back(e);
}

#include <GLFW/glfw3.h> // Need GLFW for clipboard

void InputDispatcher::dispatch(UIContext& ctx, FocusManager& focus, void* window_handle) {
    GLFWwindow* window = (GLFWwindow*)window_handle;
    
    for (auto& e : event_queue) {
        if (e.consumed) continue;
        
        NodeID target = focus.focus_id;
        
        // Tab Navigation
        if (e.type == EventType::KeyDown && e.key == GLFW_KEY_TAB) { 
            bool shift = (e.mods & GLFW_MOD_SHIFT); 
            if (shift) focus.tab_prev(ctx);
            else focus.tab_next(ctx);
            e.consumed = true;
            continue;
        }

        // Clipboard
        if (e.type == EventType::KeyDown && (e.mods & GLFW_MOD_CONTROL)) {
            if (e.key == GLFW_KEY_V) { // Paste
                const char* clip = glfwGetClipboardString(window);
                if (clip) {
                    // Inject as Text Event directly downstream
                    // We can't push to queue (iterating it).
                    // Just handle it specially or recurse?
                    // Let's manually trigger on_text_input on target
                    if (target != 0) {
                         NodeHandle node = ctx.get(target);
                          if (ctx.prev_input.count(target) && ctx.prev_input[target].on_text_input) {
                              ctx.prev_input[target].on_text_input(std::string(clip));
                              e.consumed = true;
                          }
                    }
                }
            }
            // Copy
            else if (e.key == GLFW_KEY_C) {
                if (target != 0) {
                     NodeHandle node = ctx.get(target);
                      if (ctx.prev_input.count(target) && ctx.prev_input[target].get_text) {
                          std::string txt = ctx.prev_input[target].get_text();
                          if (!txt.empty()) {
                              glfwSetClipboardString(window, txt.c_str());
                              e.consumed = true;
                          }
                      }
                }
            }
        }
        
        if (e.consumed) continue;

        if (target == 0) continue;
        
        NodeHandle node = ctx.get(target);
        if (!node.is_valid()) continue;
        
        if (e.type == EventType::KeyDown || e.type == EventType::KeyUp) {
            if (ctx.prev_input.count(target) && ctx.prev_input[target].on_key) {
                if (ctx.prev_input[target].on_key(e.key, e.action, e.mods)) {
                    e.consumed = true;
                }
            }
        }
        else if (e.type == EventType::Text) {
            if (ctx.prev_input.count(target) && ctx.prev_input[target].on_text_input) {
                if (ctx.prev_input[target].on_text_input(e.text)) {
                    e.consumed = true;
                }
            }
        }
    }
    
    event_queue.clear();
}
