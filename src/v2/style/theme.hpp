// Fantasmagorie v2 - Theme System
// Three-tier theming: Minimal, Material, Glass
#pragma once

#include "../core/types.hpp"

namespace fanta {

// ============================================================================
// Theme Base
// ============================================================================

struct Theme {
    // 1. Colors
    struct Colors {
        Color bg = Color::Hex(0x1A1A1AFF);
        Color fg = Color::White();
        Color accent = Color::Hex(0x4A90D9FF);
        Color border = Color::Hex(0x333333FF);
        
        // Semantic Colors
        Color surface = Color::Hex(0x1C1B1FFF);
        Color surface_variant = Color::Hex(0x49454FFF);
        
        Color primary = Color::Hex(0x4A90D9FF);
        Color secondary = Color::Hex(0x625B71FF);
        
        Color error = Color::Hex(0xD94A4AFF);
        Color success = Color::Hex(0x4AD94AFF);
        Color warning = Color::Hex(0xD9D94AFF);
    } color;
    
    // 2. Typography
    struct Typography {
        float font_size = 14.0f;
        float line_height = 1.4f;
         // Future: font families, weights
    } type;
    
    // 3. Spacing / Layout
    struct Spacing {
        float padding_small = 4.0f;
        float padding_medium = 8.0f;
        float padding_large = 16.0f;
        
        float margin = 4.0f;
        
        float corner_small = 2.0f;
        float corner_medium = 4.0f;
        float corner_large = 8.0f;
    } spacing;
    
    // 4. Motion
    struct Motion {
        float duration_fast = 0.1f;
        float duration_base = 0.2f;
        float duration_slow = 0.4f;
        
        Easing easing_standard = Easing::EaseOut;
    } motion;
    
    // 5. Visual Effects
    struct Visuals {
        bool enable_shadows = false;
        bool enable_blur = false;
        
        float shadow_radius_base = 4.0f;
        float blur_radius_base = 10.0f;
    } effect;
};

// ============================================================================
// Minimal Theme (ImGui-like)
// ============================================================================

struct MinimalTheme : Theme {
    MinimalTheme() {
        color.bg = Color::Hex(0x242424FF);
        color.fg = Color::Hex(0xE0E0E0FF);
        color.accent = Color::Hex(0x4A90D9FF);
        color.border = Color::Hex(0x444444FF);
        
        spacing.corner_medium = 2.0f;
        spacing.padding_medium = 4.0f;
        
        effect.enable_shadows = false;
    }
    // ... Factory methods omitted for brevity, logic is same
};

// ============================================================================
// Material Theme (Material Design 3)
// ============================================================================

struct MaterialTheme : Theme {
    MaterialTheme() {
        color.bg = Color::Hex(0x1C1B1FFF);
        color.fg = Color::Hex(0xE6E1E5FF);
        
        color.primary = Color::Hex(0xD0BCFFFF);
        color.secondary = Color::Hex(0xCCC2DCFF);
        
        color.surface = Color::Hex(0x1C1B1FFF);
        color.border = Color::Hex(0x49454FFF);
        
        spacing.corner_medium = 12.0f;
        spacing.padding_medium = 12.0f;
        
        effect.enable_shadows = true;
        effect.shadow_radius_base = 4.0f;
        
        motion.easing_standard = Easing::EaseOut;
    }
};

// ============================================================================
// Glass Theme (Apple-inspired)
// ============================================================================

struct GlassTheme : Theme {
    GlassTheme() {
        color.bg = Color::Hex(0x1A1A1ACC);  // Semi-transparent
        color.fg = Color::White();
        color.accent = Color::Hex(0x007AFFFF);  // iOS blue
        color.border = Color::Hex(0xFFFFFF20);
        
        spacing.corner_medium = 16.0f;
        spacing.padding_medium = 16.0f;
        
        effect.enable_shadows = true;
        effect.enable_blur = true;
        effect.shadow_radius_base = 16.0f;
        effect.blur_radius_base = 20.0f;
        
        motion.duration_base = 0.35f; // Slower, fluid
        motion.easing_standard = Easing::EaseInOut;
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
