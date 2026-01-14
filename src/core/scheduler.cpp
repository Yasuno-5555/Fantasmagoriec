#include "fanta.h"
#include "fanta_views.h"
#include "core/contexts_internal.hpp"
#include "backend/drawlist.hpp"
#include "backend/backend.hpp"
#include "element/layout.hpp"
#include "render/render_tree.hpp"
#include "ui/id_internal.hpp"
#include "platform/native_bridge.hpp" // For keyboard/haptic etc

#include <iostream>
#include <algorithm>
#include <map>
#include <cmath>
#include <chrono>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h> // For GetAsyncKeyState
#endif

namespace fanta {

    // Macro Compat for Scheduler (Context is defined in context.cpp but accessible)
    #define g_ctx (fanta::internal::GetEngineContext())

    // Forward Decl
    extern void Lower(Node* root);
    bool IsMouseDown();
    bool IsKeyDown(int key);
    bool IsKeyJustPressed(int key);
    bool IsMouseJustPressed();
    
    // We need IsMouseDown etc internally? Or can we query input state directly?
    // Access g_ctx.input_ctx directly.

    void BeginFrame() {
        // Phase 1: Reset V5 Memory Arena
        g_ctx.frame.arena.reset();
        
        // Sync FrameState params
        g_ctx.frame.dt = g_ctx.runtime.dt;

        g_ctx.input_ctx.state.update_frame();
        if (g_ctx.world.backend) {
            float last_x = g_ctx.input_ctx.mouse_x;
            float last_y = g_ctx.input_ctx.mouse_y;
            
            g_ctx.world.backend->get_mouse_state(
                g_ctx.input_ctx.mouse_screen.x, 
                g_ctx.input_ctx.mouse_screen.y, 
                g_ctx.input_ctx.mouse_down,
                g_ctx.input_ctx.mouse_wheel
            );
            
            // Sync Legacy & World Init
            g_ctx.input_ctx.mouse_x = g_ctx.input_ctx.mouse_screen.x;
            g_ctx.input_ctx.mouse_y = g_ctx.input_ctx.mouse_screen.y;
            g_ctx.input_ctx.mouse_world = g_ctx.input_ctx.mouse_screen;

            g_ctx.input_ctx.mouse_dx = g_ctx.input_ctx.mouse_screen.x - last_x;
            g_ctx.input_ctx.mouse_dy = g_ctx.input_ctx.mouse_screen.y - last_y;
        }

        // Phase 11: Tab-based focus traversal
        // TODO: Refactor into Accessibility System
        static bool tab_was_pressed = false;
        bool tab_is_pressed = IsKeyDown(0x09); // VK_TAB
        if (tab_is_pressed && !tab_was_pressed) {
            // Collect focusable IDs from previous frame
            std::vector<internal::ID> focusable_ids;
            for (const auto& el : g_ctx.store.frame_elements) {
                if (el.is_focusable) focusable_ids.push_back(el.id);
            }
            
            if (!focusable_ids.empty()) {
                auto it = std::find(focusable_ids.begin(), focusable_ids.end(), g_ctx.input_ctx.focused_id);
                if (it == focusable_ids.end()) {
                    g_ctx.input_ctx.focused_id = focusable_ids[0];
                } else {
                    size_t idx = std::distance(focusable_ids.begin(), it);
                    g_ctx.input_ctx.focused_id = focusable_ids[(idx + 1) % focusable_ids.size()];
                }
            }
        }
        tab_was_pressed = tab_is_pressed;
        
        // Phase 6.2: Hit Test Debugging Toggle
        static bool f12_was_pressed = false;
        bool f12_is_pressed = IsKeyDown(0x7B); // VK_F12
        if (f12_is_pressed && !f12_was_pressed) {
            g_ctx.runtime.debug_hit_test = !g_ctx.runtime.debug_hit_test;
            Log(std::string("Hit Test Debug: ") + (g_ctx.runtime.debug_hit_test ? "ON" : "OFF"));
        }
        f12_was_pressed = f12_is_pressed;


        // Phase 41: Gesture Detection (Desktop Simulation)
        auto& gesture = g_ctx.input_ctx.gesture;
        if (g_ctx.input_ctx.state.is_just_pressed()) {
            gesture.is_swiping = true;
            gesture.start_pos = {g_ctx.input_ctx.mouse_x, g_ctx.input_ctx.mouse_y};
            gesture.start_time = glfwGetTime();
            gesture.current_delta = {0, 0};
        } else if (g_ctx.input_ctx.mouse_down && gesture.is_swiping) {
            gesture.current_delta.x = g_ctx.input_ctx.mouse_x - gesture.start_pos.x;
            gesture.current_delta.y = g_ctx.input_ctx.mouse_y - gesture.start_pos.y;
        } else {
            if (gesture.is_swiping) {
                float dist = std::sqrt(gesture.current_delta.x*gesture.current_delta.x + gesture.current_delta.y*gesture.current_delta.y);
                double duration = glfwGetTime() - gesture.start_time;
                if (dist > 50.0f && duration < 0.3) {
                    Log("Gesture: Swipe Detected");
                    Native::HapticFeedback(1); 
                }
            }
            gesture.is_swiping = false;
            gesture.current_delta = {0, 0};
        }

        // Pinch Simulation
        bool is_shift = IsKeyDown(0x10) || IsKeyDown(340); 
        if (is_shift && g_ctx.input_ctx.mouse_wheel != 0) {
            if (!gesture.is_pinching) {
                gesture.is_pinching = true;
                gesture.current_scale = 1.0f;
            }
            gesture.current_scale += g_ctx.input_ctx.mouse_wheel * 0.1f;
        } else {
            if (g_ctx.input_ctx.mouse_wheel == 0) 
                gesture.is_pinching = false;
        }

        // Phase 5.1: Marquee & Dragging Logic
        auto& input = g_ctx.input_ctx;
        bool clicked_selected = false;
        if (g_ctx.input_ctx.state.is_just_pressed()) {
            for (const auto& el : g_ctx.store.elements) {
                if (el.is_selected && el.is_hovered) {
                    clicked_selected = true;
                    break;
                }
            }
            
            if (clicked_selected) {
                input.is_dragging_nodes = true;
                input.marquee_active = false;
            } else {
                input.is_dragging_nodes = false;
                input.marquee_active = true;
                input.marquee_start = {input.mouse_x, input.mouse_y};
                input.marquee_rect = {input.mouse_x, input.mouse_y, 0, 0};
                
                bool is_shift = IsKeyDown(0x10);
                bool is_ctrl = IsKeyDown(0x11);
                
                if (!is_shift && !is_ctrl) {
                    for (auto& el : g_ctx.store.elements) {
                        el.is_selected = false;
                        el.persistent.is_selected = false; 
                        g_ctx.store.persistent_states[el.id] = el.persistent;
                    }
                }
            }
        }

        // Phase 5.2: Wire Dragging Logic
        if (input.is_dragging_wire) {
            if (!g_ctx.input_ctx.mouse_down) {
                input.is_dragging_wire = false;
                for (const auto& el : g_ctx.store.elements) {
                    if (el.is_port && el.is_hovered && el.id != input.wire_start_id) {
                        Log("Connection Made");
                        input.connection_events.push_back({input.wire_start_id, el.id});
                    }
                }
            }
        } else {
            if (g_ctx.input_ctx.state.is_just_pressed()) {
                for (const auto& el : g_ctx.store.elements) {
                    if (el.is_port && el.is_hovered) {
                        input.is_dragging_wire = true;
                        input.wire_start_id = el.id;
                        return; // Consume
                    }
                }
            }
        }
        
        // Drag/Marquee Updates
        if (input.is_dragging_nodes) {
            if (g_ctx.input_ctx.mouse_down) {
                for (auto& el : g_ctx.store.elements) {
                    if (el.is_selected) {
                        el.persistent.drag_offset.x += input.mouse_dx;
                        el.persistent.drag_offset.y += input.mouse_dy;
                        g_ctx.store.persistent_states[el.id] = el.persistent;
                    }
                }
            } else {
                input.is_dragging_nodes = false;
            }
        } else if (input.marquee_active) {
            if (g_ctx.input_ctx.mouse_down) {
                float x1 = input.marquee_start.x;
                float y1 = input.marquee_start.y;
                float x2 = input.mouse_x;
                float y2 = input.mouse_y;
                input.marquee_rect.x = std::min(x1, x2);
                input.marquee_rect.y = std::min(y1, y2);
                input.marquee_rect.w = std::abs(x1 - x2);
                input.marquee_rect.h = std::abs(y1 - y2);
            } else {
                input.marquee_active = false;
            }
        }

        g_ctx.store.clear_frame();
        g_ctx.runtime.prev_hero_registry = std::move(g_ctx.runtime.hero_registry);
        g_ctx.runtime.hero_registry.clear();

        internal::GetGlobalIDCounter() = 1; 
        g_ctx.node_stack.clear();
        g_ctx.runtime.overlay_ids.clear();

        g_ctx.store.frame_start_snapshot = g_ctx.store.persistent_states;
        g_ctx.world.backend->begin_frame(g_ctx.runtime.width, g_ctx.runtime.height);
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
        for(size_t i=0; i<elements.size(); ++i) {
            if (elements[i].parent == 0) {
                internal::RenderTree(elements[i], layouts, elements, id_map, draw_list, g_ctx.runtime, g_ctx.input_ctx, g_ctx.store, 0, 0, 0, 0, 1.0f, false);
            }
        }

        // Phase 5.1: Marquee Selection Intersection Test
        auto& input = g_ctx.input_ctx;
        static bool marquee_was_active = false;
        if (!input.marquee_active && marquee_was_active) {
            for (const auto& el : elements) {
                if (layouts.count(el.id)) {
                    const auto& l = layouts.at(el.id);
                    bool inside = (l.x >= input.marquee_rect.x && l.x + l.w <= input.marquee_rect.x + input.marquee_rect.w &&
                                   l.y >= input.marquee_rect.y && l.y + l.h <= input.marquee_rect.y + input.marquee_rect.h);
                    
                    if (inside) {
                        for (auto& store_el : g_ctx.store.elements) {
                            if (store_el.id == el.id) {
                                store_el.is_selected = true;
                                store_el.persistent.is_selected = true;
                                g_ctx.store.persistent_states[store_el.id] = store_el.persistent;
                            }
                        }
                    }
                }
            }
        }
        marquee_was_active = input.marquee_active;

        // Draw Marquee Box
        if (input.marquee_active) {
            draw_list.add_rect(
                {input.marquee_rect.x, input.marquee_rect.y}, 
                {input.marquee_rect.w, input.marquee_rect.h}, 
                {0.2f, 0.4f, 1.0f, 0.2f}, 
                10.0f
            );
            float mx = input.marquee_rect.x, my = input.marquee_rect.y;
            float mw = input.marquee_rect.w, mh = input.marquee_rect.h;
            fanta::internal::ColorF deco = {0.4f, 0.6f, 1.0f, 0.8f};
            draw_list.add_line({mx, my}, {mx+mw, my}, 1.0f, deco);
            draw_list.add_line({mx+mw, my}, {mx+mw, my+mh}, 1.0f, deco);
            draw_list.add_line({mx+mw, my+mh}, {mx, my+mh}, 1.0f, deco);
            draw_list.add_line({mx, my+mh}, {mx, my}, 1.0f, deco);
        }

        // Phase 5.2: Ghost Wire
        if (g_ctx.input_ctx.is_dragging_wire) {
            auto& runtime = g_ctx.runtime;
            auto start_id = g_ctx.input_ctx.wire_start_id;
            if (runtime.layout_results.count(start_id)) {
                const auto& layout = runtime.layout_results.at(start_id);
                float x0 = layout.x + layout.w * 0.5f;
                float y0 = layout.y + layout.h * 0.5f;
                float x3 = g_ctx.input_ctx.mouse_x;
                float y3 = g_ctx.input_ctx.mouse_y;
                float cx = (x0 + x3) * 0.5f;
                draw_list.add_bezier({x0, y0}, {cx, y0}, {cx, y3}, {x3, y3}, 2.0f, {1.0f, 1.0f, 1.0f, 0.5f});
            }
        }

        for (auto id : g_ctx.runtime.overlay_ids) {
            auto it = id_map.find(id);
            if (it != id_map.end()) {
                internal::RenderTree(elements[it->second], layouts, elements, id_map, draw_list, g_ctx.runtime, g_ctx.input_ctx, g_ctx.store, 0, 0, 0, 0, 1.0f, true);
            }
        }

        g_ctx.world.backend->render(draw_list);
        
        // Interaction Undo Snapshot
        static bool mouse_was_down = false;
        static ID last_focused_id = {};
        
        bool mouse_just_released = mouse_was_down && !g_ctx.input_ctx.mouse_down;
        bool focus_lost = !last_focused_id.is_empty() && g_ctx.input.focused_id.is_empty();
        bool focus_changed = !g_ctx.input.focused_id.is_empty() && g_ctx.input.focused_id != last_focused_id;
        bool enter_pressed = IsKeyJustPressed(257); 
        
        if (mouse_just_released || focus_lost || focus_changed || enter_pressed) {
            internal::StateDelta delta;
            bool changed = false;
            for (auto const& [id, current_data] : g_ctx.store.persistent_states) {
                auto it = g_ctx.store.frame_start_snapshot.find(id);
                if (it != g_ctx.store.frame_start_snapshot.end() && !(it->second == current_data)) {
                    delta.changes[id] = it->second;
                    changed = true;
                }
            }
            if (changed) {
                g_ctx.store.undo_stack.push_back(delta);
                g_ctx.store.redo_stack.clear(); 
                if (g_ctx.store.undo_stack.size() > 100) g_ctx.store.undo_stack.erase(g_ctx.store.undo_stack.begin());
            }
        }
        mouse_was_down = g_ctx.input_ctx.mouse_down;
        
        // Mobile Keyboard
        if (focus_changed || focus_lost) {
            bool show_keyboard = false;
            if (!g_ctx.input.focused_id.is_empty()) {
                for (const auto& el : g_ctx.store.frame_elements) {
                    if (el.id == g_ctx.input.focused_id) {
                        if (el.is_text_input || el.accessibility_role == internal::AccessibilityRole::TextInput) {
                            show_keyboard = true;
                            if (g_ctx.runtime.layout_results.count(el.id)) {
                                    const auto& l = g_ctx.runtime.layout_results.at(el.id);
                                    Native::SetIMEPos(l.x, l.y + l.h);
                            }
                        }
                        break;
                    }
                }
            }
            if (show_keyboard) Native::ShowKeyboard(KeyboardType::Default);
            else Native::HideKeyboard();
        }

        last_focused_id = g_ctx.input.focused_id;

        // Accessibility Snapshot
        g_ctx.store.accessibility_snapshot.items.clear();
        for (auto const& el : g_ctx.store.frame_elements) {
            if (el.accessibility_role == internal::AccessibilityRole::Default && !el.has_visible_label) 
                continue; 
            internal::AccessibilityItem item;
            item.id = el.id;
            item.role = el.accessibility_role;
            item.name = el.accessible_name.empty() ? el.label : el.accessible_name;
            if (g_ctx.runtime.layout_results.count(el.id)) {
                auto const& lay = g_ctx.runtime.layout_results.at(el.id);
                item.x = lay.x; item.y = lay.y; item.w = lay.w; item.h = lay.h;
            }
            g_ctx.store.accessibility_snapshot.items.push_back(item);
        }

        if (g_ctx.runtime.screenshot_pending) {
            if (g_ctx.world.backend) {
                g_ctx.world.backend->capture_screenshot(g_ctx.runtime.screenshot_filename.c_str());
            }
            g_ctx.runtime.screenshot_pending = false;
        }
        
        g_ctx.world.backend->end_frame();

        // Phase 5.1: Persistence Lifecycle
        // Copy frame elements to the main store so input logic (which runs at start of frame)
        // can interact with the elements drawn in the previous frame.
        g_ctx.store.elements = g_ctx.store.frame_elements;
    }

