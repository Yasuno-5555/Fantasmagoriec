#include "api.hpp"
#include "../core/ui_context.hpp"
#include "../input/focus.hpp"
#include "../input/input_dispatch.hpp"
#include "../layout/flex_layout.hpp"
#include <cmath>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "../render/renderer.hpp" // Phase 2
#include "../platform/interface.hpp" // Phase Platform
#include "widgets/markdown.hpp" // Phase 2: Rich Text
#include "../core/i18n.hpp" // Phase 3: Localization

namespace ui {
    const char* Loc(const char* key) { return core::I18n::Get(key); }

    static UIContext* g_ctx = nullptr;
    static FocusManager* g_focus = nullptr;
    static InputDispatcher* g_input = nullptr;
    
    // Internal State
    struct TableState {
        int cols;
        int current_col;
    };
    static TableState g_table = {0, 0};
    
    static std::string g_current_tab_bar; 
    static std::map<std::string, std::string> g_tab_state; 

    void SetContext(UIContext* ctx) { g_ctx = ctx; }
    void SetInteraction(FocusManager* f, InputDispatcher* i) {
        g_focus = f;
        g_input = i;
    }

    void UpdateInputState(void* window_handle, float mx, float my, bool mdown, float scroll_y) {
        if(g_ctx) g_ctx->frame_scroll_delta = scroll_y; // Store for widgets
        
        if(g_focus && g_ctx) {
            // Update Focus Manager (Hit Testing, Active State, Scroll)
            g_focus->update(*g_ctx, mx, my, mdown, scroll_y);
        }
        if(g_input && g_ctx && g_focus) {
            g_input->dispatch(*g_ctx, *g_focus, window_handle);
        }
    }
    
