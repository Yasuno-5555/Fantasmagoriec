#include "platform/native_bridge.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fanta {
namespace internal {

class DesktopNativeBridge : public NativePlatformBridge {
public:
    void haptic_feedback(int intensity) override {
        std::cout << "[Native] Haptic Feedback (Intensity: " << intensity << ")" << std::endl;
        // Windows Desktop doesn't have system-wide haptics easily, 
        // but we could use XInput for controllers if needed.
    }
    
    void show_toast(const std::string& message) override {
        std::cout << "[Native] Toast: " << message << std::endl;
#ifdef _WIN32
        // Just a simple message box as dummy toast for Windows dev
        // MessageBoxA(NULL, message.c_str(), "Fantasmagorie Toast", MB_OK | MB_ICONINFORMATION);
#endif
    }

    void announce_message(const std::string& message) override {
        std::cout << "[Native] Accessibility Announcement: " << message << std::endl;
    }
    
    std::string get_app_version() override { return "Dev-0.1.0"; }
    
    bool is_dark_mode() override {
        // Simple heuristic or registry check on Windows
        return true; 
    }
};

// Global initializer for desktop
static DesktopNativeBridge g_desktop_bridge;
struct DesktopInitializer {
    DesktopInitializer() { SetNativeBridge(&g_desktop_bridge); }
};
static DesktopInitializer g_init;

} // namespace internal
} // namespace fanta
