#include "application.hpp"
#include "../ui/api.hpp"
#include <iostream>
#include <vector>
#include <ctime>

// --- Demo Data ---
struct LogEntry {
    std::string time;
    std::string type;
    std::string msg;
};
static std::vector<LogEntry> g_logs;

void AddLog(const char* type, const char* msg) {
    time_t now = time(0);
    char buf[64];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    g_logs.push_back({buf, type, msg});
    if(g_logs.size() > 50) g_logs.erase(g_logs.begin());
}

// Settings State
static bool s_vsync = true;
static bool s_show_fps = true;
static bool s_dark_mode = true;
static std::string s_username = "User";

FantasmagorieCore::FantasmagorieCore() {}
FantasmagorieCore::~FantasmagorieCore() {
    if(ft_lib) FT_Done_FreeType(ft_lib);
}

// Procedural texture for testing
GLuint CreateCheckerTexture() {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    unsigned char pixels[64*64*4];
    for(int y=0; y<64; ++y) {
        for(int x=0; x<64; ++x) {
            bool white = ((x/8) + (y/8)) % 2 == 0;
            unsigned char c = white ? 255 : 128;
            int i = (y*64+x)*4;
            pixels[i] = c; pixels[i+1]=c; pixels[i+2]=c; pixels[i+3]=255;
            if (x==0 || x==63 || y==0 || y==63) { pixels[i]=255; pixels[i+1]=0; pixels[i+2]=0; } // red border
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

static GLuint g_test_tex = 0;

bool FantasmagorieCore::init(const char* title, int w, int h) {
    width = w; height = h;
    
    if(!glfwInit()) return false;
    // ... rest of init
    
    // (After context creation)
    glfwMakeContextCurrent(window);
    
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetCharCallback(window, glfw_char_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);

    renderer.init(); 
    g_test_tex = CreateCheckerTexture(); // Load Texture
    
    // Init Fonts (Correct API)
    if(FT_Init_FreeType(&ft_lib)) return false;
    
    atlas.init(); 
    if(!font.load("C:/Windows/Fonts/segoeui.ttf", 18, ft_lib, &atlas)) {
        font.load("C:/Windows/Fonts/arial.ttf", 18, ft_lib, &atlas);
    }
    
    ui::SetContext(&ctx);
    ctx.font = &font; // Phase 2
    ui::SetInteraction(&focus, &input);
    
    AddLog("INFO", "System Initialized");
    AddLog("WARN", "Low Memory Simulation");
    
    return true;
}

// Scroll state
float g_scroll_y = 0.0f;

// Phase 20: Node Graph Data
struct GraphNode {
    int id;
    float x, y;
    float w, h;
    std::string title;
};
struct GraphConn {
    int from, to;
};
struct GraphState {
    float pan_x = 0.0f;
    float pan_y = 0.0f;
    float zoom = 1.0f;
    bool is_panning = false;
    float last_mx=0, last_my=0;
    
    std::vector<GraphNode> nodes;
    std::vector<GraphConn> conns;
    
    void init() {
        nodes = {
            {1, 100, 100, 150, 100, "Input Source"},
            {2, 400, 150, 150, 120, "Processor"},
            {3, 700, 100, 150, 100, "Output"}
        };
        conns = { {1, 2}, {2, 3} };
    }
} g_graph;

void FantasmagorieCore::on_input_char(unsigned int codepoint) {
    // Simple ASCII for MVP, todo: UTF8 encode
    if (codepoint < 128) {
        ctx.input_chars.push_back((unsigned int)codepoint);
    }
}
void FantasmagorieCore::on_key(int key, int scancode, int action, int mods) {
    if (action == 0) focus.mouse_was_down = false; 
    
    // Phase 5: Input Logic
    if (action == 1 || action == 2) { // Press or Repeat
        if (key == 259) { // GLFW_KEY_BACKSPACE
            ctx.input_keys.push_back(key);
        }
    }
}
void FantasmagorieCore::on_scroll(double x, double y) {
    g_scroll_y += (float)y;
    // Zoom logic if Graph Active? simplified global
    g_graph.zoom += (float)y * 0.1f;
    if(g_graph.zoom < 0.1f) g_graph.zoom = 0.1f;
}
void FantasmagorieCore::glfw_key_callback(GLFWwindow* w, int k, int s, int a, int m) {
    ((FantasmagorieCore*)glfwGetWindowUserPointer(w))->on_key(k,s,a,m);
}
void FantasmagorieCore::glfw_char_callback(GLFWwindow* w, unsigned int c) {
    ((FantasmagorieCore*)glfwGetWindowUserPointer(w))->on_input_char(c);
}
void FantasmagorieCore::glfw_scroll_callback(GLFWwindow* w, double x, double y) {
    ((FantasmagorieCore*)glfwGetWindowUserPointer(w))->on_scroll(x,y);
}

void FantasmagorieCore::run() {
    while(!glfwWindowShouldClose(window)) {
         // Reset scroll for frame (impulse)? 
         // InputState takes instantaneous scroll or accumulated?
         // FocusManager applies logic: `s.scroll_y -= scroll_delta`.
         // So I need delta per frame. 
         // `g_scroll_y` accumulates in callback.
         // I should consume it.
         float frame_scroll = g_scroll_y;
         g_scroll_y = 0; // Reset
        
        glfwPollEvents();
        
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        
        bool mdown = glfwGetMouseButton(window, 0) == GLFW_PRESS;
        
        ctx.begin_frame();
        ui::UpdateInputState(window, (float)mx, (float)my, mdown, frame_scroll);

        ui::Begin("Root");
        ui::SetNextWidth((float)width);
        ui::SetNextHeight((float)height);
        ui::Column();
        
        // --- Top Bar ---
        ui::BeginGroup("TopBar");
        ui::SetNextHeight(40);
        ui::SetNextBg(ui::Color::Hex(0x222222FF));
        ui::Row();
        ui::SetNextAlign(ui::Align::Center);
        ui::SetNextPadding(10);
        ui::Label("Fantasmagorie UI");
        ui::Spacer(20);
        ui::EndGroup();
        
        // --- Tabs ---
        ui::BeginTabBar("MainTabs");
        if(ui::TabItem("Dashboard")) { }
        if(ui::TabItem("Graph")) { /* Init once? */ if(g_graph.nodes.empty()) g_graph.init(); }
        if(ui::TabItem("Settings")) { }
        if(ui::TabItem("Logs")) { }
        ui::EndTabBar();
        
        ui::Spacer(10);
        
        // --- Graph ---
        if(ui::IsTabActive("Graph")) {
             ui::SetNextGrow(1.0f);
             ui::BeginChild("Canvas", 0, 0); 
             
             // 1. Interaction Helper: Pan & Zoom
             ui::PanZoom(&g_graph.pan_x, &g_graph.pan_y, &g_graph.zoom);
             
             // 2. Apply Transform
             ui::PushScale(g_graph.zoom);
             ui::PushTranslate(g_graph.pan_x, g_graph.pan_y);

             // 3. Grid
             ui::CanvasGrid(g_graph.zoom, 100.0f);

             // 4. Draw Connections 
             // Smart Connections with Hover!
             for(auto& conn : g_graph.conns) {
                 GraphNode* n1 = nullptr; GraphNode* n2 = nullptr;
                 for(auto& n : g_graph.nodes) {
                     if(n.id == conn.from) n1 = &n;
                     if(n.id == conn.to) n2 = &n;
                 }
                 if(n1 && n2) {
                     float x1 = n1->x + n1->w;
                     float y1 = n1->y + 24 + 10; // Input Pin Center (approx)
                     float x2 = n2->x;
                     float y2 = n2->y + 24 + 30 + 5 + 10; // Output Pin Center (approx)
                     
                     // We need meaningful pin offsets in real usage (e.g. from Layout)
                     // For now hardcoded offsets based on `NodePin` order.
                     // Input Pin is first body item. Title(24) + Pad(8) + Spacer(0) + Pin(Row 20-ish/2)
                     // Let's rely on standard DrawConnection
                     
                     ui::DrawConnection(x1, y1, x2, y2, 0xFFAAAAFF, 3.0f);
                 }
             }

             // 5. Nodes (Declarative!)
             for (auto& node : g_graph.nodes) {
                 char nid[32]; sprintf(nid, "%d", node.id);
                 ui::BeginNode(nid, &node.x, &node.y);
                    ui::Text(node.title.c_str()); // Title Text (Inside TitleBar)
                    // Wait, BeginNode starts "Body" group at end?
                    // My implementation: 
                    // BeginNode -> Starts Node, Starts TitleBar, Starts TitleContent.
                    // User calls UI inside TitleContent?
                    // No, `BeginNode` finished TitleBar and started Body.
                    // So user UI goes into Body.
                    // Where did Title Text go?
                    // I put `Draggable` overlay in TitleBar.
                    // I should accept Title as arg in BeginNode? Or let user draw it?
                    // Implementation check: 
                    // `BeginGroup("TitleContent")` ... `Draggable` ... `EndGroup()` (TitleContent) ... `EndGroup()` (TitleBar).
                    // So TitleContent is empty except for Draggable overlay.
                    // This is Empty Title Bar.
                    // Refactor BeginNode to take title? Or leave TitleContent open?
                    // "Leave TitleContent open" is dangerous if user forgets `EndNode` structure.
                    // Better: `BeginNode(id, title, x, y)`?
                 
                 // Let's rely on Body content for now.
                 // We can add `ui::TextPositive("Title")` inside TitleBar if we modify BeginNode.
                 // For now, Node has empty title bar but is draggable.
                 // Content goes in Body.
                 ui::NodePin("Input", true);
                 ui::Spacer(5);
                 ui::Label("Parameters");
                 ui::Spacer(5);
                 ui::NodePin("Output", false);
                 
                 ui::EndNode();
                 
                 // Update size for connections
                 ui::GetLastNodeSize(&node.w, &node.h);
             }
             
             ui::PopTransform(); // Translate
             ui::PopTransform(); // Scale
             ui::EndChild();
        }
        
        // --- Dash --- (existing)
        else if(ui::IsTabActive("Dashboard")) {
            ui::BeginTable("StatsGrid", 3);
            ui::TableNextColumn(); 
            ui::BeginGroup("Card1"); ui::SetNextBg(ui::Color::Hex(0x333333FF)); ui::SetNextPadding(15); ui::Column();
            ui::Row(); ui::Image(g_test_tex, 48, 48); ui::Spacer(10); ui::Column(); // Icon
            ui::Label("Revenue"); ui::TextPositive("$1,240,000"); ui::EndGroup(); ui::EndGroup();
            ui::TableNextColumn();
            ui::BeginGroup("Card2"); ui::SetNextBg(ui::Color::Hex(0x333333FF)); ui::SetNextPadding(15); ui::Column();
            ui::Label("Expenses"); ui::TextNegative("$980,000"); ui::EndGroup();
            ui::TableNextColumn();
            ui::BeginGroup("Card3"); ui::SetNextBg(ui::Color::Hex(0x333333FF)); ui::SetNextPadding(15); ui::Column();
            ui::Label("Net Income"); ui::TextPositive("$260,000"); ui::EndGroup();
            ui::EndTable();
            ui::Spacer(20);
            ui::Label("Recent Transactions");
            ui::BeginTable("TxTable", 4);
                ui::TableNextRow();
                ui::TableNextColumn(); ui::Label("ID"); ui::TableNextColumn(); ui::Label("Date"); ui::TableNextColumn(); ui::Label("Amount"); ui::TableNextColumn(); ui::Label("Status");
                ui::TableNextRow();
                ui::TableNextColumn(); ui::Label("#1001"); ui::TableNextColumn(); ui::Label("2025-01-01"); ui::TableNextColumn(); ui::TextNegative("-$500.00"); ui::TableNextColumn(); ui::Label("Pending");
                ui::TableNextRow();
                ui::TableNextColumn(); ui::Label("#1002"); ui::TableNextColumn(); ui::Label("2025-01-02"); ui::TableNextColumn(); ui::TextPositive("+$1200.00"); ui::TableNextColumn(); ui::TextPositive("Completed");
            ui::EndTable();
            if(ui::PrimaryButton("Refresh Data")) AddLog("INFO", "Data Refreshed");
        }
        // --- Settings ---
        else if(ui::IsTabActive("Settings")) {
            ui::BeginTable("SettingsLayout", 2);
            ui::TableNextColumn(); 
            ui::SetNextWidth(200); 
            ui::BeginGroup("Sidebar"); ui::Column(); ui::Label("General"); ui::Label("Graphics"); ui::EndGroup();
            ui::TableNextColumn();
            ui::SetNextGrow(1.0f);
            ui::BeginGroup("SettingsContent"); ui::Column(); ui::SetNextPadding(20);
            ui::Label("Graphics Settings"); ui::Spacer(10);
            if(ui::Checkbox("V-Sync Enabled", &s_vsync)) AddLog("INFO", s_vsync ? "V-Sync ON" : "V-Sync OFF");
            ui::SetTooltip("Toggles Vertical Sync to prevent tearing");
            
            if(ui::Checkbox("Show FPS Counter", &s_show_fps)) AddLog("INFO", "FPS Toggle");
            ui::SetTooltip("Display frames per second overlay");
            
            ui::Spacer(20); ui::Label("User Profile"); ui::TextInput("Username", s_username);
            ui::Spacer(20); if(ui::DangerButton("Reset All Settings")) AddLog("WARN", "Settings Reset");
            ui::EndGroup();
            ui::EndTable();
        }
        // --- Logs ---
        else if(ui::IsTabActive("Logs")) {
            ui::BeginGroup("LogToolbar"); ui::Row();
            if(ui::Button("Clear Logs")) g_logs.clear();
            if(ui::Button("Add Test Log")) AddLog("INFO", "Manual Entry");
            ui::EndGroup();
            ui::Spacer(10);
            ui::EndGroup();
            ui::Spacer(10);
            
            // Scrollable Log Area
            ui::SetNextBg(ui::Color::Hex(0x111111FF)); 
            ui::SetNextPadding(10); 
            ui::BeginChild("LogArea", 0, 0); // Grow=1 implied
            
            for(auto& l : g_logs) {
                ui::BeginGroup("Line"); ui::Row();
                ui::TextDisabled(l.time.c_str()); ui::Spacer(10);
                if(l.type == "INFO") ui::TextPositive("INFO");
                else if(l.type == "WARN") ui::TextNegative("WARN");
                else ui::Label(l.type.c_str());
                ui::Spacer(10); ui::Label(l.msg.c_str());
                ui::EndGroup();
            }
            ui::EndChild(); // LogArea
        }
        
        ui::End(); // Root
        
        ctx.end_frame();
        
        // Render
        DrawList dl; 
        
        build_draw_subtree(ctx.root_id, dl, 0, 0, 0.016f);
        for(auto oid : ctx.overlay_list) build_draw_subtree(oid, dl, 0, 0, 0.016f);
        

        
        // Tooltip Layer
        if (!ctx.tooltip_text.empty()) {
            // Draw at mouse cursor + offset
            float tx = (float)mx + 15;
            float ty = (float)my + 15;
            
            // Background
            // Approximate text size? Font isn't accessible here easily to measure.
            // assume 8px per char + padding
            float tw = ctx.tooltip_text.size() * 9.0f + 16;
            float th = 24.0f;
            
            // Constrain to screen? (Skip for now)
            
            dl.add_rect(tx, ty, tw, th, 0xFFFFDDFF, 4.0f); // Light Yellow
            dl.add_text(font, ctx.tooltip_text, tx+8, ty+4, 0x000000FF);
        }
        
        renderer.render(dl, (float)width, (float)height, atlas.texture_id);
        
        glfwSwapBuffers(window);
    }
}

void FantasmagorieCore::build_draw_subtree(NodeID id, DrawList& dl, float ox, float oy, float dt) {
    auto& c = ctx.node_store.layout[id];
    auto& s = ctx.node_store.style[id];
    auto& r = ctx.node_store.render[id];
    
    float x = c.x;
    float y = c.y;
    float w = c.w;
    float h = c.h;
    
    // Apply Transform (Phase 1)
    // computed_x/y are usually relative to parent content area in recursive layout, or absolute?
    // FlexLayout usually produces absolute screen coordinates if we traverse.
    // If we assume computed_* are screen coordinates relative to the Identity.
    // We need to apply the node's capture transform.
    // Warning: If FlexLayout already adds parent offsets, and we apply parent transform...
    // Only apply transform to the "local" part or if we use SetConstraints(Absolute).
    // The "Graph" example uses Absolute positioning.
    // Let's assume computed_x/y are points we want to transform.
    
    // Actually, Layout Engine (FlexLayout) computes final positions.
    // If we just transform them, we get the visual result.
    // Note: This applies to Backgrounds/Images. Text is inside.
    
    Transform& t = ctx.node_store.transform[id];
    // We transform the Top-Left and Bottom-Right to get new rect?
    // If there is rotation, rect becomes quad. DrawList expects Axis Aligned Rects for some functions.
    // But `DrawList::add_rect` is simpler.
    // Let's assume Affine Transform handles Translation/Scale well. Rotation will distort if we map to AABB.
    
    float x1 = x, y1 = y;
    float x2 = x+w, y2 = y;
    float x3 = x+w, y3 = y+h;
    float x4 = x, y4 = y+h;
    
    t.apply(x1, y1);
    // For simple Scaling/Translation, just transforming x,y and w,h scales is enough.
    // w = w * scale_x?
    // Let's just use transformed x,y and transformed width/height?
    // Using point transformation is safer.
    
    // Re-assign for drawing
    x = x1;
    y = y1;
    
    // For width/height, we need the vector (w,0) transformed?
    // Or just (x2, y2) - (x1, y1)?
    // If strict scale/translate:
    float tx2 = x+w, ty2 = y+h;
    t.apply(tx2, ty2);
    w = tx2 - x1;
    h = ty2 - y1;
    // 0. Shadow (Phase 6)
    if ((s.shadow_color & 0xFF) > 0 && s.shadow_radius > 0.0f) {
        if(s.bg_color != 0) {
            dl.add_shadow_rect(x + s.shadow_offset_x, y + s.shadow_offset_y, w, h, 
                               s.corner_radius, 
                               s.shadow_color, 
                               s.shadow_radius);
        }
    }

    // 1. Background
    if (s.bg_color != 0) {
        dl.add_rect(x,y,w,h, s.bg_color, s.corner_radius);
    }
    
    // 2. Image
    if (r.is_image) {
        dl.add_image((uint32_t)(uintptr_t)r.custom_texture_id, x, y, w, h);
    }
    
    // 3. Text
    if (r.is_text) {
        dl.add_text(font, r.text_content, x + s.border_width + 5, y + s.border_width + 5, s.text_color);
    }
    
    // 4. Custom Draw Commands (Phase 1)
    for(const auto& cmd : ctx.node_store.render[id].custom_draw_cmds) {
        if (cmd.type == DrawType::Line) { // Line
            dl.add_line(cmd.x, cmd.y, cmd.w, cmd.h, cmd.color, cmd.p1);
        } else if (cmd.type == DrawType::Bezier) {
             dl.add_bezier(cmd.x, cmd.y, cmd.w, cmd.h, cmd.p1, cmd.p2, cmd.p3, cmd.p4, cmd.color, 1.0f); // Thickness?
        } else if (cmd.type == DrawType::Rect) {
            // Need add_rect_custom? Or generic push?
            // DL usually has explicit methods.
            // For now, custom commands are just pushed directly if they match primitive types?
            // Actually, `DrawList` is being refactored in Phase 3 too?
            // "Separate DrawList primitives" was a task.
            // But `dl` has `add_line`, `add_bezier`. These methods create `DrawCmd` internally.
            // We should use those.
        } else if (cmd.type == DrawType::Rect) {
            dl.add_rect(cmd.x, cmd.y, cmd.w, cmd.h, cmd.color, cmd.p1);
        } else if (cmd.type == DrawType::Text) {
            bool bold = (cmd.p1 > 0.5f);
            bool italic = (cmd.p2 > 0.5f);
            if(cmd.text_ptr)
                dl.add_text(font, cmd.text_ptr, cmd.x, cmd.y, cmd.color, bold, italic);
        } else if (cmd.type == DrawType::Arc) {
            dl.add_arc(cmd.x, cmd.y, cmd.w, cmd.p1, cmd.p2, cmd.color, 2.0f);
        }
        // Add more as needed
    }
    
    // Image Support
    if (r.texture_id != nullptr) {
         dl.add_texture_rect(x, y, w, h, 
            node.render().texture_id,
            node.render().u0, node.render().v0,
            node.render().u1, node.render().v1,
            ui::Color::Hex(0xFFFFFFFF));
    }

    // 5. Focus Ring (Debug/Visual)
    if (s.show_focus_ring) {
        dl.add_focus_ring(x,y,w,h, 0x00AAFFFF, 2.0f);
    }
    
    // Clipping
    bool clip = ctx.node_store.scroll[id].clip_content;
    if (clip) {
        // Adjust clip rect for border? For now full rect
        dl.push_clip_rect(x, y, w, h);
    }
    
    for (auto child : ctx.node_store.tree[id].children) {
        build_draw_subtree(child, dl, x, y, dt);
    }
    
    if (clip) {
        dl.pop_clip_rect();
    }
}

void FantasmagorieCore::shutdown() {
    core::JobSystem::Shutdown();
    glfwTerminate();
}
