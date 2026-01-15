#include "fanta.h"
#include "core/contexts_internal.hpp"
#include "core/contexts_internal.hpp"
#include "backend/backend_factory.hpp" 
#include "backend/drawlist.hpp"
#include "text/font_manager.hpp"
#include "core/plugin_loader.hpp"
#include "core/script_engine.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

// --- Windows Header Guard ---
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <unknwn.h>
#endif

namespace fanta {

    // --- Global Engine Context ---
    // Phase 31: TLS Context
    static thread_local internal::EngineContext* g_current_ctx = nullptr;
    static std::vector<std::unique_ptr<internal::EngineContext>> g_context_pool; // Owner of contexts

    namespace internal {
        struct LocalizationStore {
            std::string current_locale = "en";
            std::map<std::string, std::map<std::string, std::string>> string_bundles;
        };
        static LocalizationStore g_l10n;
        
        // Internal Accessor Implementation
        EngineContext& GetEngineContext() {
            if (!g_current_ctx) {
                std::cerr << "FATAL: No active Fantasmagorie context! Did you call Init() or MakeContextCurrent()?" << std::endl;
                exit(1);
            }
            return *g_current_ctx;
        }

        // Phase 2: EngineContext Constructor
        EngineContext::EngineContext() {
            // Default init
        }

        // TLS Instance
        // TODO: This g_ctx in internal namespace is shadowing/confusing with valid TLS approach? 
        // In fanta.cpp line 150: thread_local EngineContext g_ctx; 
        // But then GetEngineContext returns referencing it? 
        // Wait, line 101 says `static thread_local internal::EngineContext* g_current_ctx`.
        // Line 150 says `thread_local EngineContext g_ctx;` inside internal namespace? 
        // Ah, the original code had TWO ways.
        // Line 122: `#define g_ctx (fanta::internal::GetEngineContext())`
        // So `g_ctx` macro uses the pointer.
        // But line 150 defined a thread_local instance.
        // And GetEngineContext returned `g_ctx`. 
        // Wait, if line 150 defines `g_ctx` instance, and line 122 defines macro `g_ctx`... collision?
        // No, line 150 is inside `namespace internal`. Macro is global.
        // Usage inside namespace internal: `g_ctx` refers to the variable.
        // Usage outside: `g_ctx` refers to macro.
        // This is messy. I will preserve the *intent* which is the Pointer approach (Multi-window).
        // I will use `g_current_ctx` logic primarily.
        
        // V4 Legacy Accessor
        // EngineContext& GetEngineContext() { return *g_current_ctx; } // Already defined above

        // V5 TLS Accessor
        RuntimeContext& GetContext() {
            static thread_local RuntimeContext r_ctx;
            // Bind to current g_ctx members for now (Hybrid)
            if (r_ctx.world == nullptr) {
                 auto& ctx = GetEngineContext();
                 r_ctx.world = &ctx.world;
                 r_ctx.frame = &ctx.frame;
                 r_ctx.store = &ctx.store;
                 r_ctx.input = &ctx.input_ctx;
            }
            return r_ctx;
        }
    } // namespace internal
    
    // Macro Compat
    #define g_ctx (fanta::internal::GetEngineContext())

    namespace L10n {
        void SetLocale(const std::string& locale) {
            internal::g_l10n.current_locale = locale;
        }
        void AddStrings(const std::string& locale, const std::map<std::string, std::string>& strings) {
            auto& bundle = internal::g_l10n.string_bundles[locale];
            for (auto const& [key, val] : strings) bundle[key] = val;
        }
        std::string Translate(const std::string& key) {
            auto it = internal::g_l10n.string_bundles.find(internal::g_l10n.current_locale);
            if (it != internal::g_l10n.string_bundles.end()) {
                auto it2 = it->second.find(key);
                if (it2 != it->second.end()) return it2->second;
            }
            return key;
        }
    }

    namespace internal {
        // Debug Logger
        void Log(const std::string& msg) {
            std::cerr << "[DEBUG] " << msg << std::endl;
            FILE* f = fopen("debug.txt", "a");
            if (f) { fprintf(f, "%s\n", msg.c_str()); fclose(f); }
        }

        // Internal Accessor Implementation
        StateStore& GetStore() { return GetEngineContext().store; }
        RuntimeState& GetRuntime() { return GetEngineContext().runtime; }
        InputContext& GetInput() { return GetEngineContext().input_ctx; }
        ThemeContext& GetThemeCtx() { return GetEngineContext().theme_ctx; }
    }

    // --- Context Management ---
    Context* CreateContext(const ContextConfig& config) {
        auto ctx = std::make_unique<internal::EngineContext>();
        g_context_pool.push_back(std::move(ctx));
        return (Context*)g_context_pool.back().get();
    }

