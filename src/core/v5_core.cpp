// V5 Core Logic - Single Source of Truth
// Minimal engine: Init, BeginFrame, EndFrame, Render

#include "core/contexts_internal.hpp"
#include "backend/drawlist.hpp"
#include "core/font_types.hpp"
#include "text/font_manager.hpp"
#include "text/glyph_cache.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <chrono>
#include <memory>
#include <functional>

namespace fanta {

// --- V5 Node (Minimal, POD-compatible) ---
enum class NodeType { Box, Label };

struct Color { uint8_t r=0, g=0, b=0, a=255; };

struct Node {
    NodeType type = NodeType::Box;
    float width = 0;
    float height = 0;
    internal::ColorF color = {0.2f, 0.2f, 0.25f, 1.0f};
    float corner_radius = 0;
    float elevation = 0;
    const char* text = nullptr;
    Node* children[8] = {nullptr};  // Fixed array, POD-compatible
    int child_count = 0;
};
static_assert(std::is_trivially_destructible_v<Node>, "Node must be trivially destructible for FrameArena!");

// Forward declaration
std::unique_ptr<Backend> CreateOpenGLBackend();

// Global Engine Context
static internal::EngineContext g_engine_ctx;

namespace internal {
    EngineContext& GetEngineContext() { return g_engine_ctx; }
    StateStore& GetStore() { return g_engine_ctx.store; }
    
    static uint64_t g_id_counter = 1;
    uint64_t& GetGlobalIDCounter() { return g_id_counter; }
    
