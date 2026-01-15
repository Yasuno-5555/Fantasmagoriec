#pragma once
// L1: Core POD Types
// Dependencies: <cstdint>, <cstddef> ONLY. No project headers.

#include <cstdint>
#include <cstddef>

namespace fanta {
namespace internal {

    // --- Primitive Types ---
    struct Vec2 {
        float x = 0, y = 0;
        Vec2 operator*(float f) const { return {x*f, y*f}; }
        Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
        Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
        bool operator==(const Vec2& o) const { return x==o.x && y==o.y; }
    };

    struct Vec4 {
        float x = 0, y = 0, z = 0, w = 0;
    };

    struct Rectangle { 
        float x = 0, y = 0, w = 0, h = 0; 
    };

    struct ColorF {
        float r = 0, g = 0, b = 0, a = 1.0f;
        bool operator==(const ColorF& o) const { return r==o.r && g==o.g && b==o.b && a==o.a; }
        ColorF operator*(float t) const { return {r*t, g*t, b*t, a*t}; }
        ColorF operator+(const ColorF& o) const { return {r+o.r, g+o.g, b+o.b, a+o.a}; }
    };

    // --- ID (FNV-1a Hash) ---
    struct ID {
        uint64_t value = 0;
        
        constexpr ID() = default;
        constexpr explicit ID(uint64_t v) : value(v) {}
        
        // Compile-time FNV-1a
        constexpr ID(const char* str) : value(14695981039346656037ULL) {
            while (*str) {
                value ^= static_cast<uint64_t>(*str++);
                value *= 1099511628211ULL;
            }
        }
        
        bool operator==(const ID& o) const { return value == o.value; }
        bool operator!=(const ID& o) const { return value != o.value; }
        bool operator<(const ID& o) const { return value < o.value; }
        bool is_empty() const { return value == 0; }
    };

    // --- Layout Result (Computed) ---
    struct ComputedLayout {
        float x = 0, y = 0, w = 0, h = 0;
        float content_w = 0, content_h = 0;
        float scroll_x = 0, scroll_y = 0;
    };

} // namespace internal

// Public API re-exports
using ID = internal::ID;

} // namespace fanta
