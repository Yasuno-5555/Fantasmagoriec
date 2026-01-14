#include "backend/backend.hpp"
#include <GLES3/gl3.h> // Common on Android/iOS
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

namespace fanta {

class GLESBackend : public Backend {
public:
    GLESBackend() = default;
    virtual ~GLESBackend() { shutdown(); }

    bool init(int width, int height, const char* title) override {
        if (!glfwInit()) return false;
        
        // Request GLES context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!window) return false;
        
        glfwMakeContextCurrent(window);
        
        // GLES doesn't use GLAD in the same way, usually 
        // linked directly or using a simple loader if needed.
        
        std::cout << "[GLES] Loaded OpenGL ES 3.0" << std::endl;
        return true;
    }

    void shutdown() override {
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool is_running() override {
        return !glfwWindowShouldClose(window);
    }

    void begin_frame(int w, int h) override {
        glfwPollEvents();
        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        (void)w; (void)h;
    }

    void end_frame() override {
        glfwSwapBuffers(window);
    }

    void render(const fanta::internal::DrawList& draw_list) override {
        // GLES specific rendering logic (similar to GL3 but with precision qualifiers in shaders)
        (void)draw_list;
    }

    void get_mouse_state(float& x, float& y, bool& down, float& wheel) override {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        x = (float)mx; y = (float)my;
        down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        wheel = 0;
    }

    Capabilities capabilities() const override { return { false }; }
    bool capture_screenshot(const char* filename) override { (void)filename; return false; }

private:
    GLFWwindow* window = nullptr;
};

std::unique_ptr<Backend> CreateGLESBackend() {
    return std::make_unique<GLESBackend>();
}

} // namespace fanta