    void MakeContextCurrent(Context* ctx) {
        g_current_ctx = (internal::EngineContext*)ctx;
    }

    Context* GetCurrentContext() {
        return (Context*)g_current_ctx;
    }

    void DestroyContext(Context* ctx) {
        // TODO: Pool cleanup
    }

    void Init(int width, int height, const char* title) {
        // Phase 31: Create Default Context
        ContextConfig cfg;
        cfg.width = width;
        cfg.height = height;
        cfg.title = title;
        
        Context* ctx = CreateContext(cfg);
        MakeContextCurrent(ctx);
        
        // Log build timestamp
        FILE* f = fopen("debug.txt", "w");
        if (f) {
            fprintf(f, "[DEBUG] Init Start - Build %s %s\n", __DATE__, __TIME__);
            fclose(f);
        }
        
        Log("Init Start");
        g_ctx.runtime.width = width;
        g_ctx.runtime.height = height;
        
        // Backend Choice
#ifdef FANTA_USE_D3D11
        g_ctx.world.backend = CreateD3D11Backend();
#elif defined(FANTA_USE_WEBGPU)
        extern std::unique_ptr<Backend> CreateWebGPUBackend();
        g_ctx.world.backend = CreateWebGPUBackend();
#else
        g_ctx.world.backend = CreateOpenGLBackend();
#endif
        
        if (!g_ctx.world.backend->init(width, height, title)) {
            Log("Backend Init Failed");
            std::cout << "Engine Init Failed!" << std::endl;
            exit(1);
        }
        
        // Phase 4: Init Glyph Cache GPU resources
        internal::GlyphCache::Get().init(g_ctx.world.backend.get());

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
        
        // Phase 40: God-level Typography Defaults
        auto& typography = g_ctx.theme_ctx.current.typography;
        typography[static_cast<size_t>(TextToken::LargeTitle)] = { 34, 700, -0.6f, 1.1f };
        typography[static_cast<size_t>(TextToken::Title1)]     = { 28, 700, -0.5f, 1.1f };
        typography[static_cast<size_t>(TextToken::Title2)]     = { 22, 600, -0.4f, 1.2f };
        typography[static_cast<size_t>(TextToken::Title3)]     = { 20, 600, -0.2f, 1.2f };
        typography[static_cast<size_t>(TextToken::Headline)]   = { 17, 600, -0.2f, 1.2f };
        typography[static_cast<size_t>(TextToken::Body)]       = { 17, 400,  0.0f, 1.3f };
        typography[static_cast<size_t>(TextToken::Caption1)]   = { 12, 400,  0.2f, 1.3f };
        typography[static_cast<size_t>(TextToken::Caption2)]   = { 11, 400,  0.3f, 1.3f };
        
        // Phase 24: Default Script Engine
        static internal::NullScriptEngine s_null_engine;
        internal::ScriptManager::Get().set_engine(&s_null_engine);

        // Init ID Stack with a root seed
        g_ctx.runtime.id_stack.clear();
        g_ctx.runtime.id_stack.push_back(ID("Root"));
    }

    void Shutdown() {
        Log("Shutdown");
        internal::FontManager::Get().shutdown();
        g_ctx.world.backend->shutdown();
        g_ctx.world.backend.reset();
    }
    
    namespace Plugins {
        bool Load(const std::string& path) { return internal::PluginManager::Get().LoadPlugin(path); }
        void UnloadAll() { internal::PluginManager::Get().UnloadAll(); }
    }

    namespace Scripting {
        bool Run(const std::string& code) {
            auto engine = internal::ScriptManager::Get().get_engine();
            return engine ? engine->run_string(code) : false;
        }
        bool LoadFile(const std::string& path) {
            auto engine = internal::ScriptManager::Get().get_engine();
            return engine ? engine->run_file(path) : false;
        }
    }

    // --- ID Stack Implementation ---
    void PushID(ID id) {
        if (g_ctx.runtime.id_stack.empty()) {
            g_ctx.runtime.id_stack.push_back(id);
        } else {
            ID parent = g_ctx.runtime.id_stack.back();
            uint64_t mixed = parent.value ^ id.value;
            mixed *= 1099511628211ULL;
            g_ctx.runtime.id_stack.push_back(ID(mixed));
        }
    }

    void PushID(const char* str_id) {
        PushID(ID(str_id));
    }

    void PopID() {
        if (g_ctx.runtime.id_stack.size() > 1) { 
            g_ctx.runtime.id_stack.pop_back();
        }
    }

