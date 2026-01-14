#pragma once

#include "../core/types_internal.hpp"
#include <string>
#include <vector>

namespace fanta {
namespace internal {

// IME Candidate
struct IMECandidate {
    std::string text;
    std::string annotation;  // Reading/pronunciation hint
};

// IME State
struct IMEState {
    bool active = false;
    bool composing = false;
    
    // Composition string (preedit)
    std::string composition;
    int cursor_in_composition = 0;
    
    // Candidate window
    std::vector<IMECandidate> candidates;
    int selected_candidate = 0;
    bool show_candidates = false;
    
    // Position for candidate window
    Vec2 candidate_window_pos = {0, 0};
    
    void start_composition() {
        composing = true;
        composition.clear();
        cursor_in_composition = 0;
        candidates.clear();
    }
    
    void update_composition(const std::string& text, int cursor) {
        composition = text;
        cursor_in_composition = cursor;
    }
    
    void set_candidates(const std::vector<IMECandidate>& cands) {
        candidates = cands;
        selected_candidate = 0;
        show_candidates = !candidates.empty();
    }
    
    void commit() {
        composing = false;
        composition.clear();
        candidates.clear();
        show_candidates = false;
    }
    
    void cancel() {
        composing = false;
        composition.clear();
        candidates.clear();
        show_candidates = false;
    }
    
    void select_next_candidate() {
        if (!candidates.empty()) {
            selected_candidate = (selected_candidate + 1) % candidates.size();
        }
    }
    
    void select_prev_candidate() {
        if (!candidates.empty()) {
            selected_candidate = (selected_candidate - 1 + candidates.size()) % candidates.size();
        }
    }
    
    std::string get_committed_text() const {
        if (!candidates.empty() && selected_candidate < static_cast<int>(candidates.size())) {
            return candidates[selected_candidate].text;
        }
        return composition;
    }
};

// IME Logic
struct IMELogic {
    static void Update(IMEState& state, InputContext& input);
    static void Render(const IMEState& state, DrawList& dl);
    static void HandlePlatformEvent(IMEState& state, void* event);  // Platform-specific
};

// Global IME accessor
IMEState& GetIME();

// Platform-specific IME initialization
void InitializeIME();
void ShutdownIME();

} // namespace internal
} // namespace fanta
