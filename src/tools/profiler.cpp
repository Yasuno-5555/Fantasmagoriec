#include "profiler.hpp"

namespace fanta {
namespace internal {

static ProfilerState g_profiler;

ProfilerState& GetProfiler() {
    return g_profiler;
}

void ProfilerLogic::Update(ProfilerState& state) {
    // Nothing to update, timing is done via begin/end frame
    (void)state;
}

void ProfilerLogic::Render(ProfilerState& state, DrawList& dl, float screen_width, float screen_height) {
    if (!state.visible) return;
    
    if (state.overlay_mode) {
        RenderOverlay(state, dl, screen_width - 150, 10);
    } else {
        RenderFullPanel(state, dl, 0, screen_height - 200, screen_width, 200);
    }
}

void ProfilerLogic::RenderOverlay(ProfilerState& state, DrawList& dl, float x, float y) {
    float w = 140.0f;
    float h = 80.0f;
    
    // Background
    ColorF bg = {0, 0, 0, 0.7f};
    dl.add_rounded_rect({x, y}, {w, h}, 4.0f, bg, 2.0f);
    
    // FPS color (green/yellow/red based on performance)
    ColorF fps_color;
    if (state.fps >= 55) fps_color = {0.2f, 0.9f, 0.2f, 1.0f};
    else if (state.fps >= 30) fps_color = {0.9f, 0.9f, 0.2f, 1.0f};
    else fps_color = {0.9f, 0.2f, 0.2f, 1.0f};
    
    // TODO: Render text
    // FPS: XX
    // Frame: XX.X ms
    // Draw: XX calls
    
    // Mini graph
    float graph_y = y + h - 20.0f;
    float graph_h = 15.0f;
    float bar_w = w / state.samples.size();
    
    for (size_t i = 0; i < state.samples.size(); i++) {
        float t = state.samples[i].frame_time_ms / 33.3f;  // Normalized to 30fps
        t = std::min(1.0f, t);
        float bar_h = t * graph_h;
        
        ColorF bar_color;
        if (t < 0.5f) bar_color = {0.2f, 0.9f, 0.2f, 1.0f};
        else if (t < 1.0f) bar_color = {0.9f, 0.9f, 0.2f, 1.0f};
        else bar_color = {0.9f, 0.2f, 0.2f, 1.0f};
        
        dl.add_rect({x + i * bar_w, graph_y + graph_h - bar_h}, {bar_w - 1, bar_h}, bar_color, 0);
    }
    
    (void)fps_color;
}

void ProfilerLogic::RenderFullPanel(ProfilerState& state, DrawList& dl, float x, float y, float w, float h) {
    // Background
    ColorF bg = {0.12f, 0.12f, 0.12f, 0.95f};
    dl.add_rect({x, y}, {w, h}, bg, 0);
    
    // Header
    ColorF header_bg = {0.2f, 0.2f, 0.2f, 1.0f};
    dl.add_rect({x, y}, {w, 24.0f}, header_bg, 0);
    
    // Graph
    RenderGraph(state, dl, x + 10, y + 30, w - 20, h - 80);
    
    // Stats row
    // TODO: Render stats text
}

void ProfilerLogic::RenderGraph(ProfilerState& state, DrawList& dl, float x, float y, float w, float h) {
    if (state.samples.empty()) return;
    
    // Grid lines
    ColorF grid = {0.3f, 0.3f, 0.3f, 0.5f};
    float target_60 = h * (16.67f / 50.0f);  // 60fps = 16.67ms
    float target_30 = h * (33.33f / 50.0f);  // 30fps = 33.33ms
    
    dl.add_line({x, y + h - target_60}, {x + w, y + h - target_60}, 1.0f, {0.2f, 0.9f, 0.2f, 0.5f});
    dl.add_line({x, y + h - target_30}, {x + w, y + h - target_30}, 1.0f, {0.9f, 0.9f, 0.2f, 0.5f});
    
    // Frame time line
    float point_spacing = w / state.max_samples;
    
    for (size_t i = 1; i < state.samples.size(); i++) {
        float x0 = x + (i - 1) * point_spacing;
        float x1 = x + i * point_spacing;
        float y0 = y + h - (state.samples[i - 1].frame_time_ms / 50.0f) * h;
        float y1 = y + h - (state.samples[i].frame_time_ms / 50.0f) * h;
        
        dl.add_line({x0, y0}, {x1, y1}, 2.0f, {0.4f, 0.6f, 1.0f, 1.0f});
    }
    
    (void)grid;
}

} // namespace internal

// Public API
void ShowProfiler(bool show) {
    internal::GetProfiler().visible = show;
}

void ToggleProfiler() {
    internal::GetProfiler().visible = !internal::GetProfiler().visible;
}

bool IsProfilerVisible() {
    return internal::GetProfiler().visible;
}

void SetProfilerOverlayMode(bool overlay) {
    internal::GetProfiler().overlay_mode = overlay;
}

} // namespace fanta
