#pragma once
#include <string>

namespace fanta {
namespace internal {

    // Internal interface to be implemented by .cpp/.mm files
    struct NativePlatformBridge {
        virtual ~NativePlatformBridge() = default;
        virtual void haptic_feedback(int intensity) = 0;
        virtual void show_toast(const std::string& message) = 0;
        virtual void announce_message(const std::string& message) = 0;
        virtual std::string get_app_version() = 0;
        virtual bool is_dark_mode() = 0;
    };

    void SetNativeBridge(NativePlatformBridge* bridge);
    NativePlatformBridge* GetNativeBridge();

} // namespace internal
} // namespace fanta
