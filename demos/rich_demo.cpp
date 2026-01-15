// V5 Rich Demo: The Fantasmagorie Editor
// Showcasing: Complex Layout, High-Fidelity Styling, and All Widgets

#include "view/api.hpp"
#include "text/font_manager.hpp"
#include "utils/fs_cache.hpp" // New util
#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <random>

// V5 Core API
namespace fanta {
    void Init(int width, int height, const char* title);
    int GetScreenWidth();
    int GetScreenHeight();
    bool RunWithUI(std::function<void(::fanta::internal::DrawList&)> ui_loop);
}

// Mock Data
struct LogEntry {
    std::string time;
    std::string level;
    std::string message;
};

std::vector<LogEntry> g_logs = {
    {"12:01:05", "INFO", "System initialized"},
    {"12:01:06", "WARN", "Texture 'skybox.png' load slow (50ms)"},
    {"12:01:06", "INFO", "Render graph built"},
    {"12:01:07", "ERROR", "Shader 'pbr.frag' compilation failed"},
    {"12:01:07", "INFO", "Fallback shader activated"}
};

float g_fps_data[100];
void UpdateMockData() {
    static int offset = 0;
    g_fps_data[offset] = 60.0f + (rand() % 10 - 5);
    offset = (offset + 1) % 100;
}

// Editor State
struct EditorState {
    bool show_sidebar = true;
    bool show_inspector = true;
    float sidebar_width = 250.0f;
    float inspector_width = 300.0f;
    
    // Inspector Props
    fanta::internal::ColorF object_color = {0.8f, 0.4f, 0.2f, 1.0f};
    float object_roughness = 0.5f;
    float object_metallic = 0.1f;
    bool is_visible = true;
    char name_buffer[64] = "HeroCharacter";
    
    // Tree State
    bool tree_root = true;
    bool tree_assets = true;
    bool tree_models = false;
    
    // Layout State
    float split_ratio = 0.2f;
    int bottom_tab = 0;
    
    // Content State
    char editor_text[4096] = "Welcome to Fantasmagoriec Editor.\nThis is a multi-line text area.\nSupports IME and Scrolling.\n\nTry typing some Japanese: こんにちは。\n";
    float transform_pos[3] = {0.0f, 0.0f, 0.0f};

    // FileBrowser State
    std::string current_path = "C:/"; // Default start
    std::vector<fanta::utils::FileEntry> current_files;
    bool fb_dirty = true; // Needs refresh
};

static EditorState g_state;

// Moved from static blocks to Global State for V5 compliance
// "State Lives Outside"
void InitState() {
     // Initialize defaults if needed
}

