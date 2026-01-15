#include "plot.hpp"
#include "../core/contexts_internal.hpp"

namespace fanta {
namespace internal {

static PlotState g_plot;

PlotState& GetPlot() {
    return g_plot;
}

void PlotLogic::Update(ElementState& el, InputContext& input, const ComputedLayout& layout) {
    if (!el.is_plot) return;
    
    auto& state = GetPlot();
    
    float mx = input.mouse_x;
    float my = input.mouse_y;
    
    float plot_x = layout.x + state.margin_left;
    float plot_y = layout.y + state.margin_top;
    float plot_w = layout.w - state.margin_left - state.margin_right;
    float plot_h = layout.h - state.margin_top - state.margin_bottom;
    
    bool in_plot = mx >= plot_x && mx < plot_x + plot_w &&
                   my >= plot_y && my < plot_y + plot_h;
    
    if (in_plot) {
        state.show_crosshair = true;
        state.crosshair_pos = {mx, my};
        
        // Scroll to zoom
        if (std::abs(input.mouse_wheel) > 0.01f) {
            float zoom = 1.0f - input.mouse_wheel * 0.1f;
            float center_x = state.x_axis.min + (state.x_axis.max - state.x_axis.min) * 0.5f;
            float center_y = state.y_axis.min + (state.y_axis.max - state.y_axis.min) * 0.5f;
            
            state.x_axis.min = center_x + (state.x_axis.min - center_x) * zoom;
            state.x_axis.max = center_x + (state.x_axis.max - center_x) * zoom;
            state.y_axis.min = center_y + (state.y_axis.min - center_y) * zoom;
            state.y_axis.max = center_y + (state.y_axis.max - center_y) * zoom;
        }
    } else {
        state.show_crosshair = false;
    }
}

void PlotLogic::Render(PlotState& state, DrawList& dl, float x, float y, float w, float h) {
    float plot_x = x + state.margin_left;
    float plot_y = y + state.margin_top;
    float plot_w = w - state.margin_left - state.margin_right;
    float plot_h = h - state.margin_top - state.margin_bottom;
    
    // Background
    dl.add_rect({x, y}, {w, h}, state.background, 0);
    
    // Grid
    RenderGrid(state, dl, plot_x, plot_y, plot_w, plot_h);
    
    // Axes
    RenderAxes(state, dl, plot_x, plot_y, plot_w, plot_h);
    
    // Series
    RenderSeries(state, dl, plot_x, plot_y, plot_w, plot_h);
    
    // Crosshair
    if (state.show_crosshair) {
        dl.add_line({state.crosshair_pos.x, plot_y}, 
                    {state.crosshair_pos.x, plot_y + plot_h}, 
                    1.0f, {1, 1, 1, 0.5f});
        dl.add_line({plot_x, state.crosshair_pos.y}, 
                    {plot_x + plot_w, state.crosshair_pos.y}, 
                    1.0f, {1, 1, 1, 0.5f});
    }
    
    // Legend
    if (state.show_legend) {
        RenderLegend(state, dl, x + w - state.margin_right - 100, y + state.margin_top + 10);
    }
}

void PlotLogic::RenderGrid(PlotState& state, DrawList& dl, float x, float y, float w, float h) {
    // Vertical grid lines
    if (state.x_axis.show_grid) {
        for (int i = 0; i <= state.x_axis.grid_divisions; i++) {
            float t = static_cast<float>(i) / state.x_axis.grid_divisions;
            float gx = x + t * w;
            dl.add_line({gx, y}, {gx, y + h}, 1.0f, state.grid_color);
        }
    }
    
    // Horizontal grid lines
    if (state.y_axis.show_grid) {
        for (int i = 0; i <= state.y_axis.grid_divisions; i++) {
            float t = static_cast<float>(i) / state.y_axis.grid_divisions;
            float gy = y + h - t * h;
            dl.add_line({x, gy}, {x + w, gy}, 1.0f, state.grid_color);
        }
    }
}

void PlotLogic::RenderAxes(PlotState& state, DrawList& dl, float x, float y, float w, float h) {
    // X axis
    dl.add_line({x, y + h}, {x + w, y + h}, 2.0f, state.axis_color);
    
    // Y axis
    dl.add_line({x, y}, {x, y + h}, 2.0f, state.axis_color);
    
    // TODO: Add axis labels and tick marks with text rendering
}

void PlotLogic::RenderSeries(PlotState& state, DrawList& dl, float x, float y, float w, float h) {
    for (const auto& s : state.series) {
        if (!s.visible || s.points.size() < 2) continue;
        
        for (size_t i = 1; i < s.points.size(); i++) {
            float x0 = state.x_axis.normalize(s.points[i-1].x);
            float y0 = state.y_axis.normalize(s.points[i-1].y);
            float x1 = state.x_axis.normalize(s.points[i].x);
            float y1 = state.y_axis.normalize(s.points[i].y);
            
            // Screen coordinates (y inverted)
            float sx0 = x + x0 * w;
            float sy0 = y + h - y0 * h;
            float sx1 = x + x1 * w;
            float sy1 = y + h - y1 * h;
            
            dl.add_line({sx0, sy0}, {sx1, sy1}, s.line_width, s.color);
        }
    }
}

void PlotLogic::RenderLegend(PlotState& state, DrawList& dl, float x, float y) {
    float row_height = 20.0f;
    float swatch_size = 12.0f;
    
    for (size_t i = 0; i < state.series.size(); i++) {
        const auto& s = state.series[i];
        float ry = y + i * row_height;
        
        // Color swatch
        dl.add_rect({x, ry + 4}, {swatch_size, swatch_size}, s.color, 0);
        
        // TODO: Add label text
    }
}

} // namespace internal
} // namespace fanta
