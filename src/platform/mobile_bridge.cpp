#include "fanta.h"
#include <iostream>
#include <windows.h> // For OutputDebugString

namespace fanta {
namespace Native {

    void HapticFeedback(int intensity) {
        // Desktop Simulation: Log and maybe simple beep?
        std::string intensityStr = "Medium";
        if (intensity == 0) intensityStr = "Light";
        if (intensity == 2) intensityStr = "Heavy";
        
        std::cout << "[MobileBridge] HapticFeedback: " << intensityStr << std::endl;
        
        // Windows Feedback: Brief Console Beep or nothing
        // Beep(400 + intensity * 200, 50); // Optional: might be annoying
    }

    void ShowToast(const std::string& message) {
        std::cout << "[MobileBridge] Toast: " << message << std::endl;
        // Could technically use a Windows MessageBox or Notification, but console is fine for sim
    }

    void Announce(const std::string& message) {
        std::cout << "[MobileBridge] Accessibility Announce: " << message << std::endl;
        // On Windows, you'd use UIA RaiseAutomationEvent ideally
    }

    std::string GetAppVersion() {
        return "1.0.0-dev (Desktop Sim)";
    }

    bool IsDarkMode() {
        // Simple check for Windows Dark Mode registry key could go here
        // For now hardcode or mock
        return true; 
    }

    void ShowKeyboard(KeyboardType type) {
        std::cout << "[MobileBridge] ShowKeyboard (Type: " << (int)type << ")" << std::endl;
        // On Windows 8+, could try to invoke Touch Keyboard logic, but tricky from generic app
    }

    void HideKeyboard() {
        std::cout << "[MobileBridge] HideKeyboard" << std::endl;
    }

    void SetIMEPos(float x, float y) {
        // Desktop: Wrapper for ImmSetCompositionWindow if we strictly wanted
        // For Mobile Primacy simulation, we just log "Setting IME Rect"
        // std::cout << "[MobileBridge] SetIMEPos: " << x << ", " << y << std::endl;
    }

} // namespace Native
} // namespace fanta
