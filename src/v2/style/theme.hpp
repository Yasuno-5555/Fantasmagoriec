// Fantasmagorie v2 - Theme System
// Three-tier theming: Minimal, Material, Glass
#pragma once

#include "../core/types.hpp"

namespace fanta {

// ============================================================================
// Theme Base
// ============================================================================

struct Theme {
    // Colors
    Color bg = Color::Hex(0x1A1A1AFF);
    Color fg = Color::White();
    Color accent = Color::Hex(0x4A90D9FF);
    Color border = Color::Hex(0x333333FF);
    Color error = Color::Hex(0xD94A4AFF);
    Color success = Color::Hex(0x4AD94AFF);
    Color warning = Color::Hex(0xD9D94AFF);
    
    // Typography
    float font_size = 14.0f;
    float line_height = 1.4f;
    
    // Spacing
    float padding = 8.0f;
    float margin = 4.0f;
    float corner_radius = 4.0f;
    
    // Animation
    float animation_speed = 8.0f;  // Lower = slower
    
    // Features
    bool enable_shadows = false;
    bool enable_blur = false;
    float shadow_radius = 0;
    float blur_radius = 0;
};

// ============================================================================
// Minimal Theme (ImGui-like)
// ============================================================================

struct MinimalTheme : Theme {
    MinimalTheme() {
        bg = Color::Hex(0x242424FF);
        fg = Color::Hex(0xE0E0E0FF);
        accent = Color::Hex(0x4A90D9FF);
        border = Color::Hex(0x444444FF);
        corner_radius = 2.0f;
        padding = 4.0f;
        enable_shadows = false;
    }
    
    static MinimalTheme Dark() { return MinimalTheme(); }
    
    static MinimalTheme Light() {
        MinimalTheme t;
        t.bg = Color::Hex(0xF0F0F0FF);
        t.fg = Color::Hex(0x202020FF);
        t.border = Color::Hex(0xC0C0C0FF);
        return t;
    }
};

// ============================================================================
// Material Theme (Material Design 3)
// ============================================================================

struct MaterialTheme : Theme {
    // Button variants
    Color button_primary = Color::Hex(0x6750A4FF);
    Color button_secondary = Color::Hex(0x625B71FF);
    Color button_error = Color::Hex(0xB3261EFF);
    
    // Surface colors
    Color surface = Color::Hex(0x1C1B1FFF);
    Color surface_variant = Color::Hex(0x49454FFF);
    Color on_surface = Color::Hex(0xE6E1E5FF);
    
    MaterialTheme() {
        bg = Color::Hex(0x1C1B1FFF);
        fg = Color::Hex(0xE6E1E5FF);
        accent = Color::Hex(0xD0BCFFFF);
        border = Color::Hex(0x49454FFF);
        corner_radius = 12.0f;
        padding = 12.0f;
        enable_shadows = true;
        shadow_radius = 4.0f;
    }
    
    static MaterialTheme Dark() { return MaterialTheme(); }
    
    static MaterialTheme Light() {
        MaterialTheme t;
        t.bg = Color::Hex(0xFFFBFEFF);
        t.fg = Color::Hex(0x1C1B1FFF);
        t.surface = Color::Hex(0xFFFBFEFF);
        t.on_surface = Color::Hex(0x1C1B1FFF);
        t.accent = Color::Hex(0x6750A4FF);
        return t;
    }
};

// ============================================================================
// Glass Theme (Apple-inspired)
// ============================================================================

struct GlassTheme : Theme {
    // Glass effect
    float glass_opacity = 0.7f;
    Color glass_tint = Color::Hex(0x1A1A1A80);
    
    // Depth
    float elevation = 8.0f;
    
    GlassTheme() {
        bg = Color::Hex(0x1A1A1ACC);  // Semi-transparent
        fg = Color::White();
        accent = Color::Hex(0x007AFFFF);  // iOS blue
        border = Color::Hex(0xFFFFFF20);
        corner_radius = 16.0f;
        padding = 16.0f;
        enable_shadows = true;
        enable_blur = true;
        shadow_radius = 16.0f;
        blur_radius = 20.0f;
        animation_speed = 12.0f;
    }
    
    static GlassTheme Dark() { return GlassTheme(); }
    
    static GlassTheme Light() {
        GlassTheme t;
        t.bg = Color::Hex(0xFFFFFFCC);
        t.fg = Color::Black();
        t.glass_tint = Color::Hex(0xFFFFFF80);
        return t;
    }
};

// ============================================================================
// Theme Manager
// ============================================================================

class ThemeManager {
public:
    static ThemeManager& instance() {
        static ThemeManager m;
        return m;
    }
    
    Theme* current() { return current_; }
    
    void set(Theme* theme) { current_ = theme; }
    
    template<typename T>
    void use() {
        static T theme;
        current_ = &theme;
    }
    
private:
    ThemeManager() {
        static MinimalTheme default_theme;
        current_ = &default_theme;
    }
    
    Theme* current_ = nullptr;
};

// Convenience
inline Theme* current_theme() { return ThemeManager::instance().current(); }

template<typename T>
inline void use_theme() { ThemeManager::instance().use<T>(); }

} // namespace fanta