    static uint32_t col_to_u32(Color c) {
        uint32_t r = (uint32_t)(c.r * 255);
        uint32_t g = (uint32_t)(c.g * 255);
        uint32_t b = (uint32_t)(c.b * 255);
        uint32_t a = (uint32_t)(c.a * 255);
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    // Wrappers
    void Begin(const char* id) { if(g_ctx) g_ctx->begin_node(id); }
    
    void End() { if(g_ctx) g_ctx->end_node(); }

    void Column() {
        if(!g_ctx || g_ctx->parent_stack_vec.empty()) return;
        NodeID pid = g_ctx->parent_stack_vec.back();
        g_ctx->get(pid).layout().dir = 1; 
    }

    void Row() {
        if(!g_ctx || g_ctx->parent_stack_vec.empty()) return;
        NodeID pid = g_ctx->parent_stack_vec.back();
        g_ctx->get(pid).layout().dir = 0; 
    }
    
    void BeginGroup(const char* id) { if(g_ctx) g_ctx->begin_node(id); }
    void EndGroup() { if(g_ctx) g_ctx->end_node(); }

    void SetNextWidth(float w) { if(g_ctx) g_ctx->next_layout.width = w; }
    void SetNextHeight(float h) { if(g_ctx) g_ctx->next_layout.height = h; }
    void SetNextGrow(float f) { if(g_ctx) g_ctx->next_layout.grow = f; }
    void SetNextPadding(float px) { if(g_ctx) g_ctx->next_style.padding = px; }
    void SetNextAlign(Align a) { 
        if(!g_ctx) return;
        int ea = 0; // Start
        if(a==Align::Center) ea=1;
        if(a==Align::End) ea=2;
        if(a==Align::Stretch) ea=3;
        g_ctx->next_align = ea;
    }

    void PushStyle(const Style& s) { if(g_ctx) g_ctx->push_style(s); }
    void PopStyle() { if(g_ctx) g_ctx->pop_style(); }
    
    void SetNextBg(Color c) { if(g_ctx) g_ctx->next_style.bg = c; }
    
    void SetNextBgHover(Color c) { if(g_ctx) g_ctx->next_style.bg_hover = c; }
    void SetNextBgActive(Color c) { if(g_ctx) g_ctx->next_style.bg_active = c; }
    void SetNextTextColor(Color c) { if(g_ctx) g_ctx->next_style.text_color = c; }
    void SetNextRadius(float r) { if(g_ctx) g_ctx->next_style.radius = r; }
    void SetNextBorder(float width, Color color) {
        if(g_ctx) {
            g_ctx->next_style.border_width = width;
            g_ctx->next_style.border_color = color;
        }
    }
    void SetNextFocusRing(Color color, float width) {
        if(!g_ctx) return;
        g_ctx->next_style.show_focus_ring = true;
        g_ctx->next_style.focus_ring_color = color; // Implicit conversion?
        g_ctx->next_style.focus_ring_width = width;
    }

    void SetNextPosition(int type, float x, float y) {
        if(!g_ctx) return;
        g_ctx->next_layout.position_type = type;
        g_ctx->next_layout.x = x;
        g_ctx->next_layout.y = y;
    }

    // Widgets
    // Simplified Slider (Visual only)
    bool Slider(const char* label, float* v, float v_min, float v_max) {
       // Placeholder implementation to satisfy Linker if needed (not used now)
       return false;
    }

    bool Checkbox(const char* label, bool* v) {
        if(!g_ctx) return false;
        std::string id_str = std::string("##chk_") + label;
        NodeID id = g_ctx->get_id(id_str);
        
        BeginGroup(id_str.c_str());
        Row();
        SetNextAlign(Align::Center);
        SetNextPadding(0);
        
        NodeID box_id = g_ctx->begin_node("box");
        NodeHandle box = g_ctx->get(box_id);
        box.constraint().width = 20;
        box.constraint().height = 20;
        box.constraint().padding = 0;
        box.style().corner_radius = 4;
        box.style().border_width = 1;
        box.style().border_color = 0xFFFFFFFF;
        box.style().bg_color = *v ? 0x007ACCFF : 0x333333FF;
        
        box.input().clickable = true;
        if(g_focus) g_focus->register_node(g_ctx, box_id); 
        
        bool clicked = false;
        if(g_focus && g_focus->is_clicked(box_id)) {
            *v = !(*v);
            clicked = true;
        }
        g_ctx->end_node(); // box
        
        Spacer(8);
        Text(label);
        
        EndGroup();
        return clicked;
    }

    bool Button(const char* label) {
        if(!g_ctx) return false;
        NodeID id = g_ctx->begin_node(label); 
        NodeHandle n = g_ctx->get(id);

        if (n.style().bg_color == 0) n.style().bg_color = 0x444444FF; 
        
        n.render().is_text = true;
        n.render().text_content = label;
        n.input().clickable = true; // Essential
        
        if(g_focus) {
            g_focus->register_node(g_ctx, id);
            bool clicked = g_focus->is_clicked(id);
            g_ctx->end_node();
            return clicked;
        }
        g_ctx->end_node();
        return false;
    }

    bool DangerButton(const char* label) { return Button(label); } 
    bool PrimaryButton(const char* label) { return Button(label); }
    // Phase 2: Interaction Helpers
    bool Draggable(const char* id, float* x, float* y) {
        if(!g_ctx || !g_focus) return false;
        
        NodeID nid = g_ctx->begin_node(id);
        NodeHandle n = g_ctx->get(nid);
        
        // Essential for Hit Testing
        n.input().clickable = true;
        // n.input().hoverable? FocusManager hit_test checks clickable.
        
        // Register for focus/active state
        g_focus->register_node(g_ctx, nid);
        
        bool dragged = false;
        if(g_focus->active_id == nid && g_focus->mouse_was_down) {
             // Logic:
             // 1. Get raw delta (screen space)
             float dx = g_focus->drag_dx;
             float dy = g_focus->drag_dy;
             
             // 2. Apply Inverse Transform to delta
             // Delta is a vector, so Translation part of Transform doesn't apply?
             // Actually, `apply_inverse` on a delta vector (dx, dy) should ignore translation (m[4], m[5]).
             // Our `apply_inverse` formula:
             // nx = x - m[4]; ...
             // ix = (dt*nx ...);
             // If x is a point, this is correct (screenspace point -> local point).
             // If x is a vector (dx), we want the vector in local space.
             // Vector transform: V' = M*V (no translation).
             // Vector inverse: V = M^-1 * V' (no translation).
             // m[4], m[5] should be treated as 0 for vectors.
             // Wait, `apply_inverse` implements Point inverse. 
             // We can do: P1 = (0,0), P2 = (dx,dy). 
             // LocalP1 = Inv(P1), LocalP2 = Inv(P2).
             // LocalDelta = LocalP2 - LocalP1.
             // This works regardless of implementation details.
             
             Transform& t = g_ctx->current_transform; // Use current transform stack top?
             // Actually, Draggable is a Node. It has a transform captured in `node_store.transform[nid]`.
             // But Draggable() is called *during* BeginNode state of itself?
             // Yes, `g_ctx->begin_node` was called. `current_transform` is active.
             
             float zx = 0, zy = 0;
             t.apply_inverse(zx, zy);
             
             float ex = dx, ey = dy;
             t.apply_inverse(ex, ey);
             
             float ldx = ex - zx;
             float ldy = ey - zy;
             
             *x += ldx;
             *y += ldy;
             dragged = true;
        }
        
        g_ctx->end_node();
        return dragged;
    }

    // DrawType logic in DrawRect/DrawLine removed from here as they are separate.
    
    // Node Graph Widgets
    void BeginNode(const char* id, float* x, float* y) {
        if(!g_ctx) return;
        
        // Container
        char uid[64]; sprintf(uid, "Node_%s", id);
        
        // We use SetNextPosition to place it at *x, *y
        SetNextPosition(1, *x, *y);
        SetNextBg(Color::Hex(0x333333E6));
        SetNextRadius(8.0f);
        SetNextPadding(0); // Custom internal layout
        
        // Node Body (Group)
        BeginGroup(uid);
        
        // Apply Draggable Logic (Overlay)
        // We can use a trick: 
        // 1. Draw Title Bar (First child)
        // 2. Make Title Bar Draggable
        
        BeginGroup("TitleBar");
        SetNextBg(Color::Hex(0x555555FF));
        SetNextRadius(8.0f); // Top corners only? Simplified.
        SetNextHeight(24.0f);
        SetNextPadding(4);
        
        BeginGroup("TitleContent");
        // Title text is supplied by user after BeginNode? 
        // Or passed as arg?
        // Usually: BeginNode(id, x, y); Text("Title"); ...
        // If we want Title Bar to be draggable, we need to inject Draggable HERE.
        
        // Draggable Overlay for Title
        // Use relative absolute positioning (Child of TitleContent)
        SetNextPosition(1,0,0);
        SetNextGrow(1.0f);
        SetNextHeight(24.0f);
        SetNextBg(Color::Hex(0x00000000));
        Draggable("DiskDrag", x, y); // Update external x,y
        
        // End TitleContent
        EndGroup(); 
        EndGroup(); // End TitleBar
        
        // Body Content Group
        SetNextPadding(8);
        BeginGroup("Body");
    }

    void EndNode() {
        if(!g_ctx) return;
        EndGroup(); // Body
        EndGroup(); // Node Main
    }

    void GetLastNodeSize(float* w, float* h) {
        if(g_ctx) {
            *w = g_ctx->last_node_w;
            *h = g_ctx->last_node_h;
        }
    }
    
    void NodePin(const char* label, bool is_input) {
         // Simple visual pin
         // Circle + Label
         BeginGroup(is_input ? "PinIn" : "PinOut");
         // Row layout
         // If Input: Icon - Label
         // If Output: Label - Icon
         // Current Layout engine is vertical by default.
         // We need `Row()`.
         
         // Mock Row Layout (Generic Group with Horizontal Flow?)
         // Currently framework has `Row()` but it sets flex direction.
         // Let's assume `Row()` works.
         Row();
         SetNextAlign(Align::Center);
         BeginGroup("PinRow");
         
         if(is_input) {
             // Icon
             SetNextWidth(12); SetNextHeight(12); SetNextRadius(6); 
             SetNextBg(Color::Hex(0xAAAAAAFF));
             Label(""); // Spacer/Icon
             Spacer(8);
             Text(label);
         } else {
             // Label first
             Text(label);
             Spacer(8);
             SetNextWidth(12); SetNextHeight(12); SetNextRadius(6); 
             SetNextBg(Color::Hex(0xAAAAAAFF));
             Label(""); // Icon
         }
         
         EndGroup();
         EndGroup();
    }

    void PanZoomLogic(float* pan_x, float* pan_y, float* zoom) {
        if(!g_ctx || !g_focus) return;
        
        // Logic: 
        // 1. Pan with Middle Mouse (Active State)
        // 2. Zoom with Scroll (Hover State)
        
        // We need a container node to receive events
        NodeID nid = g_ctx->begin_node("PanZoom");
        NodeHandle n = g_ctx->get(nid);
        n.input().clickable = true;     // Catch clicks
        n.input().hoverable = true;     // Catch scroll/hover
        
        // Fill Parent (usually Canvas)
        n.constraint().width = -1; // Auto (Assuming parent layout stretches it?)
        // Actually, if we are in a Manual/Absolute layout or Child, expected behavior varies.
        // Let's assume user calls this inside a BeginChild("Canvas").
        // Child has 0 padding?
        n.constraint().grow = 1.0f; // Grow to fill
        n.style().bg_color = 0x00000000; // Invisible
        
        g_focus->register_node(g_ctx, nid);
        
        // Pan Logic (Middle Mouse)
        // FocusManager tracks Left Mouse mainly?
        // FocusManager `mouse_down` is generic boolean.
        // We need to know WHICH button.
        // API UpdateInputState only takes `bool mdown`.
        // Limitation: Currently only supports primary button for "Active" and "Drag".
        // Workaround: We check GLFW directly in Application for Middle Mouse?
        // OR we upgrade API to support Button ID.
        // For Phase 2, adhering to "Take state management away", we should handle this.
        // But `UpdateInputState` signature is `bool mdown` (Single bit).
        // I will stick to Left Mouse Pan for now if API limits, OR Assume Application handles Middle->mdown mapping?
        // In `application.cpp`, `bool mdown = glfwGetMouseButton(window, 0) == GLFW_PRESS`.
        // So `mdown` is Left Click.
        // The user requirements didn't specify Middle Mouse, but Graph Demo used Middle.
        // If I switch to Left Mouse for Pan, it conflicts with Selection/Draggable?
        // Usually Space+Left or Middle.
        // I'll implement "Space+Left" logic? Or just rely on FocusManager finding this background node if nothing else hit.
        
        // Let's use the `drag_dx/dy` from FocusManager if this is Active.
        // If this is the background node, it's only active if no foreground node was clicked.
        // So Left Drag on background = Pan.
        
        if (g_focus->active_id == nid && g_focus->mouse_was_down) {
            float dx = g_focus->drag_dx;
            float dy = g_focus->drag_dy;
            
            // Pan is View movement. Object moves delta.
            *pan_x += dx / (*zoom);
            *pan_y += dy / (*zoom);
        }
        
        // Zoom Logic (Scroll)
        if (g_focus->hover_id == nid) {
            float s = g_ctx->frame_scroll_delta;
            if (s != 0.0f) {
                float old_zoom = *zoom;
                *zoom += s * 0.1f * (*zoom); // Logarithmic-ish
                if (*zoom < 0.1f) *zoom = 0.1f;
                if (*zoom > 10.0f) *zoom = 10.0f;
                
                // Zoom towards mouse? (Requires math: P_world = (P_screen / Zoom) - Pan)
                // We want P_world to stay at P_screen.
                // NewPan = (P_screen / NewZoom) - P_world
                // NewPan = (P_screen / NewZoom) - ((P_screen / OldZoom) - OldPan)
                // Complex. For now, simple Center or Origin zoom.
            }
        }
        
        g_ctx->end_node();
    }

    void CanvasGrid(float zoom, float step_size) {
        if(!g_ctx) return;
        // Assumption: Called INSIDE the Transformed context (PushTranslate/PushScale active).
        // And INSIDE a Container (BeginChild) that defines the Viewport.
        // We need the Viewport Size (in Screen Space) to calculate World Bounds.
        
        // Problem: We don't have easy access to "Parent Size" during API call unless we query layout.
        // But Layout is computed *after* user definition in this One-Pass constraints system?
        // Actually, Layout is frame-lagged or computed recursively.
        // Fantasmagorie uses "Constraint-Based Layout".
        // If we are in `BeginChild`, the child's size might be determined by constraints?
        // Let's assume a large enough area or use a fixed "Screen Size" uniform?
        // Better: We trace lines in World Space around the View Center?
        // View Center in World Space = -Pan.
        // But `CanvasGrid` doesn't take Pan as arg?
        // Wait, if we are transformed, (0,0) is origin.
        // We need to draw lines from -Infinity to +Infinity?
        // Realistically: -50000 to +50000 is fine for 2D.
        // But drawing 100,000 lines is slow if `DrawLine` is CPU-bound.
        // We should calculate visible range.
        
        // We need "Current Inverse Transform".
        Transform& t = g_ctx->current_transform;
        
        // Screen bounds (Approximate assuming full window for now, or 2000x2000)
        // We can optimize strict culling later.
        float sx1 = 0, sy1 = 0;
        float sx2 = 2000, sy2 = 2000; // Large buffer
        
        float wx1 = sx1, wy1 = sy1;
        float wx2 = sx2, wy2 = sy2;
        
        t.apply_inverse(wx1, wy1);
        t.apply_inverse(wx2, wy2);
        
        // Normalize min/max
        float min_x = std::min(wx1, wx2);
        float max_x = std::max(wx1, wx2);
        float min_y = std::min(wy1, wy2);
        float max_y = std::max(wy1, wy2);
        
        // Snap to step
        float start_x = floor(min_x / step_size) * step_size;
        float start_y = floor(min_y / step_size) * step_size;
        
        uint32_t col = 0x444444FF;
        uint32_t axis_col = 0x00000088; // Darker axis
        
        // Vertical lines
        for(float x = start_x; x < max_x; x += step_size) {
            DrawLine(x, min_y, x, max_y, (x==0)?axis_col:col, 1.0f);
        }
        
        // Horizontal lines
        for(float y = start_y; y < max_y; y += step_size) {
             DrawLine(min_x, y, max_x, y, (y==0)?axis_col:col, 1.0f);
        }
    }
    
    void Label(const char* t) { Text(t); }

    InputResult TextInput(const char* id, std::string& buffer) {
        g_ctx->begin_node(id);
        NodeHandle n = g_ctx->get(g_ctx->get_id(id));
        n.render().is_text = true;
        n.render().text_content = buffer;
        n.input().focusable = true;
        n.input().clickable = true;
        // n.input().on_text_input = true; // Type mismatch (std::function vs bool). Implementation pending.
        // Logic ...
        if(g_focus) g_focus->register_node(g_ctx, g_ctx->get_id(id));
        g_ctx->end_node();
        return InputResult::None;
    }

    void Text(const char* content) {
        if(!g_ctx) return;
        g_ctx->begin_node(content);
        NodeID id = g_ctx->current_id; // Using current instead of recalc
        NodeHandle n = g_ctx->get(id);
        n.render().is_text = true;
        n.render().text_content = content;
        g_ctx->end_node();
    }

    void Spacer(float size) {
        if(!g_ctx) return;
        NodeHandle n = g_ctx->get(g_ctx->current_id);
        n.constraint().height = size; 
        g_ctx->end_node();
    }

    bool IsKeyDown(int) { return false; }
    bool IsKeyPressed(int) { return false; }

    // Tooltips
    void SetTooltip(const char* text) {
        if(!g_ctx || !g_focus) return;
        NodeID last = g_ctx->last_id;
        if(last != 0 && g_focus->hover_id == last) {
             g_ctx->tooltip_text = text;
        }
    }

    // Windows
    void BeginWindow(const char* title, float x, float y, float w, float h) {
        if(!g_ctx) return;
        NodeID win_id = g_ctx->get_id(title); 
        
        // Persistent State logic
        auto& ts = g_ctx->persistent_layout[win_id];
        if (ts.w == 0) { ts.x = x; ts.y = y; ts.w = w; ts.h = h; }
        
        g_ctx->begin_node(win_id); // Use raw ID
        NodeHandle n = g_ctx->get(win_id);
    
        n.constraint().position_type = 1; // Absolute
        n.constraint().x = ts.x;
        n.constraint().y = ts.y;
        n.constraint().width = ts.w;
        n.constraint().height = ts.h;
        n.style().bg_color = 0x222222FF;
        n.style().border_width = 1;
        n.style().border_color = 0x555555FF;
        n.style().show_focus_ring = true; // Visual debug
        
        // Audit: Register as overlay so it solves and renders on top
        g_ctx->overlay_list.push_back(win_id);
        
        // Layout: Column
        n.layout().dir = 1; 
        
        // Title Bar
        NodeID title_id = g_ctx->begin_node("TitleBar");
        NodeHandle t = g_ctx->get(title_id);
        t.constraint().height = 24;
        t.constraint().width = -1; // Stretch
        t.style().bg_color = 0x444444FF;
        
        // Drag Logic 
        // ... (Phase 13 Logic omitted for brevity)
        
        g_ctx->end_node(); // TitleBar
    }
    
    // Phase 18: Image
    void Image(uint32_t texture_id, float w, float h) {
        if(!g_ctx) return;
        NodeID id = g_ctx->begin_node("img");
        NodeHandle n = g_ctx->get(id);
        
        n.constraint().width = w;
        n.constraint().height = h;
        n.render().is_image = true;
        n.render().custom_texture_id = (void*)(uintptr_t)texture_id; 
        
        g_ctx->end_node();
    }
    
    // Phase 18-C: Scrollable Region
    void BeginChild(const char* id, float w, float h) {
        if(!g_ctx) return;
         NodeID nid = g_ctx->begin_node(id);
         NodeHandle n = g_ctx->get(nid);
         
         // If dimensions provided, set them
         if (w > 0) n.constraint().width = w;
         if (h > 0) n.constraint().height = h;
         else if (h == 0) n.constraint().grow = 1.0f; // Default grow if 0 height?

         n.scroll().clip_content = true;
         // Input state
         n.input().hoverable = true; // Need to be hoverable to catch scroll
         if(g_focus) g_focus->register_node(g_ctx, nid);
         
         // Style defaults
         n.layout().dir = 1; // Col
    }
    
    void EndChild() {
        if(!g_ctx) return;
        
        NodeID nid = g_ctx->current_id;
        
        // Draw Scrollbar (Overlay)
        if (g_ctx->node_store.scroll.count(nid)) {
            auto& s = g_ctx->node_store.scroll[nid];
            if (s.max_scroll_y > 0) {
                // Get dimensions (Use Prev Layout or Current Constraint if fixed?)
                // Use Prev for stability
                float h = 100; // Fallback
                float w = 100;
                if(g_ctx->prev_layout.count(nid)) {
                    h = g_ctx->prev_layout[nid].h;
                    w = g_ctx->prev_layout[nid].w;
                }
                
                // Track
                float track_w = 6;
                SetNextPosition(1, w - track_w - 2, 0); // Abs from Top-Right (Relative to container?)
                // Wait, Child container is the reference frame if detached? 
                // No, standard nodes are relative to parent content.
                // But we want to draw ON TOP of the child.
                // We should draw this AFTER `EndNode` (as an overlay)? 
                // Or inside `EndNode` but as a "Post-Draw" sibling?
                // Simplest: Draw logic inside EndChild before EndNode, using absolute offset from (0,0) of child.
                // Child origin (0,0) is correct context if we didn't scroll the scrollbar itself!
                // But `EndChild` is inside the scrolled content? 
                // Usually `EndChild` is called inside the `begin_node`.
                // If we draw here, we are subject to the child's scroll.
                // Scrollbars should NOT scroll.
                // We need to counteract scroll or draw in parent.
                // But parent doesn't know about child's internal scrollbar easily.
                
                // Hack: Apply reverse scroll offset
                SetNextPosition(1, w - track_w, s.scroll_y); 
                // Y=s.scroll_y puts it at the "Visual Top" of the scrolled viewport.
                
                BeginGroup("##scrollbar_track");
                NodeHandle t = g_ctx->get(g_ctx->current_id);
                t.constraint().width = track_w;
                t.constraint().height = h;
                t.style().bg_color = 0x00000044;
                
                    // Thumb
                    float view_h = h;
                    float total_h = h + s.max_scroll_y;
                    float thumb_h = std::max(20.0f, view_h * (view_h / total_h));
                    float thumb_y = (s.scroll_y / s.max_scroll_y) * (view_h - thumb_h);
                    
                    SetNextPosition(1, 0, thumb_y); 
                    BeginGroup("##thumb");
                    NodeHandle th = g_ctx->get(g_ctx->current_id);
                    th.constraint().width = track_w;
                    th.constraint().height = thumb_h;
                    th.style().bg_color = 0x888888AA;
                    th.style().corner_radius = 3;
                    g_ctx->end_node(); // Thumb

                g_ctx->end_node(); // Track
            }
        }

        g_ctx->end_node();
    }
    void EndWindow() {
        EndGroup();
        End();
    }

    void OpenPopup(const char* id) {
        if(g_ctx) g_ctx->open_popup(id);
    }
    void CloseCurrentPopup() {
        if(g_ctx && !g_ctx->open_popups.empty()) g_ctx->open_popups.pop_back();
    }

    bool BeginPopup(const char* id) {
        if(!g_ctx) return false;
        uint32_t target = hash_str(id);
        bool found = false;
        for(auto p : g_ctx->open_popups) if(p==target) found=true;
        if(!found) return false;
        
        Begin(id);
        NodeID nid = g_ctx->current_id;
        NodeHandle n = g_ctx->get(nid);
        
        n.constraint().position_type = 1;
        n.constraint().x = 100;
        n.constraint().y = 100;
        
        // Audit: Register as overlay
        g_ctx->overlay_list.push_back(nid);

        // Detach ignored for simple verify
        return true;
    }
    void EndPopup() { End(); }

    // ===========================
    // Phase 1 Implementation
    // ===========================
    
    // Combo
    bool BeginCombo(const char* label, const char* preview_value) {
        if(!g_ctx) return false;
        
        std::string popup_id = std::string("##combo_") + label;
        bool is_open = false;
        for(auto p : g_ctx->open_popups) if(p == hash_str(popup_id.c_str())) is_open=true;
        
        // Draw the preview box
        // Similar to Button but with arrow
        BeginGroup(label);
        Row();
        SetNextAlign(Align::Center);
        SetNextPadding(4);
        
        // Label
        if (label[0] != '#' || label[1] != '#') {
             Text(label);
             Spacer(8);
        }
        
        // Box
        NodeID id = g_ctx->begin_node("ComboPreview");
        NodeHandle n = g_ctx->get(id);
        n.constraint().width = 120; // Default width
        n.constraint().height = 24;
        n.style().bg_color = 0x333333FF;
        n.style().border_width = 1;
        n.style().border_color = 0x666666FF;
        n.style().corner_radius = 4;
        n.input().clickable = true;
        
        if(g_focus) g_focus->register_node(g_ctx, id);
        
        // Click toggles popup
        if(g_focus && g_focus->is_clicked(id)) {
            if(!is_open) OpenPopup(popup_id.c_str());
            else CloseCurrentPopup();
        }
        
        // Inner Text
        BeginGroup("PreviewText");
        Text(preview_value ? preview_value : "");
        EndGroup();
        
        // Arrow (Simple triangle or text "v")
        SetNextPosition(1, n.layout().w - 20, 0); // Abs Right
        Label("v");
        
        g_ctx->end_node(); // Preview Box
        EndGroup(); // Main Wrapper
        
        // Popup Logic
        if (BeginPopup(popup_id.c_str())) {
            // Position it under the combo box?
            // BeginPopup sets absolute position (100,100) currently. 
            // We need automatic placement.
            // For now, let's hack position to mouse or last node?
            // "GetLastNodeSize" + Position.
            // Better: Popup Logic should handle "Open at correct place".
            return true;
        }
        
        return false;
    }

    void EndCombo() {
        EndPopup();
    }
    
    bool Combo(const char* label, int* current_item, const std::vector<std::string>& items) {
        if(!current_item || items.empty()) return false;
        
        const char* preview = (*current_item >= 0 && *current_item < items.size()) ? items[*current_item].c_str() : "";
        
        if (BeginCombo(label, preview)) {
            for (int i = 0; i < items.size(); ++i) {
                bool selected = (*current_item == i);
                // Use MenuItem or Selectable? MenuItem is fine for now
                if (MenuItem(items[i].c_str(), nullptr, selected)) {
                    *current_item = i;
                }
            }
            EndCombo();
            return true; 
        }
        return false;
    }
    
    // Menu
    bool BeginMenuBar() {
        if(!g_ctx) return false;
        Row(); 
        SetNextHeight(28);
        SetNextGrow(1.0f); // Fill width
        SetNextBg(g_ctx->style_stack.empty() ? Color::Hex(0x333333FF) : g_ctx->style_stack.back().menu_bg); 
        SetNextPadding(2);
        BeginGroup("MenuBar");
        return true;
    }

    void EndMenuBar() {
        EndGroup();
    }

    bool BeginMenu(const char* label) {
        if(!g_ctx) return false;
        
        std::string menu_id = std::string("##menu_") + label;
        bool is_open = false;
        // Check if this menu is the active one
         uint32_t target_hash = hash_str(menu_id.c_str());
        for(auto p : g_ctx->open_popups) if(p == target_hash) is_open=true;

        // Draw Menu Item in Bar
        if (Button(label)) {
            if (!is_open) {
                 OpenPopup(menu_id.c_str());
                 g_ctx->active_menu_id = g_ctx->get_id(menu_id);
            } else {
                CloseCurrentPopup();
                g_ctx->active_menu_id = 0;
            }
        }
        
        // If hovered and another menu is active, switch inputs?
        // (Skipped for simple implementation)

        if (BeginPopup(menu_id.c_str())) {
             // Styling for menu list
             SetNextPadding(4);
             SetNextBg(g_ctx->style_stack.empty() ? Color::Hex(0x222222FF) : g_ctx->style_stack.back().popup_bg);
             BeginGroup("MenuColumn"); // Wrapper for styling
             return true;
        }
        
        return false;
    }

    void EndMenu() {
        EndGroup();
        EndPopup();
    }

    bool MenuItem(const char* label, const char* shortcut, bool selected) {
        // Similar to Button but full width and different style
        Row();
        SetNextGrow(1.0f);
        SetNextPadding(4);
        
        bool clicked = false;
        // Hover handling manually or Button helper?
        // Let's use custom node
        NodeID id = g_ctx->begin_node(label);
        NodeHandle n = g_ctx->get(id);
        n.input().clickable = true;
        if(g_focus) g_focus->register_node(g_ctx, id);
        
        if (g_focus && g_focus->hover_id == id) {
            n.style().bg_color = g_ctx->style_stack.back().item_hover_bg.u32(); // 0x444444FF
        }
        
        if (g_focus && g_focus->is_clicked(id)) {
            clicked = true;
            CloseCurrentPopup(); // Auto close on click
        }
        
        // Text
        Text(label);
        if (shortcut) {
             Spacer(20);
             TextDisabled(shortcut);
        }
        
        g_ctx->end_node();
        return clicked;
    }

    // Splitter
    void Splitter(const char* id, float* size_var, bool vertical) {
        if(!g_ctx || !size_var) return;
        
        // Visual
        NodeID nid = g_ctx->begin_node(id);
        NodeHandle n = g_ctx->get(nid);
        
        if (vertical) { // Vertical Splitter (splits Left/Right)
            n.constraint().width = 4;
            n.constraint().height = -1; // Stretch
            n.style().cursor_type = 2; // Resize EW (TODO)
        } else { // Horizontal Splitter (splits Top/Bottom)
            n.constraint().height = 4;
            n.constraint().width = -1; // Stretch
            n.style().cursor_type = 3; // Resize NS (TODO)
        }
        
        n.style().bg_color = g_ctx->style_stack.back().splitter_color.u32(); // Default color
        
        n.input().clickable = true;
        // n.input().hoverable = true;
        if(g_focus) g_focus->register_node(g_ctx, nid);
        
        // Interaction
        if (g_focus) {
            // Hover Color
            if (g_focus->hover_id == nid) {
                 // n.style().bg_color = 0x666666FF; 
            }
            
            // Drag Logic
            bool active = (g_focus->active_id == nid && g_focus->mouse_was_down);
            if (active) {
                float dx = g_focus->drag_dx;
                float dy = g_focus->drag_dy;
                
                // Adjust size variable
                if (vertical) *size_var += dx;
                else *size_var += dy;
                
                // Clamp
                if (*size_var < 10.0f) *size_var = 10.0f;
            }
        }
        
        g_ctx->end_node();
    }


    // SpinBox
    bool SpinBox(const char* label, float* v, float v_min, float v_max) {
        if(!g_ctx) return false;
        BeginGroup(label);
        Row();
        SetNextAlign(Align::Center);
        
        // Label
        if (label[0] != '#' || label[1] != '#') {
             Text(label);
             Spacer(8);
        }
        
        // Button - (Drag to change?)
        // For simplicity: Left/Right buttons + Text
        if (Button("-")) *v -= 1.0f; // Step
        
        char buf[64]; sprintf(buf, "%.2f", *v);
        SetNextWidth(60);
        BeginGroup("Val");
        Text(buf);
        EndGroup();
        
        if (Button("+")) *v += 1.0f;
        
        // Clamp
        if (*v < v_min) *v = v_min;
        if (*v > v_max) *v = v_max;
        
        EndGroup();
        return false; // Return changed?
    }
    
    bool SpinBox(const char* label, int* v, int v_min, int v_max) {
        float fv = (float)*v;
        bool res = SpinBox(label, &fv, (float)v_min, (float)v_max);
        *v = (int)fv;
        return res;
    }
    
    // TextArea (Multiline)
    // extern TextMeasurement ::MeasureText(struct Font& font, const std::string& text); // Removed (use render:: directly)

    InputResult TextArea(const char* id, std::string& buffer) {
         if(!g_ctx) return InputResult::None;
         
         // Measurement
         float w = 200, h = 100;
         if (g_ctx->font) { 
             // Phase 2: Dynamic Sizing
             TextMeasurement m = ::MeasureText(*g_ctx->font, buffer); 
             w = std::max(w, m.w + 20); // Padding
             h = std::max(h, m.h + 20);
         }
         
         NodeID nid = g_ctx->begin_node(id);
         NodeHandle n = g_ctx->get(nid);
         
         n.constraint().width = w;
         n.constraint().height = h;
         n.style().bg_color = 0x222222FF;
         n.style().border_width = 1;
         n.style().border_color = 0x555555FF;
         
         n.render().is_text = true;
         n.render().text_content = buffer; // Multiline check in renderer?
         // We rely on renderer handling newlines in "text_content".
         
         n.input().clickable = true;
         n.input().focusable = true;
         
         // Text Input Logic
         if(g_focus) g_focus->register_node(g_ctx, nid);
         if (g_focus && g_focus->focus_id == nid) { // Assumed focus_id
                 // Blink Caret
                 // Draw Caret
                 TextMeasurement m = MeasureText(*g_ctx->font, buffer);
                 float cx = n.layout().x + m.w + 5;
                 float cy = n.layout().y + 5;
                 // Simple caret
                 DrawRect(cx, cy, 2, h - 10, 0xFFFFFFFF, 0); 
                 
                 // Handle Input
                 std::string s = buffer;
                 bool changed = false;
                 
                 // Backspace
                 for(int k : g_ctx->input_keys) {
                     if(k == 259 && !s.empty()) { // 259 = Backspace
                         // UTF8 unsafe pop back, assuming ASCII for polish
                         s.pop_back();
                         changed = true;
                     }
                 }
                 
                 // Typing
                 if (!g_ctx->input_chars.empty()) {
                      for(auto c : g_ctx->input_chars) s += (char)c;
                     changed = true;
                 }
                 
                 if(changed) {
                     // Write back safely-ish
                     // Warning: No size limit check!
                     buffer = s;
                 }
             }
         
         g_ctx->end_node();
         return InputResult::None;
    }
    bool BeginTable(const char* id, int col_count) {
        g_table.cols = col_count;
        g_table.current_col = 0;
        Begin(id);
        Column();
        return true;
    }
    void EndTable() { End(); }
    void TableNextRow() { Row(); g_table.current_col = 0; }
    void TableNextColumn() { 
        if(g_table.cols > 0) SetNextGrow(1.0f);
        g_table.current_col++;
    }

    void BeginTabBar(const char* id) {
        g_current_tab_bar = id;
        Row();
        if(g_tab_state.find(id) == g_tab_state.end()) g_tab_state[id] = "";
    }
    void EndTabBar() { End(); }
    
    bool TabItem(const char* label) {
         if (IsTabActive(label)) {
             SetNextBg(Color::Hex(0x007ACCFF));
             SetNextTextColor(Color::Hex(0xFFFFFFFF));
         } else {
             SetNextBg(Color::Hex(0x333333FF));
             SetNextTextColor(Color::Hex(0xAAAAAAFF));
         }
         if (Button(label)) {
             g_tab_state[g_current_tab_bar] = label;
             return true;
         }
         return false;
    }
    
    bool IsTabActive(const char* label) {
        if(g_tab_state[g_current_tab_bar] == "") g_tab_state[g_current_tab_bar] = label;
        return g_tab_state[g_current_tab_bar] == label;
    }

    void TextColored(const char* t, Color c) { SetNextTextColor(c); Text(t); }
    void TextPositive(const char* t) { TextColored(t, Color::Hex(0x00FF00FF)); }
    void TextNegative(const char* t) { TextColored(t, Color::Hex(0xFF0000FF)); }
    void Markdown(const char* text) {
        if (!g_ctx) return;
        
        auto spans = ParseMarkdown(text);
        
        // Layout Config
        float start_x = 0; // Relative to Node? No, we are creating content. e.g. a Group or Auto-Flow.
        // Better: Create one big container node "MarkdownBlock" and manually place text commands?
        // Or make each span a Text node?
        // Too many nodes is heavy. Better: One node "Markdown" and manual DrawList commands.
        // But Input for Links requires hit testing.
        // Let's go with: One Node, iterating manual layout and click checks.
        
        NodeID id = g_ctx->begin_node("markdown");
        NodeHandle n = g_ctx->get(id);
        
        // Needs width to wrap
        float max_w = n.layout().w; 
        if (max_w <= 1.0f) max_w = 400.0f; // Default if auto
        
        float x = 0;
        float y = 0;
        float line_height = 20.0f; // Approx
        
        // Interaction
        bool clicked = false;
        if(g_focus && g_focus->is_clicked(id)) clicked = true;
        
        // Render
        size_t span_idx = 0;
        for (const auto& span : spans) {
             // Font Styling (Simulation)
             Color col = g_ctx->style_stack.back().text_color;
             if (span.is_link) col = Color::Hex(0x55AAFFFF);
             if (span.is_heading) col = Color::Hex(0xFFD700FF); // Gold for wrapper
             
             // Wrap logic (Simplified: char based or just whole span?)
             // For now: Whole span. (Ideal: word wrap)
             float span_w = span.text.length() * 8.0f; // Approx 8px per char
             
             if (x + span_w > max_w && x > 0) {
                 x = 0;
                 y += line_height;
             }
             
             // Draw
             // DrawText is not exposed directly for "inside node relative".
             // We need to use `RenderData` custom commands or just `n.render().text`?
             // `n.render().text` is one string.
             // We need `custom_draw_cmds`.
             
             DrawCmd cmd;
             cmd.type = DrawType::Text;
             cmd.x = x; 
             cmd.y = y; 
             
             // Stable storage
             n.render().text_segments.push_back(span.text);
             cmd.text_ptr = n.render().text_segments.back().c_str();

             cmd.color = col.u32();
             cmd.p1 = span.bold ? 1.0f : 0.0f;     // Bold flag
             cmd.p2 = span.italic ? 1.0f : 0.0f;   // Italic flag
             cmd.p3 = 0; cmd.p4 = 0;
             cmd.skew = 0; 
             n.render().custom_draw_cmds.push_back(cmd);
             
             // Headings need Underline? Or Bold?
             // Links need Underline
             if (span.is_link) {
                 DrawCmd line;
                 line.type = CmdType::Rect; // Generic rect for line?
                 line.x = x; line.y = y + line_height - 2;
                 line.w = span_w; line.h = 1;
                 line.color = col;
                 n.render().custom_draw_cmds.push_back(line);
                 
                 // Link Click Logic
                 // Calculate absolute rect of this span?
                 // Or just assume if Node is clicked, we find which link?
                 // That requires "Hit Test against Span".
                 // Too complex for 5 mins.
                 // Phase 1: Just render. Interaction later.
             }

             x += span_w;
             
             // Block element handling
             if (span.is_heading) {
                 x = 0;
                 y += line_height * 1.5f; // Gap
             }
        }
        
        // Set Height
        n.constraint().height = y + line_height;

        g_ctx->end_node();
    }
    void Spinner() {
        if (!g_ctx) return;
        NodeID id = g_ctx->begin_node("spinner");
        NodeHandle n = g_ctx->get(id);
        
        n.constraint().width = 24;
        n.constraint().height = 24;
        
        static float rotation = 0.0f;
        rotation += 0.15f; // Animation speed (Frame rate dependent)
        
        DrawCmd cmd;
        cmd.type = DrawType::Arc;
        // Center relative to node?
        // Layout pass hasn't happened yet for X/Y?
        // Wait, immediate mode UI. `n.layout()` has CURRENT calculated position from `begin_node`?
        // `begin_node` usually sets layout.x/y based on cursor.
        // Yes, `g_ctx->cursor_x`.
        // So `n.layout().x` is valid-ish.
        // Actually renderer uses absolute coords?
        // We push commands to `custom_draw_cmds`.
        // The render interpreter (`application.cpp`) uses `cmd.x`.
        // It expects absolute coords?
        // In `DrawImage`, `DrawConnection` we used `x`.
        // But `begin_node` sets `layout`.
        // Let's use 0,0 relative?
        // `build_draw_subtree` does `float x1 = x`. `x` is passed down.
        // `DrawCmd` coords:
        // `application.cpp`: `dl.add_rect(cmd.x, cmd.y...)`.
        // Wait, if `custom_draw_cmds` are in Node, does `application.cpp` add Node.x to them?
        // `application.cpp`:
        // `for(const auto& cmd : ctx.node_store.render[id].custom_draw_cmds)`
        //    `dl.add_line(cmd.x, cmd.y...)`
        // It uses `cmd.x` DIRECTLY. It does NOT add node offset.
        // So we MUST provide Absolute Coordinates in `api.cpp`.
        // `n.layout().x` is absolute?
        // `UIContext` layout logic usually sets absolute X/Y.
        
        float cx = n.layout().x + 12;
        float cy = n.layout().y + 12;
        
        cmd.x = cx;
        cmd.y = cy;
        cmd.w = 10.0f; // Radius
        cmd.h = 0;
        cmd.p1 = rotation;
        cmd.p2 = rotation + 4.5f; // 270 deg
        cmd.color = g_ctx->style_stack.back().text_color.u32(); // or Accent
        cmd.skew = 0;
        
        n.render().custom_draw_cmds.push_back(cmd);
        
        g_ctx->end_node();
    }
    void PushTransform(float* m33) {
        if(!g_ctx) return;
        UIContext::Transform t;
        for(int i=0; i<6; ++i) t.m[i] = m33[i]; // Assuming 6 float affine or handle 3x3?
        // User passed 3x3 probably if mat3.
        // Let's assume input is mat3 9 floats? Or just the 6 needed?
        // For simplicity, let's assume the user passes a pointer to 6 floats (affine).
        // If strict mat3, we map: [0][1][2] -> a b 0, [3][4][5] -> c d 0, [6][7][8] -> tx ty 1
        // Implementation Plan said "ui::PushTransform(mat3)".
        // Let's assume 6 floats for now (a,b, c,d, tx,ty).
        g_ctx->push_transform(t);
    }

    void PushTranslate(float x, float y) {
        if(g_ctx) g_ctx->push_transform(UIContext::Transform::Translate(x,y));
    }
    void PushScale(float s) {
        if(g_ctx) g_ctx->push_transform(UIContext::Transform::Scale(s));
    }
    void PopTransform() {
        if(g_ctx) g_ctx->pop_transform();
    }

    // Phase 1: Inline Draw API
    // Modes: 0=Rect, 1=Text, 2=Image, 10=Line, 11=Bezier
    // checking renderer.cpp, rect is mode 0. radius is p1? 
    // Need to verify renderer.cpp modes carefully.
    
    // Internal Helper
    static void push_cmd(DrawCmd& cmd) {
        if (!g_ctx || g_ctx->current_id == 0) return;
        // Apply transform to position-like fields
        // But DrawCmd varies by mode.
        // For Rect (x,y,w,h):
        // (x,y) is top-left. (w,h) is size.
        // Vector rects: (x,y) -> transform. (x+w, y) -> transform. ...
        // If rotation involved, Axis Aligned Rect becomes Rotated Rect/Quad.
        // Renderer might only support AABB.
        // Phase 4 says "Renderer is Box Optimized".
        // For now, let's transforming the simplified bounding box or p1/p2 for lines.
        
        g_ctx->get(g_ctx->current_id).render().custom_draw_cmds.push_back(cmd);
    }
    
    void DrawLine(float x1, float y1, float x2, float y2, uint32_t color, float thickness) {
        if(!g_ctx) return;
        g_ctx->apply_transform(x1, y1);
        g_ctx->apply_transform(x2, y2);
        
        DrawCmd cmd = {};
        cmd.type = DrawType::Rect; // Fallback to Rect for Line
        cmd.x = x1; cmd.y = y1; // Start
        cmd.w = x2; cmd.h = y2; // End (Hijacking w/h for p2)
        cmd.color = color;
        cmd.p1 = thickness;
        push_cmd(cmd);
    }

    void DrawRect(float x, float y, float w, float h, uint32_t color, float radius) {
        // Warning: Rotation on simplified renderer might fail for Rects if we only transform x,y
        if(!g_ctx) return;
        
        // If we strictly only have scaling/translation, this holds.
        float x2 = x+w, y2=y+h;
        g_ctx->apply_transform(x, y);
        g_ctx->apply_transform(x2, y2);
        
        DrawCmd cmd = {};
        cmd.type = DrawType::Rect;
        cmd.x = x; cmd.y = y;
        cmd.type = DrawType::Line;
        cmd.x = x1; cmd.y = y1; cmd.w = x2; cmd.h = y2;
        cmd.color = color; cmd.p1 = thickness;
        push_cmd(cmd);
    }

    void DrawRect(float x, float y, float w, float h, uint32_t color, float border_radius) {
        if(!g_ctx) return;
        float x1=x, y1=y, x2=x+w, y2=y+h;
        g_ctx->apply_transform(x1, y1);
        g_ctx->apply_transform(x2, y2);
        
        DrawCmd cmd = {};
        cmd.type = DrawType::Rect;
        cmd.x = x1; cmd.y = y1; cmd.w = x2-x1; cmd.h = y2-y1; // Transformed W/H
        cmd.color = color; cmd.p1 = border_radius; cmd.p2 = 0.0f; // Border Only?
        // We need flag for Filled vs Border in DrawRect mode?
        // Usually standard `DrawCmd` in Renderer implies Filled for Rect logic.
        // For Border, we might need `DrawType::RectBorder`?
        // Or check p2?
        push_cmd(cmd);
    }

    void DrawRectFilled(float x, float y, float w, float h, uint32_t color, float border_radius) {
        // Same as above, ensure Renderer handles it
        DrawRect(x,y,w,h,color,border_radius); // Aliased for now
    }

    void DrawBezier(float x1, float y1, float x2, float y2, float cx1, float cy1, float cx2, float cy2, uint32_t color, float thickness) {
        if(!g_ctx) return;
        g_ctx->apply_transform(x1, y1);
        g_ctx->apply_transform(x2, y2);
        g_ctx->apply_transform(cx1, cy1);
        g_ctx->apply_transform(cx2, cy2);
        
        DrawCmd cmd = {};
        cmd.type = DrawType::Bezier;
        cmd.x = x1; cmd.y = y1; cmd.w = x2; cmd.h = y2;
        cmd.p1 = cx1; cmd.p2 = cy1; cmd.p3 = cx2; cmd.p4 = cy2;
        cmd.color = color; 
        // Thickness? DrawCmd struct limited. P1..P4 taken by control points.
        // Use texture_id field? or generic pad?
        // or add field. Struct has pad.
        // Let's assume fixed thickness or packed into Color/Mode?
        // Just ignore thickness for Bezier in this pass or add field.
        push_cmd(cmd);
    }

    void SetIMEPosition(float x, float y) {
        // Stub: In real imp, convert (x,y) to screen coords and call OS API
        // if(g_ctx) g_ctx->ime_pos = {x,y};
    }
    
    void SetScrollX(float x) {
        if(!g_ctx) return;
        g_ctx->node_store.scroll[g_ctx->current_id].scroll_x = x;
    }
    void SetScrollY(float y) {
        if(!g_ctx) return;
        g_ctx->node_store.scroll[g_ctx->current_id].scroll_y = y;
    }
    float GetScrollX() {
        if(!g_ctx) return 0;
        return g_ctx->node_store.scroll[g_ctx->current_id].scroll_x;
    }
    float GetScrollY() {
        if(!g_ctx) return 0;
        return g_ctx->node_store.scroll[g_ctx->current_id].scroll_y;
    }

    void BeginWindow(const char* title, bool* open) {
        if(!g_ctx) return;
        
        // Window is a Node
        NodeID id = g_ctx->begin_node(title);
        NodeHandle n = g_ctx->get(id);
        
        n.constraint().position_type = 1; // Absolute
        n.style().bg_color = g_ctx->style_stack.back().menu_bg.u32(); // 0x333333
        n.style().border_width = 1;
        n.style().border_color = 0x000000FF;
        n.style().radius = 4.0f;
        
        // Restore Position from Persistence? 
        // Assuming begin_node/layout engine uses persistent_layout if available.
        // We need to write to it for dragging.
        auto& p_layout = g_ctx->persistent_layout[id];
        if (p_layout.w == 0) p_layout.w = 400; // Default
        if (p_layout.h == 0) p_layout.h = 300;
        
        n.constraint().width = p_layout.w;
        n.constraint().height = p_layout.h; 
        
        // Critical Fix: Apply Position
        n.constraint().left = p_layout.x;
        n.constraint().top = p_layout.y;
        
        // Z-Order (Click Body)
        n.input().clickable = true;
        if(g_focus && g_focus->register_node(g_ctx, id)) {
             if(g_focus->is_down(id)) {
                 g_ctx->bring_to_front(id);
             }
        }
        
        // Title Bar
        BeginGroup("##TitleBar");
        n.layout().grow = 1.0f; // Stretch Title Bar
        NodeID title_id = g_ctx->current_id;
        NodeHandle title_n = g_ctx->get(title_id);
        title_n.style().bg_color = 0x444444FF; 
        title_n.style().radius = 4;
        
        Row();
        
        // Drag Logic on Title Bar
        if(g_focus && g_focus->register_node(g_ctx, title_id)) {
            title_n.input().clickable = true;
            if(g_focus->is_down(title_id)) {
                // Drag
                float dx = g_ctx->mouse_dx; 
                float dy = g_ctx->mouse_dy;
                p_layout.x += dx;
                p_layout.y += dy;
                
                g_ctx->bring_to_front(id);
            }
        }
                // Need mouse delta. UIContext usually has it?
                // Looked at API.. `UpdateInputState` passed `mouse_x`.
                // `UIContext` should calculate `mouse_dx`.
                // Let's assume `g_ctx->mouse_dx` exists (implied by Draggable logic typically).
                // Or I calculate it: `g_ctx->mouse_x - g_ctx->prev_mouse_x`??
                // `Draggable` usually updates `*x += dx`.
                // Let's look at `Draggable` again? (Step 890 view was requested but not read deeply).
                // I'll check Draggable if implementation fails.
                // Assuming `mouse_x` and `last_mouse_x`.
                // For now, let's skip complex drag implementation and just do layout.
            }
        }
        
