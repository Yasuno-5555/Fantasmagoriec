#pragma once

#include "../core/types_internal.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <deque>

namespace fanta {
namespace internal {

// Plot Data Point
struct PlotPoint {
    float x;
    float y;
};

// Plot Series (one line)
struct PlotSeries {
    std::string label;
    std::deque<PlotPoint> points;
    ColorF color = {0.2f, 0.6f, 1.0f, 1.0f};
    float line_width = 2.0f;
    bool visible = true;
    int max_points = 1000;  // For real-time data
    
    void add_point(float x, float y) {
        points.push_back({x, y});
        while (points.size() > static_cast<size_t>(max_points)) {
            points.pop_front();
        }
    }
    
    void clear() { points.clear(); }
};

// Axis Configuration
struct PlotAxis {
    std::string label;
    float min = 0;
    float max = 1;
    bool auto_fit = true;
    bool show_grid = true;
    int grid_divisions = 5;
    
    float normalize(float v) const {
        return (v - min) / (max - min);
    }
    
    void fit_to_data(const std::vector<PlotSeries>& series, bool is_x) {
        if (!auto_fit || series.empty()) return;
        
        float data_min = std::numeric_limits<float>::max();
        float data_max = std::numeric_limits<float>::lowest();
        
        for (const auto& s : series) {
            for (const auto& p : s.points) {
                float v = is_x ? p.x : p.y;
                data_min = std::min(data_min, v);
                data_max = std::max(data_max, v);
            }
        }
        
        if (data_min < data_max) {
            float margin = (data_max - data_min) * 0.1f;
            min = data_min - margin;
            max = data_max + margin;
        }
    }
};

// Plot Widget State
struct PlotState {
    std::vector<PlotSeries> series;
    PlotAxis x_axis;
    PlotAxis y_axis;
    
    // Appearance
    ColorF background = {0.12f, 0.12f, 0.12f, 1.0f};
    ColorF grid_color = {0.3f, 0.3f, 0.3f, 1.0f};
    ColorF axis_color = {0.5f, 0.5f, 0.5f, 1.0f};
    ColorF text_color = {0.8f, 0.8f, 0.8f, 1.0f};
    
    // Interaction
    bool show_legend = true;
    bool show_crosshair = false;
    Vec2 crosshair_pos = {0, 0};
    bool panning = false;
    bool zooming = false;
    
    // Margins
    float margin_left = 60.0f;
    float margin_right = 20.0f;
    float margin_top = 20.0f;
    float margin_bottom = 40.0f;
    
    void add_series(const std::string& label, ColorF color) {
        PlotSeries s;
        s.label = label;
        s.color = color;
        series.push_back(s);
    }
    
    void auto_fit() {
        x_axis.fit_to_data(series, true);
        y_axis.fit_to_data(series, false);
    }
};

// Plot Logic
struct PlotLogic {
    static void Update(ElementState& el, InputContext& input, const ComputedLayout& layout);
    static void Render(PlotState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderGrid(PlotState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderAxes(PlotState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderSeries(PlotState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderLegend(PlotState& state, DrawList& dl, float x, float y);
};

// Global accessor
PlotState& GetPlot();

} // namespace internal
} // namespace fanta
