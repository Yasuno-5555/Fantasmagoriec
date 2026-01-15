#pragma once

#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "core/ui_context.hpp"
#include "core/text_buffer.hpp"
#include "render/renderer.hpp"
#include "render/font_manager.hpp"
#include "platform/ime_win32.hpp"
#include "input/input_dispatch.hpp"
#include "input/focus.hpp"

class FantasmagorieCore {

public:
    FantasmagorieCore();
    ~FantasmagorieCore();


    bool init(const char* title, int width, int height);
    void run();
    void shutdown();

    // Callbacks
    void on_key(int key, int scancode, int action, int mods);
    void on_scroll(double xoffset, double yoffset);
    void on_input_char(unsigned int codepoint);
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void glfw_char_callback(GLFWwindow* window, unsigned int c);

public:
    GLFWwindow* window = nullptr;
    
    // Systems
    Renderer renderer;
    FontAtlas atlas;
    Font font;
    ImeHandler ime;
    InputDispatcher input;
    FocusManager focus;
    UIContext ctx;
    
    FT_Library ft_lib = nullptr;
    int width=0, height=0;

    // State
    TextBuffer text_buf;
    double scroll_offset = 0;

    
    // Helper to build UI

    void build_draw_subtree(NodeID id, DrawList& dl, float ox, float oy, float dt);
};

