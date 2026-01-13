#define NOMINMAX
#include "core/types.hpp"
#include "core/animation.hpp"
#define FANTA_ENABLE_TEXT
#include "v3/core/paint_list.hpp"
#include "core/font_manager.hpp"
#include "platform/opengl.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>

#undef rect
#undef circle
#undef line
#undef text

using namespace fanta;

int main() {
    std::cout << "Starting demo..." << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        return -1;
    }
    std::cout << "GLFW Initialized" << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 800, "Fantasmagorie v3-Final Pimpl Demo", nullptr, nullptr);
    if (!window) { 
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate(); return -1; 
    }
    std::cout << "Window Created" << std::endl;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    RenderBackend backend;
    std::cout << "Backend Constructed" << std::endl;
    if (!backend.init()) { std::cerr << "Failed to init backend" << std::endl; return -1; }
    std::cout << "Backend Initialized" << std::endl;

    // Load fonts and get IDs directly
    uint32_t default_font = FontManager::instance().load_font("C:/Windows/Fonts/arial.ttf", "default");
    uint32_t jp_font = FontManager::instance().load_font("C:/Windows/Fonts/msgothic.ttc", "jp");
    
    // Fallback if load failed
    if (jp_font == 0) jp_font = default_font;

    PaintList main_paint_list;
    main_paint_list.set_font(jp_font);

    int frame_count = 0;
    while (!glfwWindowShouldClose(window)) {
        if (frame_count < 5) std::cout << "Top of loop frame " << frame_count << std::endl;
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        backend.begin_frame(w, h, 1.0f);

        glClearColor(0.1f, 0.11f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        main_paint_list.clear();
        
        // 1. Stress Test: 10,000 Rects (proving performance)
        for(int i=0; i<100; ++i) {
            for(int j=0; j<100; ++j) {
                main_paint_list.rect({(float)i*10, (float)j*8, 8, 6}, Color::Hex(0x334455FF));
            }
        }

        // 2. Beautiful Typography with Elevation
        auto& r = main_paint_list.rect({100, 100, 600, 400}, Color::Hex(0x1E1E1EFF));
        r.radius = 16.0f;
        r.shadow = ElevationShadow::Level3();
        r.border_color = Color::Hex(0x333333FF);
        r.border_width = 1.0f;

        main_paint_list.text({150, 180}, "Fantasmagorie v3", 48.0f, Color::White());
        main_paint_list.text({150, 240}, "Commercial-Grade Pimpl Backend", 24.0f, Color::Hex(0xAAAAAAFF));
        main_paint_list.text({150, 300}, "日本語テキストの表示 (MS Gothic)", 32.0f, Color::Hex(0x66CCFFFF));

        // Stats Display
        auto stats = backend.get_stats();
        char buf[256];
        sprintf(buf, "Glyphs: %d | Atlas: %.2f%% | Pages: %d | Buf: %d", 
            stats.total_glyphs, stats.atlas_utilization * 100.0f, stats.atlas_pages, stats.active_buffer_idx);
        main_paint_list.text({150, 450}, buf, 18.0f, Color::Hex(0x00FF00FF));

        backend.execute(main_paint_list);
        backend.end_frame();

        glfwSwapBuffers(window);
        glfwPollEvents();
        frame_count++;
    }

    backend.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
