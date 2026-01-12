// Fantasmagorie v2 - Mobile Platform
// DPI scaling and mobile-specific utilities
#pragma once

#include "../core/types.hpp"
#include <cmath>

namespace fanta {
namespace mobile {

// ============================================================================
// Screen Info
// ============================================================================

struct ScreenInfo {
    float width = 0;
    float height = 0;
    float dpi = 160.0f;       // Default Android mdpi
    float scale = 1.0f;       // UI scale factor
    bool is_portrait = true;
    bool has_notch = false;
    float safe_area_top = 0;
    float safe_area_bottom = 0;
    float safe_area_left = 0;
    float safe_area_right = 0;
};

inline ScreenInfo g_screen;

// ============================================================================
// DPI Scaling
// ============================================================================

inline float dp_to_px(float dp) {
    return dp * (g_screen.dpi / 160.0f) * g_screen.scale;
}

inline float px_to_dp(float px) {
    return px / (g_screen.dpi / 160.0f) / g_screen.scale;
}

inline float sp_to_px(float sp) {
    // sp = scale-independent pixels (for text)
    return dp_to_px(sp);
}

// ============================================================================
// Standard Sizes (in dp)
// ============================================================================

namespace size {
    constexpr float touch_target = 48.0f;    // Minimum touch target
    constexpr float button_height = 48.0f;
    constexpr float icon_small = 24.0f;
    constexpr float icon_medium = 32.0f;
    constexpr float icon_large = 48.0f;
    constexpr float padding_small = 8.0f;
    constexpr float padding_medium = 16.0f;
    constexpr float padding_large = 24.0f;
    constexpr float margin = 16.0f;
}

// ============================================================================
// Safe Area
// ============================================================================

inline Rect safe_area() {
    return Rect(
        g_screen.safe_area_left,
        g_screen.safe_area_top,
        g_screen.width - g_screen.safe_area_left - g_screen.safe_area_right,
        g_screen.height - g_screen.safe_area_top - g_screen.safe_area_bottom
    );
}

// ============================================================================
// Haptic Feedback
// ============================================================================

enum class HapticType {
    Light,
    Medium,
    Heavy,
    Selection,
    Success,
    Warning,
    Error
};

inline void haptic(HapticType type) {
    // Platform-specific implementation
    // Android: VibrationEffect
    // iOS: UIImpactFeedbackGenerator
    (void)type;
}

// ============================================================================
// Device Detection
// ============================================================================

enum class DeviceType { Phone, Tablet, Desktop };

inline DeviceType device_type() {
    float diagonal = std::sqrt(g_screen.width * g_screen.width + 
                               g_screen.height * g_screen.height);
    float diagonal_inches = diagonal / g_screen.dpi;
    
    if (diagonal_inches > 10.0f) return DeviceType::Desktop;
    if (diagonal_inches > 7.0f) return DeviceType::Tablet;
    return DeviceType::Phone;
}

inline bool is_phone() { return device_type() == DeviceType::Phone; }
inline bool is_tablet() { return device_type() == DeviceType::Tablet; }
inline bool is_desktop() { return device_type() == DeviceType::Desktop; }

// ============================================================================
// Orientation
// ============================================================================

inline bool is_portrait() { return g_screen.is_portrait; }
inline bool is_landscape() { return !g_screen.is_portrait; }

inline void set_orientation(float width, float height) {
    g_screen.width = width;
    g_screen.height = height;
    g_screen.is_portrait = height > width;
}

} // namespace mobile
} // namespace fanta
