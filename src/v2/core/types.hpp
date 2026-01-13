// Fantasmagorie v2 - Core Types
// Fundamental type definitions for the UI framework
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace fanta {

// ============================================================================
// Fundamental Types
// ============================================================================

using NodeID = uint64_t;
constexpr NodeID INVALID_NODE = 0;

// ============================================================================
// Color
// ============================================================================

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;
    
    constexpr Color() = default;
    constexpr Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}
    
    static constexpr Color Hex(uint32_t rgba) {
        return Color(
            (rgba >> 24) & 0xFF,
            (rgba >> 16) & 0xFF,
            (rgba >> 8) & 0xFF,
            rgba & 0xFF
        );
    }
    
    constexpr uint32_t u32() const {
        return (uint32_t(r) << 24) | (uint32_t(g) << 16) | (uint32_t(b) << 8) | a;
    }

    // Swizzle helpers for different Graphics APIs
    // u32() returns 0xRRGGBBAA
    
    // For OpenGL with GL_RGBA8 (little endian platform)
    // byte ordering in memory: [R, G, B, A]
    constexpr uint32_t to_rgba8() const { return u32(); }
    
    // For systems expecting BGRA (e.g. some Windows APIs or Vulkan/D3D swapchains)
    // byte ordering in memory: [B, G, R, A] -> 0xAARRGGBB in u32
    constexpr uint32_t to_bgra8() const {
        return (uint32_t(b) << 24) | (uint32_t(g) << 16) | (uint32_t(r) << 8) | a;
    }

    // For systems expecting ABGR (Common in some OpenGL extensions)
    // byte ordering in memory: [A, B, G, R]
    constexpr uint32_t to_abgr8() const {
        return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(g) << 8) | r;
    }
    
    // Common colors
    static constexpr Color White() { return Color(255, 255, 255); }
    static constexpr Color Black() { return Color(0, 0, 0); }
    static constexpr Color Transparent() { return Color(0, 0, 0, 0); }
    
    static Color lerp(const Color& a, const Color& b, float t) {
        if (t <= 0) return a;
        if (t >= 1) return b;
        return Color(
            (uint8_t)(a.r + (b.r - a.r) * t),
            (uint8_t)(a.g + (b.g - a.g) * t),
            (uint8_t)(a.b + (b.b - a.b) * t),
            (uint8_t)(a.a + (b.a - a.a) * t)
        );
    }
};

// ============================================================================
// Geometry
// ============================================================================

struct Vec2 {
    float x = 0, y = 0;
    
    constexpr Vec2() = default;
    constexpr Vec2(float x_, float y_) : x(x_), y(y_) {}
    
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

struct Rect {
    float x = 0, y = 0, w = 0, h = 0;
    
    constexpr Rect() = default;
    constexpr Rect(float x_, float y_, float w_, float h_)
        : x(x_), y(y_), w(w_), h(h_) {}
    
    bool contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
    
    bool contains(Vec2 p) const { return contains(p.x, p.y); }
};

// ============================================================================
// Transform (2D Affine)
// ============================================================================

struct Transform {
    // [a b tx]   transforms point (x,y) to:
    // [c d ty]   (a*x + c*y + tx, b*x + d*y + ty)
    // [0 0  1]
    float a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;
    
    static Transform Identity() { return {}; }
    
    static Transform Translate(float x, float y) {
        Transform t;
        t.tx = x;
        t.ty = y;
        return t;
    }
    
    static Transform Scale(float s) {
        Transform t;
        t.a = s;
        t.d = s;
        return t;
    }
    
    static Transform Scale(float sx, float sy) {
        Transform t;
        t.a = sx;
        t.d = sy;
        return t;
    }
    
    void apply(float& x, float& y) const {
        float nx = a * x + c * y + tx;
        float ny = b * x + d * y + ty;
        x = nx;
        y = ny;
    }
    
    Vec2 apply(Vec2 p) const {
        apply(p.x, p.y);
        return p;
    }
    
    Transform operator*(const Transform& o) const {
        Transform r;
        r.a = a * o.a + b * o.c;
        r.b = a * o.b + b * o.d;
        r.c = c * o.a + d * o.c;
        r.d = c * o.b + d * o.d;
        r.tx = tx * o.a + ty * o.c + o.tx;
        r.ty = tx * o.b + ty * o.d + o.ty;
        return r;
    }
};

// ============================================================================
// Enums
// ============================================================================

enum class LayoutDir { Row, Column };
enum class Align { Start, Center, End, Stretch };
enum class CursorType { Arrow, Hand, Text, Resize };



enum class IconType : uint32_t {
    Check,
    Close,
    Menu,
    ChevronRight,
    ChevronDown,
    Settings,
    Search,
    User,
    Info,
    Warning
};

} // namespace fanta
