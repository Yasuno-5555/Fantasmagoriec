#include "fanta.h"
#include "platform/native_bridge.hpp"

namespace fanta {
namespace internal {
    static NativePlatformBridge* g_bridge = nullptr;
    void SetNativeBridge(NativePlatformBridge* bridge) { g_bridge = bridge; }
    NativePlatformBridge* GetNativeBridge() { return g_bridge; }
}

namespace Native {
    void HapticFeedback(int intensity) {
        if (auto* b = internal::GetNativeBridge()) b->haptic_feedback(intensity);
    }
    void ShowToast(const std::string& message) {
        if (auto* b = internal::GetNativeBridge()) b->show_toast(message);
    }
    void Announce(const std::string& message) {
        if (auto* b = internal::GetNativeBridge()) b->announce_message(message);
    }
    std::string GetAppVersion() {
        if (auto* b = internal::GetNativeBridge()) return b->get_app_version();
        return "1.0.0";
    }
    bool IsDarkMode() {
        if (auto* b = internal::GetNativeBridge()) return b->is_dark_mode();
        return false;
    }
}
}
