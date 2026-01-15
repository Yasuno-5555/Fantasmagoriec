#pragma once
#include <string>

// Defines the application state structure.
// In the future this could be generic or template-based, but for now specific.

struct AppState {
    // User data
    std::string user_name;
    int click_count = 0;
    
    // IME / Input buffers
    std::string ime_buffer; // UTF-8
    int caret_pos = 0;
};
