#pragma once
#include <vector>
#include <functional>
#include <unordered_map>

struct Shortcut {
    int key;       // GLFW key code
    int mods;      // GLFW mods (1=Shift, 2=Ctrl, 4=Alt, 8=Super)
    std::function<void()> callback;
};

class ShortcutManager {
public:
    void register_shortcut(int key, int mods, std::function<void()> callback) {
        shortcuts.push_back({key, mods, callback});
    }

    bool process(int key, int action, int mods) {
        if (action != 1) return false; // Only on PRESS (GLFW_PRESS=1)
        
        for (const auto& s : shortcuts) {
            if (s.key == key && s.mods == mods) {
                if (s.callback) {
                    s.callback();
                    return true;
                }
            }
        }
        return false;
    }

    void clear() {
        shortcuts.clear();
    }

private:
    std::vector<Shortcut> shortcuts;
};
