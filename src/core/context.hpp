#pragma once
// L4: Context Structures
// Dependencies: types_core.hpp (L1), types_fwd.hpp (L0), <memory>, <vector>, <map>
// Knows: Backend*, but NOT DrawList internals.

#include "core/types_core.hpp"
#include "core/types_fwd.hpp"
#include <memory>
#include <vector>
#include <map>
#include <cstring>
#include "core/animation.hpp"

namespace fanta {

// Forward declaration - defined in backend.hpp
class Backend;

namespace internal {

    // --- Frame Arena (Simplified) ---
    class FrameArena {
    public:
        static constexpr size_t CAPACITY = 1024 * 1024; // 1MB
        
        FrameArena() { buffer_.resize(CAPACITY); }
        
        void reset() { offset_ = 0; }
        
        template<typename T>
        T* alloc() {
            static_assert(std::is_trivially_destructible_v<T>, "FrameArena only for POD types");
            size_t aligned = (offset_ + alignof(T) - 1) & ~(alignof(T) - 1);
            if (aligned + sizeof(T) > CAPACITY) return nullptr;
            T* ptr = reinterpret_cast<T*>(buffer_.data() + aligned);
            offset_ = aligned + sizeof(T);
            return new(ptr) T();
        }

        template<typename T>
        T* alloc_array(size_t count) {
            static_assert(std::is_trivially_destructible_v<T>, "FrameArena only for POD types");
            size_t aligned = (offset_ + alignof(T) - 1) & ~(alignof(T) - 1);
            size_t size = sizeof(T) * count;
            if (aligned + size > CAPACITY) return nullptr;
            T* ptr = reinterpret_cast<T*>(buffer_.data() + aligned);
            offset_ = aligned + size;
            for (size_t i = 0; i < count; ++i) new(&ptr[i]) T();
            return ptr;
        }
        
        char* alloc_string(const char* s) {
            if (!s) return nullptr;
            size_t len = strlen(s);
            char* buf = (char*)alloc_array<char>(len + 1);
            if (buf) memcpy(buf, s, len + 1);
            return buf;
        }

    private:
        std::vector<uint8_t> buffer_;
        size_t offset_ = 0;
    };

    // --- World State (Persistent) ---
    struct WorldState {
        std::unique_ptr<Backend> backend;
        int window_width = 0;
        int window_height = 0;
        // Theme is defined elsewhere, just store index/enum here if needed
    };

    // --- Frame State (Per-Frame Transient) ---
    struct FrameState {
        FrameArena arena;
        float dt = 0.016f;
        double time = 0.0;
        std::vector<ID> id_stack;
        std::map<ID, ComputedLayout> layout_results;
        std::vector<ID> parent_stack;  // For implicit parent tracking (Philosophy VII compliant)
        
        // MenuBar state (Philosophy VII: moved from static global)
        uint64_t current_menu_bar = 0;
        uint64_t current_menu = 0;
        
        // DragDrop state (Philosophy VII: moved from static global)
        uint64_t drag_source = 0;
        void* drag_payload = nullptr;
        
        void reset() {
            arena.reset();
            id_stack.clear();
            parent_stack.clear();
            current_menu_bar = 0;
            current_menu = 0;
            drag_source = 0;
            drag_payload = nullptr;
        }
    };

    // --- Runtime State (Legacy Compat) ---
    struct RuntimeState {
        int width = 0;
        int height = 0;
        float dt = 0.016f;
        bool screenshot_pending = false;
        std::vector<ID> overlay_ids;
        std::map<ID, ComputedLayout> layout_results;
        bool debug_hit_test = false;
        std::vector<ID> id_stack;
        
        // Keyboard navigation (Philosophy VII: moved from static global)
        bool keyboard_nav_enabled = false;
    };

    // --- Input Context (FACTS ONLY) ---
    // Per Iron Philosophy XI: "Input は事実であれ、意味になるな"
    // NO interpretation methods here. No IsClicked, IsPressed, IsDragging.
    // Just raw facts from the OS.
    
    struct InputContext {
        // Mouse position (facts)
        float mouse_x = 0;
        float mouse_y = 0;
        float mouse_dx = 0;
        float mouse_dy = 0;
        
        // Mouse button state (facts)
        bool mouse_down = false;      // Current frame: is button held?
        bool mouse_was_down = false;  // Previous frame: was button held?
        
        // Wheel delta (fact)
        float mouse_wheel = 0;
        
        // Keyboard (facts)
        std::vector<uint32_t> chars;
        std::vector<int> keys;
        
        // IME (facts)
        bool ime_active = false;          // Is IME currently composing?
        std::string ime_composition;      // Current composition string (e.g. "kyou")
        int ime_cursor = 0;               // Caret position in composition
        std::string ime_result;           // Confirmed result string (e.g. "今日")
        
        // Frame update (copy current to previous)
        void advance_frame() {
            mouse_was_down = mouse_down;
            mouse_wheel = 0; // Reset per-frame delta
            chars.clear();
            keys.clear();
            ime_result.clear(); // Clear one-shot result
        }
    };

    // --- Interaction State (Ephemeral - RuntimeState) ---
    // Per Iron Philosophy XII: "RuntimeState は揮発性メモ"
    // Hover, Active, Focus - can be regenerated each frame.
    // NOT mixed with PersistentState.
    
    struct InteractionState {
        ID hot_id{};     // Currently hovered (this frame)
        ID active_id{};  // Currently pressed/captured
        ID focus_id{};   // Keyboard focus candidate
        
        void reset_frame() {
            hot_id = ID{};
            // active_id persists within frame (for drag)
            // focus_id persists within frame
        }
    };

    // --- Persistent State (Survives frames) ---
    struct PersistentState {
        // ID -> Last frame's computed rect (for 1-frame-delayed hit test)
        std::map<uint64_t, Rectangle> last_frame_rects;
        
        // ID -> Scroll offset (for scroll containers)
        std::map<uint64_t, Vec2> scroll_offsets;
        
        // ID -> Animation State
        std::map<uint64_t, AnimationState> animations;

        ID focused_id{};
        
        void store_rect(ID id, float x, float y, float w, float h) {
            last_frame_rects[id.value] = {x, y, w, h};
        }
        
        Rectangle get_rect(ID id) const {
            auto it = last_frame_rects.find(id.value);
            if (it != last_frame_rects.end()) return it->second;
            return {0, 0, 0, 0};
        }
    };

} // namespace internal
} // namespace fanta

