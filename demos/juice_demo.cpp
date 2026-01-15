#include "view/api.hpp"
#include "core/contexts_internal.hpp"
#include "backend/backend_factory.hpp"
#include <chrono>
#include <thread>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "view/debug.hpp"
#include <iostream>

using namespace fanta;
using namespace fanta::ui;

int main(int argc, char** argv) {
    bool auto_screenshot = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--screenshot") auto_screenshot = true;
    }

    auto backend = CreateOpenGLBackend();
    if (!backend->init(1200, 800, "Visual Juice Verification")) return 1;

    internal::GetEngineContext().world.backend = std::move(backend);
    auto& ctx = internal::GetEngineContext();

    int frame_count = 0;

    while (ctx.world.backend->is_running()) {
        frame_count++;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Input Handling
        float mx, my, wheel;
        bool down;
        ctx.world.backend->get_mouse_state(mx, my, down, wheel);
        
        // Update Input Context Facts
        ctx.input.mouse_x = mx;
        ctx.input.mouse_y = my;
        ctx.input.mouse_down = down;
        ctx.input.mouse_wheel = wheel;

        ctx.world.backend->begin_frame(1200, 800);
        ctx.frame.reset(); // Reset Arena
        
        // --- Phase 10: Debug Setup ---
        ctx.config.show_debug_overlay = true;
        ctx.config.deterministic_mode = true; 
        
        // --- UI Definition ---
        // Root
        BoxConfig root_cfg = Column().width(1200).height(800).bg({0.1f, 0.1f, 0.12f, 1.0f}).padding(40);
        root_cfg.prop([&]{
            
            Text("Phase N: Visual Juice Verification").size(32).color({1,1,1,1}).margin(0,0,20,0);
            
            // Phase 10: Debug Overlay Test
            DebugOverlay::text("Frame ID: %d", frame_count);
            DebugOverlay::text("Mouse: %.1f, %.1f", ctx.input.mouse_x, ctx.input.mouse_y);
            DebugOverlay::bar("Animation Progress", (float)(frame_count % 100) / 100.0f);

            // 1. Auto-Transition Test
            Text("1. Auto-Transition (Hover Buttons)").size(20).color({0.7,0.7,0.8,1}).margin(0,0,10,0);
            Row().prop([&]{
                Button("Standard Button").margin(10);
                Button("Blue Button").bg({0.2, 0.4, 0.8, 1}).margin(10);
                Button("Red Button").bg({0.8, 0.2, 0.2, 1}).margin(10);
            }); End();
            
            // 2. Glassmorphism Test
            Box().height(40); // Spacer
            Text("2. Glassmorphism (Backdrop Blur)").size(20).color({0.7,0.7,0.8,1}).margin(0,0,10,0);
            
            // Create a colorful background to test blur
            BoxConfig bg_box = Box().width(400).height(200).bg({0.2,0.2,0.3,1});
            bg_box.prop([&]{
                // Background pattern
                Box().width(50).height(50).bg({1,0,0,1}).margin(20);
                Box().width(50).height(50).bg({0,1,0,1}).margin(20, 100);
                
                // Glass Panel overlay
                Box().width(300).height(100).bg({1, 1, 1, 0.2f}).blur(15.0f).radius(16).margin(-100, 50).prop([&]{
                     Text("Liquid Glass Panel").color({1,1,1,1});
                }).elevation(15);
            });
            
            // 3. Bezier Test
            Box().height(40);
            Text("3. Bezier Curves").size(20).color({0.7,0.7,0.8,1}).margin(0,0,10,0);
            Box().width(300).height(200).bg({0.15,0.15,0.2,1}).prop([&]{
                Bezier().points(20, 180, 50, 20, 250, 20, 280, 180)
                        .color({1, 0.8, 0.2, 1}).thickness(4.0f);
            });

        });
        End();
        ViewHeader* root = root_cfg;
        if (frame_count == 10) {
            debug_dump(root->id);
        }

        // --- Render ---
        internal::DrawList dl;
        RenderUI(root, 1200, 800, dl);
        ctx.world.backend->render(dl);

        ctx.world.backend->end_frame();
        
        ctx.input.advance_frame();
        
        // DT Simulation
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = end_time - start_time;
        ctx.frame.dt = elapsed.count();
        if (ctx.frame.dt < 0.001f) ctx.frame.dt = 0.016f; // Min DT
        ctx.frame.time += ctx.frame.dt;
        
        // Frame/Animation Tick is handled inside RenderUI (Animation Physics Step)
        
        if (auto_screenshot && frame_count == 5) {
            ctx.world.backend->capture_screenshot("juice_verify.png");
            ctx.world.backend->end_frame();
            break; // Exit
        }
    }
    
    internal::GetEngineContext().world.backend->shutdown();
    return 0;
}
