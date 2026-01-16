#pragma once
#include "view/definitions.hpp"
#include <algorithm>

namespace fanta {
namespace ui {

    // --- CRTP Mixins for Universal API ---
    // The Universal Masquerade: Every widget can look and act like any other.

    // LayoutConfig: Provides width, height, padding, margin, flex props
    template<typename Derived>
    struct LayoutConfig {
        Derived& self() { return static_cast<Derived&>(*this); }
        
        // Dimensions
        Derived& width(float w) { self().view_->width = w; return self(); }
        Derived& height(float h) { self().view_->height = h; return self(); }
        Derived& size(float w, float h) { return width(w).height(h); }
        
        // Flex
        Derived& grow(float g) { self().view_->flex_grow = g; return self(); }
        Derived& shrink(float s) { self().view_->flex_shrink = s; return self(); }
        Derived& wrap(bool w = true) { self().view_->wrap = w; return self(); }
        Derived& align(Align a) { self().view_->align = a; return self(); }

        // Spacing
        Derived& padding(float p) { self().view_->padding = p; return self(); }
        Derived& margin(float m) { self().view_->margin = m; return self(); }
        
        Derived& padding(float v, float h) { return padding(std::max(v, h)); }
        Derived& padding(float t, float r, float b, float l) { return padding(std::max({t, r, b, l})); }
        
        Derived& margin(float v, float h) { return margin(std::max(v, h)); }
        Derived& margin(float t, float r, float b, float l) { return margin(std::max({t, r, b, l})); }
        
        // Property Block Helper
        template<typename F>
        Derived& prop(F&& f) { f(); return self(); }
    };

    // StyleConfig: Provides visuals (bg, color, radius, shadow, blur)
    template<typename Derived>
    struct StyleConfig {
        Derived& self() { return static_cast<Derived&>(*this); }
        
        // Common visual props (Now on ViewHeader!)
        Derived& bg(internal::ColorF c) { self().view_->bg_color = c; return self(); }
        Derived& color(internal::ColorF c) { self().view_->fg_color = c; return self(); } // Foreground
        Derived& radius(float r) { self().view_->border_radius = r; return self(); }
        Derived& squircle(bool s = true) { self().view_->is_squircle = s; return self(); }
        Derived& shadow(float s) { self().view_->elevation = s; return self(); }
        Derived& elevation(float e) { self().view_->elevation = e; return self(); }
        Derived& blur(float b) { self().view_->backdrop_blur = b; return self(); }
        Derived& wobble(float x, float y) { self().view_->wobble_x = x; self().view_->wobble_y = y; return self(); }
        
        // Phase 50: Advanced Styling
        Derived& border(float width, internal::ColorF c) {
            self().view_->border_width = width;
            self().view_->border_color = c;
            return self();
        }
        
        Derived& glow(float strength, internal::ColorF c) {
            self().view_->glow_strength = strength;
            self().view_->glow_color = c;
            return self();
        }
        
        // Font Size (Universal Masquerade)
        Derived& size(float s) { self().view_->font_size = s; return self(); }
        
        // Presets
        Derived& red() { return bg({1, 0, 0, 1}); }
        Derived& green() { return bg({0, 1, 0, 1}); }
        Derived& blue() { return bg({0, 0, 1, 1}); }
        Derived& dark() { return bg({0.1f, 0.1f, 0.1f, 1.0f}); }
        Derived& ghost() { return bg({0, 0, 0, 0}); }
    };

}
}