int main() {
    using namespace fanta;
    using namespace fanta::ui;
    
    Init(1600, 900, "Fantasmagorie V5: Editor Showcase");
    
    // Load System Font for Japanese Support
    // Init() loads Segoe UI as primary (0). We load MS Gothic as secondary.
    auto font_jp = internal::FontManager::Get().load_font("C:/Windows/Fonts/msgothic.ttc");
    internal::FontManager::Get().add_fallback_font(font_jp);
    
    // Fill initial plot data
    for (int i = 0; i < 100; ++i) g_fps_data[i] = 60.0f;
    
    RunWithUI([&](internal::DrawList& dl) {
        UpdateMockData();
        
        // Root: Full Screen, Deep Dark Theme
        auto root = Column()
            .width((float)GetScreenWidth())
            .height((float)GetScreenHeight())
            .bg({0.12f, 0.12f, 0.14f, 1.0f});
        {
            // --- TOP BAR (Menu) ---
            BeginMenuBar()
                .bar_height(30)
                .bg({0.18f, 0.18f, 0.20f, 1.0f})
                .shadow(5);
            {
                if (BeginMenu("File")) {
                    if (MenuItem("New Project", "Ctrl+N")) {}
                    if (MenuItem("Open...", "Ctrl+O")) {}
                    MenuSeparator();
                    if (MenuItem("Exit", "Alt+F4")) { exit(0); }
                    EndMenu();
                }
                if (BeginMenu("Edit")) {
                    if (MenuItem("Undo", "Ctrl+Z")) {}
                    if (MenuItem("Redo", "Ctrl+Y")) {}
                    EndMenu();
                }
                if (BeginMenu("View")) {
                    if (MenuItem("Toggle Sidebar", nullptr)) g_state.show_sidebar = !g_state.show_sidebar;
                    if (MenuItem("Toggle Inspector", nullptr)) g_state.show_inspector = !g_state.show_inspector;
                    EndMenu();
                }
            }
            EndMenuBar();
            
            // --- MAIN CONTENT AREA (Splitter) ---
            Splitter(g_state.split_ratio).grow(1).prop([&]{
                // [LEFT] Project Explorer
                if (g_state.show_sidebar) {
                    Column().grow(1).margin(4).padding(8) // Width controlled by Splitter logic in layout
                        .bg({0.15f, 0.15f, 0.17f, 1.0f})
                        .radius(8).prop([&]{
                            Text("プロジェクト (Project)").size(14).color({0.6f, 0.6f, 0.7f, 1.0f}).margin(8);
                            
                            // FileBrowser Logic
                            if (g_state.fb_dirty) {
                                g_state.current_files = fanta::utils::FileSystemCache::ScanDirectory(g_state.current_path);
                                g_state.fb_dirty = false;
                            }
                            
                            // Breadcrumb / Up
                            Row().height(24).margin(4).prop([&]{
                                if (Button("..")) {
                                    std::filesystem::path p(g_state.current_path);
                                    if (p.has_parent_path()) {
                                        g_state.current_path = p.parent_path().string();
                                        g_state.fb_dirty = true;
                                    }
                                }
                                Text(g_state.current_path.c_str()).size(12).color({0.8f, 0.8f, 0.8f, 1.0f}).margin(4);
                            });
                            End();

                            Scroll().grow(1).prop([&]{
                                for (const auto& entry : g_state.current_files) {
                                    if (entry.is_directory) {
                                        // Directory: Yellow-ish
                                        if (Button(entry.name.c_str())) {
                                            g_state.current_path = entry.path;
                                            g_state.fb_dirty = true;
                                        }
                                    } else {
                                        // File: White
                                        Text(entry.name.c_str()).margin(4, 8).color({0.7f, 0.9f, 1.0f, 1.0f});
                                    }
                                }
                            });
                        });
                } else {
                    // Logic to handle hidden sidebar in Splitter if needed, or just dummy
                    Box().width(0);
                }
                
                // [CENTER] Viewport & Console
                Column().grow(1).margin(4).prop([&]{
                    // 3D Viewport Mock
                    Box().grow(2).bg({0.05f, 0.05f, 0.06f, 1.0f})
                        .radius(8)
                        .shadow(10).squircle().prop([&]{
                        // Overlay FPS
                        Row().padding(10).height(40).align(Align::Center);
                        {
                            Text("VIEWPORT").size(16).color({1,1,1,0.3f});
                        }
                        End();
                    }); 
                    End();
                    
                        // Console / Editor Tabs
                        Box().grow(1).margin(4)
                            .bg({0.15f, 0.15f, 0.17f, 1.0f})
                            .radius(8).prop([&]{
                        // static int bottom_tab removed
                        
                        // Tab Header
                        Row().height(34).padding(4).bg({0.2f, 0.2f, 0.22f, 1.0f}).radius(4);
                        {
                            if (Button("Console")) g_state.bottom_tab = 0;
                            if (Button("Editor")) g_state.bottom_tab = 1;
                        }
                        End();
                        
                        // Tab Content
                        if (g_state.bottom_tab == 0) {
                            // Console List
                            Scroll().grow(1).padding(4);
                            {
                                for (const auto& log : g_logs) {
                                    Row().height(20);
                                    {
                                        Text(log.time.c_str()).width(80).color({0.5f, 0.5f, 0.5f, 1.0f});
                                        fanta::internal::ColorF level_col = {0.8f, 0.8f, 0.8f, 1.0f};
                                        if (log.level == "ERROR") level_col = {1.0f, 0.4f, 0.4f, 1.0f};
                                        if (log.level == "WARN") level_col = {1.0f, 0.8f, 0.4f, 1.0f};
                                        Text(log.level.c_str()).width(60).color(level_col);
                                        Text(log.message.c_str()).grow(1).color({0.9f, 0.9f, 0.9f, 1.0f});
                                    }
                                    End();
                                }
                            }
                            End();
                        }
                        else if (g_state.bottom_tab == 1) {
                             // Editor
                             // static char editor_text removed
                             
                             Column().grow(1).padding(0);
                             {
                                 Scroll(true).grow(1).bg({0.1f, 0.1f, 0.12f, 1.0f}).padding(10).prop([&]{
                                     TextArea(g_state.editor_text, 4096).grow(1);
                                 });
                             }
                             End();
                        }
                    });
                    End();
                });
                End();
            });
            End(); // Main Content Row
                if (g_state.show_inspector) {
                    Scroll().width(g_state.inspector_width).margin(4).padding(10)
                        .bg({0.16f, 0.16f, 0.18f, 1.0f})
                        .radius(8);
                    {
                        Text("Inspector").size(16).margin(0, 0, 10, 0); // Multi-arg margin!
                        
                        // Object Name
                        Text("Name").size(12).color({0.6f, 0.6f, 0.7f, 1.0f});
                        TextInput(g_state.name_buffer, 64).height(28).margin(0, 0, 10, 0);
                        
                        // Visibility
                        Row().height(30).align(Align::Center);
                        {
                            Toggle("Visible", &g_state.is_visible);
                        }
                        End();
                        
                        // Transform
                        Text("Transform").size(12).color({0.6f, 0.6f, 0.7f, 1.0f}).margin(10, 0, 4, 0);
                        Row().height(28).margin(0, 0, 4, 0);
                        {
                            Text("X").width(15); Slider("##X", &g_state.transform_pos[0], -10, 10).grow(1);
                            Text("Y").width(15); Slider("##Y", &g_state.transform_pos[1], -10, 10).grow(1);
                            Text("Z").width(15); Slider("##Z", &g_state.transform_pos[2], -10, 10).grow(1);
                        }
                        End();
                        
                        // Material
                        Text("Material").size(12).color({0.6f, 0.6f, 0.7f, 1.0f}).margin(15, 0, 4, 0);
                        
                        Text("Albedo").size(11).color({0.5f, 0.5f, 0.5f, 1.0f});
                        ColorPicker(&g_state.object_color).size(200); // Pointer overload
                        
                        Text("Roughness").size(11).color({0.5f, 0.5f, 0.5f, 1.0f}).margin(8,0,0,0);
                        Slider("Roughness", &g_state.object_roughness, 0.0f, 1.0f).height(24).margin(0,0,8,0);
                        
                        Text("Metallic").size(11).color({0.5f, 0.5f, 0.5f, 1.0f});
                        Slider("Metallic", &g_state.object_metallic, 0.0f, 1.0f).height(24);
                        
                        // Performance Plot
                        Text("Performance").size(12).color({0.6f, 0.6f, 0.7f, 1.0f}).margin(20, 0, 4, 0);
                        Box().height(100).bg({0,0,0,0.2f}).radius(4);
                        {
                            Plot(g_fps_data, 100).color({0.4f, 0.9f, 0.4f, 1.0f});
                        }
                        End();
                        
                        Button("Apply Changes").height(32).margin(20, 0, 0, 0)
                            .bg({0.3f, 0.4f, 0.8f, 1.0f}).radius(4).shadow(4);
                    }
                    End();
                } // Close if show_inspector

            
            // --- STATUS BAR ---
            Row().height(24).padding(4).align(Align::Center)
                .bg({0.3f, 0.3f, 0.8f, 1.0f}); // Accent Color Status Bar
            {
                Text("Ready").size(12).color({1,1,1,1}).margin(8);
                Box().grow(1); // Spacer
                Text("V5 Engine | FPS: 60").size(12).color({1,1,1,0.7f}).margin(8);
            }
            End();
        }
        End(); // Root
        
        RenderUI(root, (float)GetScreenWidth(), (float)GetScreenHeight(), dl);
    });
    
    return 0;
}
