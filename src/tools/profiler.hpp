#pragma once

#include "../core/types_internal.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <deque>

namespace fanta {
namespace internal {

// Frame Timing Sample
struct FrameSample {
    float frame_time_ms;
    float cpu_time_ms;
    float gpu_time_ms;  // Requires GPU timer queries
    float layout_time_ms;
    float render_time_ms;
    int draw_calls;
    int triangles;
    int elements;
};

// Profiler State
struct ProfilerState {
    bool visible = false;
    bool overlay_mode = true;  // Small overlay vs full panel
    
    // Samples
    std::deque<FrameSample> samples;
    static constexpr int max_samples = 120;  // 2 seconds at 60fps
    
    // Current frame timers
    std::chrono::steady_clock::time_point frame_start;
    std::chrono::steady_clock::time_point cpu_start;
    float current_layout_ms = 0;
    float current_render_ms = 0;
    int current_draw_calls = 0;
    int current_triangles = 0;
    int current_elements = 0;
    
    // Stats
    float avg_frame_time = 0;
    float max_frame_time = 0;
    float min_frame_time = 999.0f;
    int fps = 0;
    
    void begin_frame() {
        frame_start = std::chrono::steady_clock::now();
        cpu_start = frame_start;
    }
    
    void end_frame() {
        auto now = std::chrono::steady_clock::now();
        float frame_ms = std::chrono::duration<float, std::milli>(now - frame_start).count();
        float cpu_ms = std::chrono::duration<float, std::milli>(now - cpu_start).count();
        
        FrameSample sample = {
            frame_ms,
            cpu_ms,
            0,  // GPU time would need GL/D3D timer queries
            current_layout_ms,
            current_render_ms,
            current_draw_calls,
            current_triangles,
            current_elements
        };
        
        samples.push_back(sample);
        while (samples.size() > max_samples) {
            samples.pop_front();
        }
        
        update_stats();
    }
    
    void update_stats() {
        if (samples.empty()) return;
        
        float total = 0;
        max_frame_time = 0;
        min_frame_time = 999.0f;
        
        for (const auto& s : samples) {
            total += s.frame_time_ms;
            max_frame_time = std::max(max_frame_time, s.frame_time_ms);
            min_frame_time = std::min(min_frame_time, s.frame_time_ms);
        }
        
        avg_frame_time = total / samples.size();
        fps = static_cast<int>(1000.0f / avg_frame_time);
    }
    
    void record_layout(float ms) { current_layout_ms = ms; }
    void record_render(float ms) { current_render_ms = ms; }
    void record_draw_calls(int n) { current_draw_calls = n; }
    void record_triangles(int n) { current_triangles = n; }
    void record_elements(int n) { current_elements = n; }
};

// Profiler Logic
struct ProfilerLogic {
    static void Update(ProfilerState& state);
    static void Render(ProfilerState& state, DrawList& dl, float screen_width, float screen_height);
    static void RenderOverlay(ProfilerState& state, DrawList& dl, float x, float y);
    static void RenderFullPanel(ProfilerState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderGraph(ProfilerState& state, DrawList& dl, float x, float y, float w, float h);
};

// Global accessor
ProfilerState& GetProfiler();

} // namespace internal

// Public API
void ShowProfiler(bool show = true);
void ToggleProfiler();
bool IsProfilerVisible();
void SetProfilerOverlayMode(bool overlay);

} // namespace fanta