    // --- Theme Utils ---
    Theme Theme::Dark() {
        Theme t;
        t.palette[static_cast<size_t>(ColorToken::SystemBackground)] = 0xFF1C1C1E; 
        t.palette[static_cast<size_t>(ColorToken::SecondarySystemBackground)] = 0xFF2C2C2E; 
        t.palette[static_cast<size_t>(ColorToken::TertiarySystemBackground)] = 0xFF3A3A3C; 
        t.palette[static_cast<size_t>(ColorToken::Label)] = 0xFFFFFFFF;
        t.palette[static_cast<size_t>(ColorToken::SecondaryLabel)] = 0x99EBEBF5; // 60%
        t.palette[static_cast<size_t>(ColorToken::TertiaryLabel)] = 0x4DEBEBF5; // 30%
        t.palette[static_cast<size_t>(ColorToken::Accent)] = 0xFF0A84FF; 
        t.palette[static_cast<size_t>(ColorToken::Separator)] = 0x66545458; 
        t.palette[static_cast<size_t>(ColorToken::Fill)] = 0x33787880; 
        t.typography[static_cast<size_t>(TextToken::LargeTitle)] = { 34, 700, -0.6f, 1.1f };
        t.typography[static_cast<size_t>(TextToken::Title1)]     = { 28, 700, -0.5f, 1.1f };
        t.typography[static_cast<size_t>(TextToken::Title2)]     = { 22, 600, -0.4f, 1.2f };
        t.typography[static_cast<size_t>(TextToken::Title3)]     = { 20, 600, -0.2f, 1.2f };
        t.typography[static_cast<size_t>(TextToken::Headline)]   = { 17, 600, -0.2f, 1.2f };
        t.typography[static_cast<size_t>(TextToken::Body)]       = { 17, 400,  0.0f, 1.3f };
        t.typography[static_cast<size_t>(TextToken::Caption1)]   = { 12, 400,  0.2f, 1.3f };
        t.typography[static_cast<size_t>(TextToken::Caption2)]   = { 11, 400,  0.3f, 1.3f };
        return t;
    }

    Theme Theme::Light() {
        Theme t = Theme::Dark();
        t.palette[static_cast<size_t>(ColorToken::SystemBackground)] = 0xFFFFFFFF;
        t.palette[static_cast<size_t>(ColorToken::Label)] = 0xFF000000;
        t.palette[static_cast<size_t>(ColorToken::SecondaryLabel)] = 0x993C3C43;
        return t;
    }

    Theme Theme::AppleHIG() { return Dark(); } 
    Theme Theme::Fantasmagorie() {
        Theme t = Dark();
        t.palette[static_cast<size_t>(ColorToken::Accent)] = 0xFFFF00FF; 
        t.palette[static_cast<size_t>(ColorToken::SystemBackground)] = 0xFF050505; 
        return t;
    }
    
    static internal::ColorF ToInternalColor(uint32_t c) {
        float a = (c & 0xFF) / 255.0f;
        float b = ((c >> 8) & 0xFF) / 255.0f;
        float g = ((c >> 16) & 0xFF) / 255.0f;
        float r = ((c >> 24) & 0xFF) / 255.0f;
        return {r, g, b, a};
    }

    static uint32_t FromInternalColor(const internal::ColorF& c) {
        uint8_t r = static_cast<uint8_t>(c.r * 255);
        uint8_t g = static_cast<uint8_t>(c.g * 255);
        uint8_t b = static_cast<uint8_t>(c.b * 255);
        uint8_t a = static_cast<uint8_t>(c.a * 255);
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    void SetTheme(const Theme& theme) {
        auto& ct = g_ctx.theme_ctx.current;
        for (size_t i = 0; i < static_cast<size_t>(ColorToken::Count); ++i) {
            ct.palette[i] = ToInternalColor(theme.palette[i]);
        }
        for (size_t i = 0; i < static_cast<size_t>(TextToken::Count); ++i) {
            ct.typography[i].size = theme.typography[i].size;
            ct.typography[i].weight = theme.typography[i].weight;
            ct.typography[i].tracking = theme.typography[i].tracking;
            ct.typography[i].line_height = theme.typography[i].line_height;
        }
    }

    Theme GetTheme() {
        auto& ct = g_ctx.theme_ctx.current;
        Theme t;
        for (size_t i = 0; i < static_cast<size_t>(ColorToken::Count); ++i) {
            t.palette[i] = FromInternalColor(ct.palette[i]);
        }
        for (size_t i = 0; i < static_cast<size_t>(TextToken::Count); ++i) {
            t.typography[i] = ct.typography[i];
        }
        return t;
    }

    Color Color::alpha(float a) const {
        Color c = *this;
        uint8_t new_a = (uint8_t)(std::max(0.0f, std::min(a, 1.0f)) * 255.0f);
        c.rgba = (c.rgba & 0xFFFFFF00) | new_a;
        return c;
    }

} // namespace fanta