    bool Run(std::function<void()> app_loop) {
        using Clock = std::chrono::high_resolution_clock;
        auto last_time = Clock::now();
        
        try {
            while(g_ctx.world.backend->is_running()) {
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

    void SetDebugHitTest(bool enable) {
        g_ctx.runtime.debug_hit_test = enable;
    }

    void Render(Node* root) {
        Lower(root);
    }

    // --- Input Queries ---
    void GetMousePos(float& x, float& y) {
        x = g_ctx.input_ctx.mouse_x;
        y = g_ctx.input_ctx.mouse_y;
    }

    bool IsMouseDown() {
        return g_ctx.input_ctx.mouse_down;
    }

    // Helper (Internal or Public?)
    bool IsMouseJustPressed() {
        return g_ctx.input_ctx.state.is_just_pressed(); // Access internal state directly?
        // InputContext def is in contexts_internal.hpp
    }

    bool IsKeyDown(int key) {
#ifdef _WIN32
        return (GetAsyncKeyState(key) & 0x8000) != 0;
#else
        if (!g_ctx.world.backend) return false;
        return false; 
#endif
    }
    
    bool IsKeyJustPressed(int key) {
        return IsKeyDown(key); 
    }

    bool IsClicked(ID id) {
        return g_ctx.input_ctx.state.is_just_pressed() && g_ctx.input_ctx.focused_id == id;
    }

    std::vector<std::pair<ID, ID>> GetConnectionEvents() {
        auto events = g_ctx.input_ctx.connection_events;
        g_ctx.input_ctx.connection_events.clear(); 
        return events;
    }

    // Legacy Element Queries (kept for now if useful for debugging, or remove?)
    // User said "Discard Compatibility".
    // IsDown/IsSelected rely on linear search of Elements.
    // If we use them in v5_demo (unlikely), keep them.
    // If wire_demo uses them (yes), but wire_demo is disabled.
    // Let's keep them ONLY if they are generally useful.
    bool IsDown(ID id) {
        for (const auto& el : g_ctx.store.elements) {
            if (el.id == id) return el.is_active;
        }
        return false;
    }

    bool IsSelected(ID id) {
        for (const auto& el : g_ctx.store.elements) {
            if (el.id == id) return el.is_selected;
        }
        return false;
    }
    
    Rect GetLayout(ID id) {
        if (g_ctx.runtime.layout_results.count(id)) {
            const auto& l = g_ctx.runtime.layout_results.at(id);
            return {l.x, l.y, l.w, l.h};
        }
        return {0,0,0,0};
    }
    
    // Capture
    void Capture(ID id) { g_ctx.input_ctx.captured_id = id; }
    void Release() { g_ctx.input_ctx.captured_id = ID{}; }
    bool IsCaptured(ID id) { return g_ctx.input_ctx.captured_id == id; }
    bool IsAnyCaptured() { return g_ctx.input_ctx.captured_id.value != 0; }

} // namespace fanta
