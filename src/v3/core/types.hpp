// Fantasmagorie v3 - Core Types
// Minimal, pod-like types for maximum performance
#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace fanta {

// ============================================================================
// Vec2
// ============================================================================
struct Vec2 {
    float x = 0, y = 0;
    
    constexpr Vec2() = default;
    constexpr Vec2(float x_, float y_) : x(x_), y(y_) {}
    
    Vec2 operator+(Vec2 v) const { return {x + v.x, y + v.y}; }
    Vec2 operator-(Vec2 v) const { return {x - v.x, y - v.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    
    float length() const { return std::sqrt(x * x + y * y); }
    float dot(Vec2 v) const { return x * v.x + y * v.y; }
};

// ============================================================================
// Vec4 (for radius, colors as floats)
// ============================================================================
struct Vec4 {
    float x = 0, y = 0, z = 0, w = 0;
    
    constexpr Vec4() = default;
    constexpr Vec4(float x_, float y_, float z_, float w_) 
        : x(x_), y(y_), z(z_), w(w_) {}
};

// ============================================================================
// Rect (logical pixels)
// ============================================================================
struct Rect {
    float x = 0, y = 0, w = 0, h = 0;
    
    constexpr Rect() = default;
    constexpr Rect(float x_, float y_, float w_, float h_) 
        : x(x_), y(y_), w(w_), h(h_) {}
    
    Vec2 center() const { return {x + w * 0.5f, y + h * 0.5f}; }
    Vec2 size() const { return {w, h}; }
    bool contains(Vec2 p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
};

// ============================================================================
// Color (RGBA, 0-1 float, premultiplied alpha optional)
// ============================================================================
struct Color {
    float r = 0, g = 0, b = 0, a = 1;
    
    constexpr Color() = default;
    constexpr Color(float r_, float g_, float b_, float a_ = 1.0f)
        : r(r_), g(g_), b(b_), a(a_) {}
    
    // From 0xRRGGBBAA hex
    static constexpr Color Hex(uint32_t rgba) {
        return Color(
            ((rgba >> 24) & 0xFF) / 255.0f,
            ((rgba >> 16) & 0xFF) / 255.0f,
            ((rgba >> 8) & 0xFF) / 255.0f,
            (rgba & 0xFF) / 255.0f
        );
    }
    
    // Premultiply alpha
    Color premultiplied() const {
        return Color(r * a, g * a, b * a, a);
    }
    
    // Common colors
    static constexpr Color White() { return {1, 1, 1, 1}; }
    static constexpr Color Black() { return {0, 0, 0, 1}; }
    static constexpr Color Clear() { return {0, 0, 0, 0}; }
    
    // Lerp
    static Color lerp(Color a, Color b, float t) {
        return Color(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }
};

// ============================================================================
// CornerRadius (per-corner, clockwise from top-left)
// ============================================================================
struct CornerRadius {
    float tl = 0, tr = 0, br = 0, bl = 0;
    
    constexpr CornerRadius() = default;
    constexpr CornerRadius(float all) : tl(all), tr(all), br(all), bl(all) {}
    constexpr CornerRadius(float tl_, float tr_, float br_, float bl_)
        : tl(tl_), tr(tr_), br(br_), bl(bl_) {}
    
    Vec4 as_vec4() const { return {tl, tr, br, bl}; }
};

// ============================================================================
// Shadow Layer (for multi-layer elevation shadows)
// ============================================================================
struct ShadowLayer {
    Color color = Color::Clear();
    float blur = 0;
    float spread = 0;
    Vec2 offset = {0, 0};
    
    bool enabled() const { return color.a > 0.001f; }
};

// ============================================================================
// Elevation Shadow (Ambient + Key light, Apple/Material hybrid)
// ============================================================================
struct ElevationShadow {
    ShadowLayer ambient;
    ShadowLayer key;
    
    // Presets matching Material Design 3 / Apple HIG
    static ElevationShadow None() { return {}; }
    
    static ElevationShadow Level1() {  // Cards (2dp)
        return {
            { Color::Hex(0x00000020), 3.0f, 0.0f, {0, 1} },
            { Color::Hex(0x00000040), 2.0f, 0.0f, {0, 1} }
        };
    }
    
    static ElevationShadow Level2() {  // Dialogs (6dp)
        return {
            { Color::Hex(0x00000018), 10.0f, 0.0f, {0, 3} },
            { Color::Hex(0x00000030), 5.0f, 1.0f, {0, 3} }
        };
    }
    
    static ElevationShadow Level3() {  // Modals (12dp)
        return {
            { Color::Hex(0x00000015), 18.0f, 0.0f, {0, 6} },
            { Color::Hex(0x00000028), 8.0f, 2.0f, {0, 6} }
        };
    }
    
    static ElevationShadow Level4() {  // Drawers (24dp)
        return {
            { Color::Hex(0x00000012), 24.0f, 0.0f, {0, 12} },
            { Color::Hex(0x00000020), 12.0f, 4.0f, {0, 12} }
        };
    }
};

// ============================================================================
// Easing Functions
// ============================================================================
namespace easing {

inline float linear(float t) { return t; }

inline float ease_in_quad(float t) { return t * t; }
inline float ease_out_quad(float t) { return t * (2 - t); }
inline float ease_in_out_quad(float t) {
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

inline float ease_in_cubic(float t) { return t * t * t; }
inline float ease_out_cubic(float t) { 
    float t1 = t - 1; 
    return t1 * t1 * t1 + 1; 
}
inline float ease_in_out_cubic(float t) {
    return t < 0.5f 
        ? 4 * t * t * t 
        : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

inline float ease_out_back(float t, float overshoot = 1.70158f) {
    float t1 = t - 1;
    return t1 * t1 * ((overshoot + 1) * t1 + overshoot) + 1;
}

inline float ease_out_elastic(float t) {
    if (t == 0 || t == 1) return t;
    return std::pow(2.0f, -10.0f * t) * std::sin((t - 0.075f) * (2 * 3.14159f) / 0.3f) + 1;
}

inline float ease_out_bounce(float t) {
    if (t < 1 / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2 / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

// Spring physics (critically damped approximation)
inline float spring(float t, float damping = 0.5f, float stiffness = 100.0f) {
    (void)stiffness;
    float decay = std::exp(-damping * t * 10.0f);
    return 1.0f - decay * std::cos(t * 10.0f);
}

// Cubic Bezier (for custom curves, like CSS)
inline float cubic_bezier(float t, float x1, float y1, float x2, float y2) {
    // Newton-Raphson to solve for t given x
    // Simplified: just use y-lerp for now
    float mt = 1 - t;
    float mt2 = mt * mt;
    float t2 = t * t;
    return 3 * mt2 * t * y1 + 3 * mt * t2 * y2 + t2 * t;
}

} // namespace easing

} // namespace fanta
