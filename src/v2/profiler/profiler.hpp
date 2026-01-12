// Fantasmagorie v2 - Performance Profiler
// Built-in frame timing and widget performance analysis
#pragma once

#include "../core/types.hpp"
#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>

namespace fanta {
namespace profiler {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::duration<double, std::milli>;

// ============================================================================
// Frame Stats
// ============================================================================

struct FrameStats {
    double frame_time_ms = 0;
    double layout_time_ms = 0;
    double render_time_ms = 0;
    double input_time_ms = 0;
    
    int widget_count = 0;
    int draw_cmd_count = 0;
    size_t memory_bytes = 0;
    
    int frame_number = 0;
};

// ============================================================================
// Scoped Timer
// ============================================================================

class ScopedTimer {
public:
    ScopedTimer(double& target) : target_(target), start_(Clock::now()) {}
    ~ScopedTimer() {
        auto end = Clock::now();
        target_ = Duration(end - start_).count();
    }
private:
    double& target_;
    TimePoint start_;
};

// ============================================================================
// Widget Profiler
// ============================================================================

struct WidgetProfile {
    std::string name;
    double total_time_ms = 0;
    int call_count = 0;
    double avg_time_ms() const { return call_count > 0 ? total_time_ms / call_count : 0; }
};

// ============================================================================
// Profiler
// ============================================================================

class Profiler {
public:
    static Profiler& instance() {
        static Profiler p;
        return p;
    }
    
    bool enabled = false;
    bool show_overlay = true;
    
    FrameStats current_frame;
    std::vector<FrameStats> history;
    std::unordered_map<std::string, WidgetProfile> widgets;
    
    void begin_frame() {
        if (!enabled) return;
        frame_start_ = Clock::now();
        current_frame = FrameStats();
        current_frame.frame_number = frame_count_++;
    }
    
    void end_frame() {
        if (!enabled) return;
        auto end = Clock::now();
        current_frame.frame_time_ms = Duration(end - frame_start_).count();
        
        // Keep history (last 120 frames)
        history.push_back(current_frame);
        if (history.size() > 120) {
            history.erase(history.begin());
        }
    }
    
    ScopedTimer timer_layout() { return ScopedTimer(current_frame.layout_time_ms); }
    ScopedTimer timer_render() { return ScopedTimer(current_frame.render_time_ms); }
    ScopedTimer timer_input() { return ScopedTimer(current_frame.input_time_ms); }
    
    void record_widget(const char* name, double time_ms) {
        if (!enabled) return;
        auto& w = widgets[name];
        w.name = name;
        w.total_time_ms += time_ms;
        w.call_count++;
        current_frame.widget_count++;
    }
    
    void record_draw_cmds(int count) {
        current_frame.draw_cmd_count += count;
    }
    
    void record_memory(size_t bytes) {
        current_frame.memory_bytes = bytes;
    }
    
    // Analysis
    double avg_frame_time() const {
        if (history.empty()) return 0;
        double sum = 0;
        for (const auto& f : history) sum += f.frame_time_ms;
        return sum / history.size();
    }
    
    double fps() const {
        double avg = avg_frame_time();
        return avg > 0 ? 1000.0 / avg : 0;
    }
    
    std::vector<WidgetProfile> slowest_widgets(int n = 5) const {
        std::vector<WidgetProfile> result;
        for (const auto& [name, profile] : widgets) {
            result.push_back(profile);
        }
        std::sort(result.begin(), result.end(),
            [](const WidgetProfile& a, const WidgetProfile& b) {
                return a.total_time_ms > b.total_time_ms;
            });
        if (result.size() > (size_t)n) result.resize(n);
        return result;
    }
    
    void clear() {
        history.clear();
        widgets.clear();
        frame_count_ = 0;
    }
    
private:
    TimePoint frame_start_;
    int frame_count_ = 0;
};

// ============================================================================
// Convenience
// ============================================================================

inline void enable() { Profiler::instance().enabled = true; }
inline void disable() { Profiler::instance().enabled = false; }
inline bool is_enabled() { return Profiler::instance().enabled; }

inline void begin_frame() { Profiler::instance().begin_frame(); }
inline void end_frame() { Profiler::instance().end_frame(); }

inline double fps() { return Profiler::instance().fps(); }
inline const FrameStats& current() { return Profiler::instance().current_frame; }

} // namespace profiler
} // namespace fanta
