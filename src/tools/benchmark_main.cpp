#include "fanta.h"
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <numeric>
#include <cmath>

using namespace fanta;

// Stats tracking
struct FrameStats {
    double frame_time_ms;
    size_t element_count;
};

std::vector<FrameStats> g_stats;
const int MAX_FRAMES = 500;
int g_frame_count = 0;

void BenchmarkLoop() {
    auto start = std::chrono::high_resolution_clock::now();

    // 1. Element Creation Stress Test
    // Create a deep nested structure
    Element(ID("Root")).size(1280, 720).column().align(Align::Start).justify(Justify::Start);
    
    // Header
    Element(ID("Header")).height(50).row().bg(Color(30, 30, 30)).padding(10);
    Element(ID("Title")).label("Benchmark Running...").fontSize(20).color(Color::White());
    
    // Grid of Items
    Element(ID("Content")).flexGrow(1).wrap().row().gap(5).padding(10);
    
    // Create 5000 items
    int item_count = 5000;
    for (int i = 0; i < item_count; ++i) {
        // Unique ID generation cost included
        std::string id_str = "Item_" + std::to_string(i);
        Element(ID(id_str.c_str()))
            .size(40, 40)
            .bg(Color(50, 50, 60))
            .rounded(4)
            .hoverBg(Color(100, 100, 120))
            .onClick([](){}); // Closure creation cost
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    if (g_frame_count > 10) { // Skip warmup
        g_stats.push_back({elapsed.count(), (size_t)item_count});
    }

    g_frame_count++;
    
    // Progress indicator
    if (g_frame_count % 50 == 0) {
        std::cout << "Frame " << g_frame_count << " / " << MAX_FRAMES << std::endl;
    }

    if (g_frame_count >= MAX_FRAMES) {
        // Analyze
        double total_time = 0;
        double max_time = 0;
        double min_time = 99999;
        
        for (const auto& s : g_stats) {
            total_time += s.frame_time_ms;
            if (s.frame_time_ms > max_time) max_time = s.frame_time_ms;
            if (s.frame_time_ms < min_time) min_time = s.frame_time_ms;
        }
        
        double avg_time = total_time / g_stats.size();
        
        std::cout << "==========================================" << std::endl;
        std::cout << " BENCHMARK RESULTS (Baseline)" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << " Total Frames: " << g_stats.size() << std::endl;
        std::cout << " Item Count:   " << item_count << std::endl;
        std::cout << " Avg API Time: " << avg_time << " ms" << std::endl;
        std::cout << " Min API Time: " << min_time << " ms" << std::endl;
        std::cout << " Max API Time: " << max_time << " ms" << std::endl;
        std::cout << " Thruput:      " << (item_count / avg_time) * 1000.0 << " elements/sec" << std::endl;
        std::cout << "==========================================" << std::endl;
        
        exit(0);
    }
}

int main() {
    Init(1280, 720, "Fantasmagorie Benchmark");
    Run(BenchmarkLoop);
    Shutdown(); // Unreachable due to exit(0), but good practice
    return 0;
}
