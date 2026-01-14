#include "color_picker.hpp"
#include "../core/contexts_internal.hpp"

namespace fanta {
namespace internal {

static ColorPickerState g_color_picker;

ColorPickerState& GetColorPicker() {
    return g_color_picker;
}

void ColorPickerLogic::Update(ElementState& el, InputContext& input, const ComputedLayout& layout) {
    if (!el.is_color_picker) return;
    
    auto& state = GetColorPicker();
    
    float mx = input.mouse_x - layout.x;
    float my = input.mouse_y - layout.y;
    
    float sv_x = 0;
    float sv_y = 0;
    float hue_x = state.sv_size + 10.0f;
    float hue_y = 0;
    float alpha_x = hue_x + state.hue_width + 10.0f;
    
    // SV Square interaction
    bool in_sv = mx >= sv_x && mx < sv_x + state.sv_size &&
                 my >= sv_y && my < sv_y + state.sv_size;
    
    // Hue bar interaction  
    bool in_hue = mx >= hue_x && mx < hue_x + state.hue_width &&
                  my >= hue_y && my < hue_y + state.sv_size;
    
    // Alpha bar interaction
    bool in_alpha = state.show_alpha &&
                    mx >= alpha_x && mx < alpha_x + state.alpha_width &&
                    my >= hue_y && my < hue_y + state.sv_size;
    
    // Start drag
    if (input.state.is_just_pressed()) {
        if (in_sv) state.dragging_sv = true;
        else if (in_hue) state.dragging_hue = true;
        else if (in_alpha) state.dragging_alpha = true;
    }
    
    // End drag
    if (!input.mouse_down) {
        state.dragging_sv = false;
        state.dragging_hue = false;
        state.dragging_alpha = false;
    }
    
    // Update values while dragging
    if (state.dragging_sv) {
        state.current.s = std::clamp((mx - sv_x) / state.sv_size, 0.0f, 1.0f);
        state.current.v = 1.0f - std::clamp((my - sv_y) / state.sv_size, 0.0f, 1.0f);
    }
    
    if (state.dragging_hue) {
        state.current.h = std::clamp((my - hue_y) / state.sv_size, 0.0f, 1.0f) * 360.0f;
    }
    
    if (state.dragging_alpha) {
        state.current.a = 1.0f - std::clamp((my - hue_y) / state.sv_size, 0.0f, 1.0f);
    }
    
    // Update RGB from HSV
    state.rgb = state.current.ToRGB();
}

void ColorPickerLogic::RenderSVSquare(ColorPickerState& state, DrawList& dl, float x, float y, float size) {
    // Background: gradient from white to hue color (horizontal)
    // Then overlay: gradient from transparent to black (vertical)
    
    // For simplicity, render as a solid rect with current hue, 
    // then use shaders for gradient (would need custom implementation)
    
    HSV pure_hue = {state.current.h, 1.0f, 1.0f, 1.0f};
    ColorF hue_color = pure_hue.ToRGB();
    
    // Base rect with hue
    dl.add_rect({x, y}, {size, size}, hue_color, 0);
    
    // Horizontal white gradient would require shader
    // Vertical black gradient would require shader
    
    // Cursor position
    float cx = x + state.current.s * size;
    float cy = y + (1.0f - state.current.v) * size;
    
    // Draw cursor circle
    dl.add_circle({cx, cy}, 6.0f, {1, 1, 1, 1}, 0);
    dl.add_circle({cx, cy}, 4.0f, {0, 0, 0, 1}, 0);
}

void ColorPickerLogic::RenderHueBar(ColorPickerState& state, DrawList& dl, float x, float y, float w, float h) {
    // Render hue spectrum as vertical bar
    int segments = 6;
    float seg_height = h / segments;
    
    // Hue colors at each segment
    float hues[] = {0, 60, 120, 180, 240, 300, 360};
    
    for (int i = 0; i < segments; i++) {
        HSV top_hsv = {hues[i], 1, 1, 1};
        HSV bot_hsv = {hues[i + 1], 1, 1, 1};
        
        // Simple: just draw rect with average color
        HSV mid_hsv = {(hues[i] + hues[i + 1]) / 2.0f, 1, 1, 1};
        dl.add_rect({x, y + i * seg_height}, {w, seg_height}, mid_hsv.ToRGB(), 0);
    }
    
    // Cursor
    float cursor_y = y + (state.current.h / 360.0f) * h;
    dl.add_rect({x - 2, cursor_y - 2}, {w + 4, 4}, {1, 1, 1, 1}, 0);
}

void ColorPickerLogic::RenderAlphaBar(ColorPickerState& state, DrawList& dl, float x, float y, float w, float h) {
    // Checkerboard background for alpha preview
    float check_size = 8.0f;
    for (float cy = y; cy < y + h; cy += check_size) {
        for (float cx = x; cx < x + w; cx += check_size) {
            int row = static_cast<int>((cy - y) / check_size);
            int col = static_cast<int>((cx - x) / check_size);
            ColorF c = ((row + col) % 2 == 0) ? ColorF{0.8f, 0.8f, 0.8f, 1} : ColorF{0.5f, 0.5f, 0.5f, 1};
            dl.add_rect({cx, cy}, {check_size, check_size}, c, 0);
        }
    }
    
    // Alpha gradient overlay
    ColorF top = state.current.ToRGB();
    top.a = 1.0f;
    ColorF bot = top;
    bot.a = 0.0f;
    
    // Simple: draw solid with current alpha
    ColorF mid = top;
    mid.a = 0.5f;
    dl.add_rect({x, y}, {w, h}, mid, 0);
    
    // Cursor
    float cursor_y = y + (1.0f - state.current.a) * h;
    dl.add_rect({x - 2, cursor_y - 2}, {w + 4, 4}, {1, 1, 1, 1}, 0);
}

void ColorPickerLogic::RenderHistory(ColorPickerState& state, DrawList& dl, float x, float y, float size) {
    float swatch_size = 24.0f;
    float gap = 4.0f;
    int per_row = static_cast<int>(size / (swatch_size + gap));
    
    for (size_t i = 0; i < state.history.size(); i++) {
        int row = static_cast<int>(i) / per_row;
        int col = static_cast<int>(i) % per_row;
        
        float sx = x + col * (swatch_size + gap);
        float sy = y + row * (swatch_size + gap);
        
        dl.add_rounded_rect({sx, sy}, {swatch_size, swatch_size}, 4.0f, state.history[i], 0);
    }
}

} // namespace internal
} // namespace fanta
