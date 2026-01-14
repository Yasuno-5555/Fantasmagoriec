#include "fanta.h"

// --- Standard C++ Headers ---
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <algorithm>
#include <cmath>
#include <string>
#include <functional>
#include <chrono>

// --- Windows Header Guard ---
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <unknwn.h>
#endif

// --- Backend Headers ---
#include "core/types_internal.hpp"
#include "backend/opengl_backend.hpp"
#include "backend/drawlist.hpp"
#ifdef FANTA_USE_D3D11
#include "backend/d3d11_backend.hpp"
#endif

// --- Component Headers ---
#include "element/layout.hpp"
#include "text/font_manager.hpp"
#include "text/text_layout.hpp"
#include "core/input.hpp"
#include "core/markdown.hpp"
#include "core/contexts_internal.hpp"
#include "render/render_tree.hpp"

namespace fanta {

// --- Global Engine Context (Hidden) ---
static struct EngineContext {
    internal::StateStore store;
    std::unique_ptr<Backend> backend;
    std::vector<internal::ID> node_stack;
    
    internal::RuntimeState runtime;
    internal::InputContext input_ctx;
    internal::ThemeContext theme_ctx;
} g_ctx;

namespace internal {
    StateStore& GetStore() { return g_ctx.store; }
    RuntimeState& GetRuntime() { return g_ctx.runtime; }
    InputContext& GetInput() { return g_ctx.input_ctx; }
    ThemeContext& GetThemeCtx() { return g_ctx.theme_ctx; }
}

// Debug Logger
void Log(const std::string& msg) {
    std::cerr << "[DEBUG] " << msg << std::endl;
    static FILE* f = fopen("debug.txt", "a");
    if (f) { fprintf(f, "%s\n", msg.c_str()); fflush(f); }
}

// --- API Implementation ---

void Init(int width, int height, const char* title) {
    Log("Init Start");
    g_ctx.runtime.width = width;
    g_ctx.runtime.height = height;
#ifdef FANTA_USE_D3D11
    g_ctx.backend = std::make_unique<D3D11Backend>();
#else
    g_ctx.backend = std::make_unique<OpenGLBackend>();
#endif
    
    if (!g_ctx.backend->init(width, height, title)) {
        Log("Backend Init Failed");
        std::cout << "Engine Init Failed!" << std::endl;
        exit(1);
    }
    
    if (!internal::FontManager::Get().init()) {
        Log("FontManager Init Failed");
    } else {
        internal::FontID font_id = internal::FontManager::Get().load_font("C:/Windows/Fonts/segoeui.ttf");
        if (font_id == 0 && internal::FontManager::Get().get_face(0) == nullptr) {
            font_id = internal::FontManager::Get().load_font("C:/Windows/Fonts/arial.ttf");
        }
    }

    Log("Init Success");
    SetTheme(Theme::Dark()); // Default Theme
}

void Shutdown() {
    Log("Shutdown");
    internal::FontManager::Get().shutdown();
    g_ctx.backend->shutdown();
    g_ctx.backend.reset();
}

void GetMousePos(float& x, float& y) {
    x = g_ctx.input_ctx.mouse_x;
    y = g_ctx.input_ctx.mouse_y;
}

bool IsMouseDown() {
    return g_ctx.input_ctx.mouse_down;
}

bool IsMouseJustPressed() {
    return g_ctx.input_ctx.state.is_just_pressed();
}

bool IsKeyDown(int key) {
#ifdef _WIN32
    return (GetAsyncKeyState(key) & 0x8000) != 0;
#else
    return false;
#endif
}

void BeginFrame() {
    g_ctx.input_ctx.state.update_frame();
    if (g_ctx.backend) {
        g_ctx.backend->get_mouse_state(
            g_ctx.input_ctx.mouse_x, 
            g_ctx.input_ctx.mouse_y, 
            g_ctx.input_ctx.mouse_down,
            g_ctx.input_ctx.mouse_wheel
        );
    }

    g_ctx.store.clear_frame();
    g_ctx.node_stack.clear();
    g_ctx.runtime.overlay_ids.clear();
    g_ctx.backend->begin_frame(g_ctx.runtime.width, g_ctx.runtime.height);
}

void EndFrame() {
    auto& elements = g_ctx.store.frame_elements;
    if (elements.empty()) return;

    std::map<internal::ID, size_t> id_map;
    for(size_t i=0; i<elements.size(); ++i) {
        id_map[elements[i].id] = i;
    }

    internal::LayoutEngine layout_engine;
    std::map<internal::ID, internal::ComputedLayout> layouts = layout_engine.solve(
        elements,
        id_map,
        (float)g_ctx.runtime.width,
        (float)g_ctx.runtime.height
    );

    internal::DrawList draw_list;
    for(const auto& el : elements) {
        if (el.parent == 0) {
            internal::RenderTree(el, layouts, elements, id_map, draw_list, g_ctx.runtime, g_ctx.input_ctx, g_ctx.store, 0, 0, 1.0f, false);
        }
    }

    for (auto id : g_ctx.runtime.overlay_ids) {
        auto it = id_map.find(id);
        if (it != id_map.end()) {
            internal::RenderTree(elements[it->second], layouts, elements, id_map, draw_list, g_ctx.runtime, g_ctx.input_ctx, g_ctx.store, 0, 0, 1.0f, true);
        }
    }

    g_ctx.backend->render(draw_list);
    
    if (g_ctx.runtime.screenshot_pending) {
#ifdef FANTA_USE_D3D11
        auto* d3d_backend = dynamic_cast<D3D11Backend*>(g_ctx.backend.get());
        if (d3d_backend) d3d_backend->capture_screenshot(g_ctx.runtime.screenshot_filename.c_str());
#else
        auto* gl_backend = dynamic_cast<OpenGLBackend*>(g_ctx.backend.get());
        if (gl_backend) gl_backend->capture_screenshot(g_ctx.runtime.screenshot_filename.c_str());
#endif
        g_ctx.runtime.screenshot_pending = false;
    }
    
    g_ctx.backend->end_frame();
}

bool Run(std::function<void()> app_loop) {
    using Clock = std::chrono::high_resolution_clock;
    auto last_time = Clock::now();
    
    try {
        while(g_ctx.backend->is_running()) {
            auto now = Clock::now();
            std::chrono::duration<float> duration = now - last_time;
            g_ctx.runtime.dt = duration.count();
            if (g_ctx.runtime.dt > 0.1f) g_ctx.runtime.dt = 0.1f;
            last_time = now;
            
            BeginFrame();
            app_loop();
            EndFrame();
        }
    } catch (...) {
        return false;
    }
    Shutdown();
    return true;
}

bool CaptureScreenshot(const char* filename) {
    g_ctx.runtime.screenshot_pending = true;
    g_ctx.runtime.screenshot_filename = filename;
    return true;
}

void SetTheme(const Theme& theme) {
    auto& ct = g_ctx.theme_ctx.current;
    ct.background = {theme.background.r, theme.background.g, theme.background.b, theme.background.a};
    ct.surface = {theme.surface.r, theme.surface.g, theme.surface.b, theme.surface.a};
    ct.primary = {theme.primary.r, theme.primary.g, theme.primary.b, theme.primary.a};
    ct.accent = {theme.accent.r, theme.accent.g, theme.accent.b, theme.accent.a};
    ct.text = {theme.text.r, theme.text.g, theme.text.b, theme.text.a};
    ct.text_muted = {theme.text_muted.r, theme.text_muted.g, theme.text_muted.b, theme.text_muted.a};
    ct.border = {theme.border.r, theme.border.g, theme.border.b, theme.border.a};
}

Theme GetTheme() {
    auto& ct = g_ctx.theme_ctx.current;
    Theme t;
    t.background = {ct.background.r, ct.background.g, ct.background.b, ct.background.a};
    t.surface = {ct.surface.r, ct.surface.g, ct.surface.b, ct.surface.a};
    t.primary = {ct.primary.r, ct.primary.g, ct.primary.b, ct.primary.a};
    t.accent = {ct.accent.r, ct.accent.g, ct.accent.b, ct.accent.a};
    t.text = {ct.text.r, ct.text.g, ct.text.b, ct.text.a};
    t.text_muted = {ct.text_muted.r, ct.text_muted.g, ct.text_muted.b, ct.text_muted.a};
    t.border = {ct.border.r, ct.border.g, ct.border.b, ct.border.a};
    return t;
}

} // namespace fanta
