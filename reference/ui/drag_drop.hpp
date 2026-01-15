#pragma once

#include "../core/types_internal.hpp"
#include <string>
#include <vector>
#include <any>

namespace fanta {
namespace internal {

// Drag Payload (can carry any data)
struct DragPayload {
    std::string type;           // Payload type identifier
    std::any data;              // Actual payload data
    ID source_id{};             // Source element ID
    bool is_external = false;   // From OS file drop
    
    template<typename T>
    T* get() {
        try {
            return std::any_cast<T>(&data);
        } catch (...) {
            return nullptr;
        }
    }
    
    template<typename T>
    void set(const std::string& type_name, const T& value) {
        type = type_name;
        data = value;
    }
};

// Drag & Drop State
struct DragDropState {
    bool is_dragging = false;
    DragPayload payload;
    Vec2 drag_start = {0, 0};
    Vec2 drag_current = {0, 0};
    ID drop_target{};
    
    // OS file drops
    std::vector<std::string> pending_file_drops;
    
    void begin_drag(ID source, const DragPayload& p, float x, float y) {
        is_dragging = true;
        payload = p;
        payload.source_id = source;
        drag_start = {x, y};
        drag_current = {x, y};
    }
    
    void update_drag(float x, float y) {
        drag_current = {x, y};
    }
    
    void end_drag() {
        is_dragging = false;
        payload = DragPayload{};
        drop_target = ID{};
    }
    
    void add_file_drop(const std::string& path) {
        pending_file_drops.push_back(path);
    }
    
    std::vector<std::string> consume_file_drops() {
        auto result = std::move(pending_file_drops);
        pending_file_drops.clear();
        return result;
    }
};

// Drag & Drop Logic
struct DragDropLogic {
    static void Update(InputContext& input, RuntimeState& runtime);
    static bool IsDragActive();
    static const DragPayload* GetPayload();
    static bool AcceptsPayload(const std::string& type);
};

// Global drag/drop state accessor
DragDropState& GetDragDrop();

} // namespace internal
} // namespace fanta