    void Log(const std::string& msg) {
        std::cout << "[Fanta] " << msg << std::endl;
        std::cout.flush();
    }
}

#define g_ctx (internal::GetEngineContext())

// --- Public API ---

void Init(int width, int height, const char* title) {
    internal::Log("Init V5 Standard");
    
    g_ctx.runtime.width = width;
    g_ctx.runtime.height = height;
    
    g_ctx.world.backend = CreateOpenGLBackend();
    if (!g_ctx.world.backend->init(width, height, title)) {
        internal::Log("Backend init failed!");
        exit(1);
    }

    // Initialize Text Engine (Phase D)
    if (!internal::FontManager::Get().init()) {
        internal::Log("FontManager init failed!");
    }
    internal::GlyphCache::Get().init(g_ctx.world.backend.get());

    // Load Default Font
#ifdef _WIN32
    internal::FontManager::Get().load_font("C:/Windows/Fonts/segoeui.ttf");
#else
    internal::FontManager::Get().load_font("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
#endif
}

void Shutdown() {
    internal::Log("Shutdown V5");
    g_ctx.world.backend->shutdown();
    g_ctx.world.backend.reset();
}

void BeginFrame() {
    g_ctx.frame.reset();
    g_ctx.store.clear_frame();
    
    // Advance input state (facts: previous frame → current frame)
    g_ctx.input.advance_frame();
    
    if (g_ctx.world.backend) {
        g_ctx.world.backend->get_mouse_state(
            g_ctx.input.mouse_x,
            g_ctx.input.mouse_y,
            g_ctx.input.mouse_down,
            g_ctx.input.mouse_wheel
        );
        g_ctx.world.backend->get_keyboard_events(g_ctx.input.chars, g_ctx.input.keys);
        
        // Handle Debug Keys (F5: Pause, F6: Step)
        for (int key : g_ctx.input.keys) {
            if (key == 294) { // GLFW_KEY_F5
                g_ctx.config.is_paused = !g_ctx.config.is_paused;
                internal::Log(g_ctx.config.is_paused ? "Engine PAUSED" : "Engine RESUMED");
            }
            if (key == 295) { // GLFW_KEY_F6
                if (g_ctx.config.is_paused) {
                    g_ctx.config.step_requested = true;
                }
            }
        }
    }
    
    g_ctx.world.backend->begin_frame(g_ctx.runtime.width, g_ctx.runtime.height);
}

void EndFrame() {
    internal::DrawList dl;
    
    for (const auto& el : g_ctx.store.frame_elements) {
        dl.add_rounded_rect(
            {el.layout.x, el.layout.y},
            {el.layout.w, el.layout.h},
            el.corner_radius,
            el.color,
            el.elevation,
            false, 0.0f, {0, 0}
        );
    }
    
    g_ctx.world.backend->render(dl);
    g_ctx.world.backend->end_frame();
}

bool Run(std::function<void()> app_loop) {
    using Clock = std::chrono::high_resolution_clock;
    auto last = Clock::now();
    
    while (g_ctx.world.backend->is_running()) {
        auto now = Clock::now();
        float real_dt = std::chrono::duration<float>(now - last).count();
        last = now;
        
        // Time logic: Normal, Paused, or Stepped
        float dt = real_dt;
        if (g_ctx.config.deterministic_mode) {
            dt = g_ctx.config.fixed_dt;
        }
        
        bool should_update = !g_ctx.config.is_paused || g_ctx.config.step_requested;
        
        if (!should_update) {
            dt = 0.0f;
        } else if (g_ctx.config.is_paused && g_ctx.config.step_requested) {
            dt = g_ctx.config.fixed_dt;
            g_ctx.config.step_requested = false;
        }
        
        g_ctx.runtime.dt = dt;
        
        BeginFrame();
        app_loop();
        EndFrame();
    }
    
    Shutdown();
    return true;
}

// Extension: Run with DrawList access for UI Layer
// This is NOT a core modification, it's an extension for User Land.
bool RunWithUI(std::function<void(internal::DrawList&)> ui_loop) {
    using Clock = std::chrono::high_resolution_clock;
    auto last = Clock::now();
    printf("[Core] Starting loop...\n");
    fflush(stdout);
    
    while (g_ctx.world.backend->is_running()) {
        auto now = Clock::now();
        float real_dt = std::chrono::duration<float>(now - last).count();
        last = now;
        
        // Time logic: Normal, Paused, or Stepped
        float dt = real_dt;
        if (g_ctx.config.deterministic_mode) {
            dt = g_ctx.config.fixed_dt;
        }
        
        bool should_update = !g_ctx.config.is_paused || g_ctx.config.step_requested;
        
        if (!should_update) {
            dt = 0.0f;
        } else if (g_ctx.config.is_paused && g_ctx.config.step_requested) {
            dt = g_ctx.config.fixed_dt;
            g_ctx.config.step_requested = false;
        }
        
        g_ctx.runtime.dt = dt;
        
        // BeginFrame (inline)
        g_ctx.frame.reset();
        g_ctx.store.clear_frame();
        
        // Advance input state (facts: previous frame → current frame)
        g_ctx.input.advance_frame();
        
        if (g_ctx.world.backend) {
            g_ctx.world.backend->get_mouse_state(
                g_ctx.input.mouse_x,
                g_ctx.input.mouse_y,
                g_ctx.input.mouse_down,
                g_ctx.input.mouse_wheel
            );
            g_ctx.world.backend->get_keyboard_events(g_ctx.input.chars, g_ctx.input.keys);
            
            // Handle Debug Keys (F5: Pause, F6: Step)
            for (int key : g_ctx.input.keys) {
                if (key == 294) { // GLFW_KEY_F5
                    g_ctx.config.is_paused = !g_ctx.config.is_paused;
                    internal::Log(g_ctx.config.is_paused ? "Engine PAUSED" : "Engine RESUMED");
                }
                if (key == 295) { // GLFW_KEY_F6
                    if (g_ctx.config.is_paused) {
                        g_ctx.config.step_requested = true;
                    }
                }
            }
        }
        g_ctx.world.backend->begin_frame(g_ctx.runtime.width, g_ctx.runtime.height);
        
        // User code with DrawList access
        internal::DrawList dl;
        ui_loop(dl);
        
        // Render and finish frame
        g_ctx.world.backend->render(dl);
        g_ctx.world.backend->end_frame();
    }
    
    Shutdown();
    return true;
}

// Helpers for User Land
int GetScreenWidth() { return g_ctx.runtime.width; }
int GetScreenHeight() { return g_ctx.runtime.height; }

// --- V5 View API ---
Node* Box(float w, float h, Color c) {
    auto* node = g_ctx.frame.arena.alloc<Node>();
    if (!node) return nullptr;
    node->type = NodeType::Box;
    node->width = w;
    node->height = h;
    node->color = {c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f};
    return node;
}

void Render(Node* root) {
    if (!root) return;
    
    internal::ElementState el;
    el.id = internal::ID(internal::GetGlobalIDCounter()++);
    el.layout = {0, 0, root->width, root->height};
    el.color = root->color;
    el.corner_radius = root->corner_radius;
    el.elevation = root->elevation;
    
    g_ctx.store.frame_elements.push_back(el);
    
    for (int i = 0; i < root->child_count; ++i) {
        Render(root->children[i]);
    }
}

} // namespace fanta
