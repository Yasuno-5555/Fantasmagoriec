#pragma once

#include "../core/types_internal.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace fanta {
namespace internal {

// HSV Color representation
struct HSV {
    float h = 0;  // 0-360
    float s = 0;  // 0-1
    float v = 0;  // 0-1
    float a = 1;  // 0-1
    
    static HSV FromRGB(float r, float g, float b, float a = 1.0f) {
        HSV hsv;
        hsv.a = a;
        
        float max = std::max({r, g, b});
        float min = std::min({r, g, b});
        float delta = max - min;
        
        hsv.v = max;
        hsv.s = (max > 0) ? delta / max : 0;
        
        if (delta < 0.00001f) {
            hsv.h = 0;
        } else if (max == r) {
            hsv.h = 60.0f * std::fmod((g - b) / delta, 6.0f);
        } else if (max == g) {
            hsv.h = 60.0f * ((b - r) / delta + 2.0f);
        } else {
            hsv.h = 60.0f * ((r - g) / delta + 4.0f);
        }
        
        if (hsv.h < 0) hsv.h += 360.0f;
        return hsv;
    }
    
    ColorF ToRGB() const {
        float c = v * s;
        float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;
        
        float r = 0, g = 0, b = 0;
        
        if (h < 60)       { r = c; g = x; }
        else if (h < 120) { r = x; g = c; }
        else if (h < 180) { g = c; b = x; }
        else if (h < 240) { g = x; b = c; }
        else if (h < 300) { r = x; b = c; }
        else              { r = c; b = x; }
        
        return {r + m, g + m, b + m, a};
    }
};

// Color Picker Widget State
struct ColorPickerState {
    HSV current;
    ColorF rgb;
    
    // Mode
    bool show_alpha = true;
    bool hsv_mode = true;  // false = RGB mode
    
    // Picker geometry
    float sv_size = 200.0f;  // Saturation/Value square size
    float hue_width = 24.0f;  // Hue bar width
    float alpha_width = 16.0f;
    
    // Drag state
    bool dragging_sv = false;
    bool dragging_hue = false;
    bool dragging_alpha = false;
    
    // History
    std::vector<ColorF> history;
    static constexpr int max_history = 16;
    
    void set_color(const ColorF& c) {
        rgb = c;
        current = HSV::FromRGB(c.r, c.g, c.b, c.a);
    }
    
    ColorF get_color() const {
        return current.ToRGB();
    }
    
    void add_to_history(const ColorF& c) {
        // Add to front, remove duplicates
        for (auto it = history.begin(); it != history.end(); ) {
            if (std::abs(it->r - c.r) < 0.01f && 
                std::abs(it->g - c.g) < 0.01f &&
                std::abs(it->b - c.b) < 0.01f) {
                it = history.erase(it);
            } else {
                ++it;
            }
        }
        history.insert(history.begin(), c);
        if (history.size() > max_history) {
            history.pop_back();
        }
    }
};

// Color Picker Logic
struct ColorPickerLogic {
    static void Update(ElementState& el, InputContext& input, const ComputedLayout& layout);
    static void RenderSVSquare(ColorPickerState& state, DrawList& dl, float x, float y, float size);
    static void RenderHueBar(ColorPickerState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderAlphaBar(ColorPickerState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderHistory(ColorPickerState& state, DrawList& dl, float x, float y, float size);
};

// Global accessor
ColorPickerState& GetColorPicker();

} // namespace internal
} // namespace fanta
