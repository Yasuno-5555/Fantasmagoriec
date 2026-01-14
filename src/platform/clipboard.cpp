#include "clipboard.hpp"

// Forward declare GLFW functions to avoid including GLFW header
struct GLFWwindow;
extern "C" {
    const char* glfwGetClipboardString(GLFWwindow* window);
    void glfwSetClipboardString(GLFWwindow* window, const char* string);
}

// Access to current GLFW window (defined in fanta.cpp)
namespace fanta {
namespace internal {
    GLFWwindow* GetGLFWWindow();
}
}

namespace fanta {

void SetClipboardText(const char* text) {
    auto* window = internal::GetGLFWWindow();
    if (window && text) {
        glfwSetClipboardString(window, text);
    }
}

std::string GetClipboardText() {
    auto* window = internal::GetGLFWWindow();
    if (window) {
        const char* text = glfwGetClipboardString(window);
        if (text) {
            return std::string(text);
        }
    }
    return "";
}

bool HasClipboardText() {
    return !GetClipboardText().empty();
}

} // namespace fanta
