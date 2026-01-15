#include "ime.hpp"
#include "../core/contexts_internal.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <imm.h>
#pragma comment(lib, "imm32.lib")
#endif

namespace fanta {
namespace internal {

static IMEState g_ime;

IMEState& GetIME() {
    return g_ime;
}

void IMELogic::Update(IMEState& state, InputContext& input) {
    // Update candidate window position based on focused element
    if (state.composing) {
        // Position candidate window near the text cursor
        // This would need to be set by the focused TextInput
        (void)input;
    }
}

void IMELogic::Render(const IMEState& state, DrawList& dl) {
    if (!state.show_candidates || state.candidates.empty()) return;
    
    float x = state.candidate_window_pos.x;
    float y = state.candidate_window_pos.y;
    float item_height = 28.0f;
    float width = 200.0f;
    float padding = 8.0f;
    float height = state.candidates.size() * item_height + padding * 2;
    
    // Background
    ColorF bg = {0.15f, 0.15f, 0.15f, 0.98f};
    ColorF selected_bg = {0.3f, 0.3f, 0.5f, 1.0f};
    ColorF text = {1.0f, 1.0f, 1.0f, 1.0f};
    
    dl.add_rounded_rect({x, y}, {width, height}, 4.0f, bg, 8.0f);
    
    // Candidates
    for (size_t i = 0; i < state.candidates.size(); i++) {
        float item_y = y + padding + i * item_height;
        
        if (static_cast<int>(i) == state.selected_candidate) {
            dl.add_rect({x + 4, item_y}, {width - 8, item_height - 2}, selected_bg, 0);
        }
        
        // TODO: Render candidate text
        // Format: "1. 候裁E
        (void)text;
    }
}

void IMELogic::HandlePlatformEvent(IMEState& state, void* event) {
    // Platform-specific event handling would go here
    (void)state;
    (void)event;
}

void InitializeIME() {
#ifdef _WIN32
    // Windows IME initialization is automatic
#endif
}

void ShutdownIME() {
#ifdef _WIN32
    // Nothing special needed
#endif
}

} // namespace internal
} // namespace fanta
