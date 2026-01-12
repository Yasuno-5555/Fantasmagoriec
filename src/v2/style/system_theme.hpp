// Fantasmagorie v2 - System Theme Integration
// OS dark/light mode detection and accent color sync
#pragma once

#include "theme.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace fanta {

// ============================================================================
// System Theme Detection
// ============================================================================

inline bool is_system_dark_mode() {
#ifdef _WIN32
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, 
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        DWORD value = 1;
        DWORD size = sizeof(value);
        RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, nullptr, 
                        (LPBYTE)&value, &size);
        RegCloseKey(key);
        return value == 0;  // 0 = dark mode
    }
#endif
    return true;  // Default to dark
}

inline uint32_t get_system_accent_color() {
#ifdef _WIN32
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\DWM",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        DWORD color = 0;
        DWORD size = sizeof(color);
        if (RegQueryValueExW(key, L"AccentColor", nullptr, nullptr,
                            (LPBYTE)&color, &size) == ERROR_SUCCESS) {
            RegCloseKey(key);
            // Windows stores as ABGR, convert to RGBA
            uint8_t a = (color >> 24) & 0xFF;
            uint8_t b = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t r = color & 0xFF;
            return (r << 24) | (g << 16) | (b << 8) | a;
        }
        RegCloseKey(key);
    }
#endif
    return 0x4A90D9FF;  // Default blue
}

// ============================================================================
// Auto Theme Selection
// ============================================================================

template<typename DarkTheme, typename LightTheme>
inline void sync_system_theme() {
    if (is_system_dark_mode()) {
        use_theme<DarkTheme>();
    } else {
        use_theme<LightTheme>();
    }
}

inline void auto_theme() {
    sync_system_theme<MaterialTheme, MaterialTheme>();
    
    // Apply system accent
    if (current_theme()) {
        current_theme()->accent = Color::Hex(get_system_accent_color());
    }
}

} // namespace fanta
