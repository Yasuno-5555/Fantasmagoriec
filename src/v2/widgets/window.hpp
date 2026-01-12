// Fantasmagorie v2 - Window Widget
// Draggable, resizable window container
#pragma once

#include "builder_base.hpp"

namespace fanta {

class WindowBuilder : public ContainerBuilder<WindowBuilder> {
public:
    explicit WindowBuilder(const char* title) : title_(title) {
        width_ = 400;
        height_ = 300;
    }
    
    WindowBuilder& title(const char* t) { title_ = t; return *this; }
    WindowBuilder& size(float w, float h) { width_ = w; height_ = h; return *this; }
    WindowBuilder& closable(bool c = true) { closable_ = c; return *this; }
    WindowBuilder& resizable(bool r = true) { resizable_ = r; return *this; }
    WindowBuilder& draggable(bool d = true) { draggable_ = d; return *this; }
    WindowBuilder& on_close(std::function<void()> fn) { on_close_ = fn; return *this; }
    
    void build();
    ~WindowBuilder() { if (!built_) build(); }
    
private:
    const char* title_;
    bool closable_ = true;
    bool resizable_ = false;
    bool draggable_ = true;
    std::function<void()> on_close_;
};

inline WindowBuilder Window(const char* title) {
    return WindowBuilder(title);
}

} // namespace fanta