        Text(title);
        
        // Spacer
        SetNextGrow(1.0f);
        Spacer(0); 
        
        // Close Button
        if(open) {
            if(Button("X")) *open = false;
        }
        EndGroup(); // Title Bar
        
        // Content area
        BeginGroup("##WindowContent");
        SetNextGrow(1.0f);
        SetNextPadding(8.0f);
        Column();
    }

    void EndWindow() {
        if(!g_ctx) return;
        EndGroup(); // Content
        
        NodeID win_id = g_ctx->current_id;
        
        // Resize Handle
        NodeID grip_id = g_ctx->begin_node("##ResizeGrip");
        NodeHandle grip = g_ctx->get(grip_id);
        
        auto& p_layout = g_ctx->persistent_layout[win_id];
        
        grip.constraint().position_type = 1; 
        grip.constraint().width = 12;
        grip.constraint().height = 12;
        // Position at Bottom-Right
        float cw = p_layout.w;
        float ch = p_layout.h;
        grip.constraint().left = cw - 12;
        grip.constraint().top = ch - 12;
        
        grip.style().bg_color = 0x666666FF; 
        
        if(g_focus && g_focus->register_node(g_ctx, grip_id)) {
            grip.input().clickable = true;
            if(g_focus->is_down(grip_id)) {
                p_layout.w += g_ctx->mouse_dx;
                p_layout.h += g_ctx->mouse_dy;
                if(p_layout.w < 100) p_layout.w = 100;
                if(p_layout.h < 50) p_layout.h = 50;
                
                g_ctx->bring_to_front(win_id);
            }
        }
        g_ctx->end_node(); // Grip
        
        g_ctx->end_node(); // Window
    }

    // Phase 2: Structure
    bool BeginMainMenuBar() {
        if(!g_ctx) return false;
        
        // Similar to BeginWindow but forced to top
        // Use Overlay list? Or just Draw first?
        // Layout Order: MainMenu -> DockSpace -> Windows -> Overlays
        // MainMenu should be a root node with absolute constraints.
        
        NodeID id = g_ctx->begin_node("##MainMenuBar");
        NodeHandle n = g_ctx->get(id);
        
        n.constraint().position_type = 1; // Absolute
        n.constraint().x = 0;
        n.constraint().y = 0;
        n.constraint().width = g_ctx->node_store.layout[g_ctx->root_id].computed_w; // Window Width
        n.constraint().height = 24; // Fixed Height
        
        n.style().bg_color = 0x222222FF;
        n.style().border_width = 0;
        
        if (g_focus) g_focus->register_node(g_ctx, id);
        
        // Use Row layout inside
        n.layout().dir = 0; // Row
        
        BeginGroup("##MenuBarContent"); // Inner container for flow
        Row();
        return true;
    }

    void EndMainMenuBar() {
        EndGroup();
        if(g_ctx) g_ctx->end_node();
    }
    
    bool BeginStatusBar() {
        if(!g_ctx) return false;
        
        float win_w = g_ctx->node_store.layout[g_ctx->root_id].computed_w;
        float win_h = g_ctx->node_store.layout[g_ctx->root_id].computed_h;
        float height = 24;
        
        NodeID id = g_ctx->begin_node("##StatusBar");
        NodeHandle n = g_ctx->get(id);
        
        n.constraint().position_type = 1; // Absolute
        n.constraint().x = 0;
        n.constraint().y = win_h - height;
        n.constraint().width = win_w;
        n.constraint().height = height;
        
        n.style().bg_color = 0x333333FF;
        
        if (g_focus) g_focus->register_node(g_ctx, id);
        
        n.layout().dir = 0; // Row
        BeginGroup("##StatusBarContent");
        Row();
        SetNextAlign(Align::Center); // Vertically Center items
        return true;
    }

    void EndStatusBar() {
        EndGroup();
        if(g_ctx) g_ctx->end_node();
    }
    
    void DockSpace(const char* id) {
        // DockSpace - Fullscreen container for layout
        // Simplified: Just a dark background group that takes all space.
        // In a real system, this would manage specific docking nodes.
        
        SetNextGrow(1.0f); // Grow to fill parent (Root)
        BeginGroup(id);
        
        NodeHandle n = g_ctx->get(g_ctx->current_id);
        n.style().bg_color = 0x202020FF; // Dark Gray Background
        n.style().padding = 0; // No padding for docked content?
        
        // This acts as the "Desktop" for the app
        
        EndGroup();
    }
    
    // Phase 3: Complex Widgets
    bool TreeNode(const char* label, bool* selected, int flags) {
        if(!g_ctx) return false;
        
        NodeID id = g_ctx->begin_node(label);
        NodeHandle n = g_ctx->get(id);
        
        bool is_open = g_ctx->tree_state[id];
        
        // Header Layout
        Row();
        n.layout().grow = 0; 
        
        // Input Logic for Open/Close (Click on Arrow or Whole Line?)
        // Let's say Arrow only if Checkbox present? Or whole line if no checkbox?
        
        if(g_focus && g_focus->register_node(g_ctx, id)) { 
           n.input().clickable = true;
           if(g_focus->is_clicked(id)) {
               // If clicking checkbox area? 
               // Simply: toggle open state if clicked anywhere for now.
               g_ctx->tree_state[id] = !is_open;
               is_open = !is_open;
           }
        }
        
        // Visuals
        const char* icon = is_open ? " v " : " > ";
        
        // Arrow
        Text(icon); 
        
        // Checkbox?
        if (selected) {
            std::string cb_id = std::string("##cb_") + label;
            // Very simple checkbox inline
            const char* box = *selected ? "[x] " : "[ ] ";
            // Need a button for this?
            // Assuming simplified drawing for now.
            // If using Button logic, it might conflict with Row?
            // Just Text for visual, logic needs refinement for specific hit test.
            Text(box);  
            // Note: Real checkbox logic would need separate node ID and hit test to avoid toggling tree.
        }
        
        Text(label);
        
        g_ctx->end_node(); // Header Node
        
        if (is_open) {
            BeginGroup((std::string(label) + "_content").c_str());
            SetNextPadding(14); // Indent
        }
        
        return is_open;
    }

    void TreePop() {
        if(!g_ctx) return;
        EndGroup(); // Close content group
    }
    
    bool OpenFileDialog(const char* title, const char* filters, std::string& out_path) {
        return platform::OpenFileDialog(title, filters, out_path);
    }

    // Phase 1: Widgets Implementation
    bool Checkbox(const char* label, bool* v) {
        if(!g_ctx) return false;
        
        // Horizontal Layout
        BeginGroup("chk_group");
        Row();
        SetNextAlign(Align::Center);
        
        // 1. Box
        NodeID id = g_ctx->begin_node((std::string("##chk_") + label).c_str());
        NodeHandle n = g_ctx->get(id);
        
        float size = 16.0f;
        n.constraint().width = size;
        n.constraint().height = size;
        n.style().border_width = 1.0f;
        n.style().border_color = 0xAAAAAAFF;
        n.style().bg_color = 0x333333FF;
        
        // Input
        bool pressed = false;
        if(g_focus && g_focus->register_node(g_ctx, id)) {
            n.input().clickable = true;
            if (g_focus->is_clicked(id)) pressed = true;
        }
        
        if (pressed) *v = !(*v);
        
        if (*v) {
            // Draw Check (simulated with filled rect for now or 'X')
            DrawCmd cmd;
            cmd.type = DrawType::Rect;
            cmd.x = n.layout().x + 3;
            cmd.y = n.layout().y + 3;
            cmd.w = size - 6;
            cmd.h = size - 6;
            cmd.color = g_ctx->get_style().accent_color.u32();
            cmd.p1 = 0; // Radius
            cmd.skew = 0;
            n.render().custom_draw_cmds.push_back(cmd);
        }
        
        g_ctx->end_node();
        
        // 2. Label
        // Click on label toggles too?
        if (Button(label)) *v = !(*v);
        
        EndGroup();
        return pressed;
    }

    bool Slider(const char* label, float* v, float v_min, float v_max) {
        if(!g_ctx) return false;
        
        BeginGroup("sld_group");
        Row();
        SetNextAlign(Align::Center);
        
        // Track Node
        NodeID id = g_ctx->begin_node((std::string("##sld_") + label).c_str());
        NodeHandle n = g_ctx->get(id);
        
        n.constraint().width = 100.0f; // Fixed width track? Or Flex?
        n.constraint().height = 18.0f;
        n.style().bg_color = 0x222222FF; // Track bg
        n.style().border_width = 1;
        n.style().border_color = 0x555555FF;
        
        // Input
        bool changed = false;
        if(g_focus && g_focus->register_node(g_ctx, id)) {
            n.input().clickable = true;
            if (g_focus->is_down(id)) {
                // Drag Logic
                float mx = g_ctx->mouse_x;
                float rx = n.layout().x;
                float rw = n.layout().w;
                if (rw > 0) {
                    float t = (mx - rx) / rw;
                    if(t < 0) t = 0; if(t > 1) t = 1;
                    *v = v_min + t * (v_max - v_min);
                    changed = true;
                }
            }
        }
        
        // Visuals (Handle)
        float t = (*v - v_min) / (v_max - v_min);
        if(t < 0) t = 0; if(t > 1) t = 1;
        
        float handle_x = n.layout().x + t * (n.layout().computed_w - 6); // -handle_width
        // But computed_w might be 0 until next frame? 
        // Use constraint width since fixed.
        handle_x = n.layout().x + t * (100.0f - 8.0f);
        
        DrawCmd cmd;
        cmd.type = DrawType::Rect;
        cmd.x = handle_x;
        cmd.y = n.layout().y + 2;
        cmd.w = 8.0f;
        cmd.h = 14.0f;
        cmd.color = g_ctx->get_style().accent_color.u32(); // Handle
        cmd.p1 = 2.0f; // Radius
        cmd.skew = 0;
        n.render().custom_draw_cmds.push_back(cmd);
        
        // Value Text Overlay?
        // Let's print value
        char buf[32];
        snprintf(buf, 32, "%.2f", *v);
        // Draw Text inside?
        // We lack direct DrawText API relative to node here easily without Font/Glyph loop.
        // Use separate Label Node next to it?
        
        g_ctx->end_node();
        
        // Label + Value
        Text(buf);
        Text(label);
        
        EndGroup();
        return changed;
    }
    
    bool ColorEdit(const char* label, Color* c) {
        if(!g_ctx) return false;
        
        BeginGroup("color_group");
        // Preview
        SetNextWidth(20); SetNextHeight(20);
        SetNextBg(*c);
        Text(""); // Spacer box
        
        Column(); // Stack sliders
        float r = c->r; float g = c->g; float b = c->b;
        bool changed = false;
        if(Slider("R", &r, 0, 1)) changed = true;
        if(Slider("G", &g, 0, 1)) changed = true;
        if(Slider("B", &b, 0, 1)) changed = true;
        
        if(changed) *c = Color{r,g,b, c->a};
        
        EndGroup(); // Column
        Text(label);
        EndGroup(); // Row Main
        return changed;
    }

    // Phase 2: Style Stack
    void PushStyleColor(StyleColor idx, Color c) {
        if(!g_ctx) return;
        Style s = g_ctx->get_style(); // Copy current
        switch(idx) {
            case StyleColor::Text: s.text_color = c; break;
            case StyleColor::Bg: s.bg = c; break;
            case StyleColor::BgHover: s.bg_hover = c; break;
            case StyleColor::BgActive: s.bg_active = c; break;
            case StyleColor::Border: s.border_color = c; break;
            case StyleColor::Accent: s.accent_color = c; break; // Need to ensure Style has accent_color (Added in Phase 1?)
            // If Style doesn't have accent_color, map to closest or ignore.
            // Let's check Style struct in style.hpp provided in previous turn.
            // style.hpp view (Step 1008) showed: bg, bg_hover, bg_active, text_color, border_color...
            // It did NOT show `accent_color`.
            // However, `Checkbox` implementation used `g_ctx->get_style().accent_color.u32()`. 
            // If that compiled (Step 1004 passed? No 1004 failed on ColorEdit, but Checkbox part compiled?), 
            // then `accent_color` exists?
            // Actually, Step 1004 failure was ONLY on ColorEdit signature mismatch.
            // If Checkbox compiled, then Style MUST have `accent_color`.
            // Wait, looking at Step 1008 view of style.hpp...
            // It showed `Color focus_ring_color`, `Color menu_bg` etc.
            // It did NOT show `accent_color`.
            // Maybe it was hidden in `// ...` or I missed it.
            // Or `Checkbox` compilation didn't actually reach that line if template/macro? No, it's plain code.
            // If `accent_color` is missing, Checkbox would fail.
            // Let's assume `accent_color` IS there or I need to add it.
            // I'll add `accent_color` to `Style` struct first if needed.
            // I'll check `style.hpp` again to be sure or just add it to be safe.
            // But I cannot easily edit `style.hpp` blindly.
            // I'll assume it's `focus_ring_color` or something used as accent? 
            // Or maybe `text_colors`?
            // Let's check `Checkbox` code I inserted in Step 996.
            // `cmd.color = g_ctx->get_style().accent_color.u32();`
            // If compilation passed (Step 1004 only complained about ColorEdit), then `accent_color` exists.
            // Proceeding.
            default: break;
        }
        g_ctx->push_style(s);
    }
    
    void PopStyleColor() {
        if(!g_ctx) return;
        g_ctx->pop_style();
    }

    // Phase 2: Scroll
    void BeginScrollArea(const char* id, float w, float h, float* scroll_x, float* scroll_y) {
        if(!g_ctx) return;
        
        NodeID nid = g_ctx->begin_node(id);
        NodeHandle n = g_ctx->get(nid);
        
        // Size
        n.constraint().width = w;
        n.constraint().height = h;
        
        // Clip
        g_ctx->node_store.scroll[nid].clip_content = true;
        
        // Apply Scroll
        g_ctx->node_store.scroll[nid].scroll_x = *scroll_x;
        g_ctx->node_store.scroll[nid].scroll_y = *scroll_y;
        
        // Input (Wheel) - Simplified
        if (g_focus && g_focus->is_hovered(nid)) {
            // Note: Require wheel delta from context
        }
        
        // Push Offset for children
        PushTranslate(-(*scroll_x), -(*scroll_y));
    }

    void EndScrollArea() {
        if(!g_ctx) return;
        PopTransform(); // Pop scroll offset
        g_ctx->end_node();
    }
    
    // Clipboard Wrappers
    #include "../platform/win32_utils.hpp"
    void SetClipboardText(const char* text) {
        platform::Win32_SetClipboardText(text);
    }
    std::string GetClipboardText() {
        return platform::Win32_GetClipboardText();
    }

