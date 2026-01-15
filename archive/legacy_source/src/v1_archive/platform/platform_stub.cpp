#include "interface.hpp"

#ifndef _WIN32
// Stub implementation for Linux/Mac/Mobile (until properly implemented)

namespace platform {
    bool OpenFileDialog(const char* title, const char* filter, std::string& out_path) {
        // Mobile/Web often doesn't have "Open File Dialog" sync API.
        // It's usually async.
        // For now, return false (not handled) or mock path.
        return false; 
    }
    
    void SetClipboardText(const char* text) {
        // Log?
    }
    
    std::string GetClipboardText() {
        return "";
    }
    
    void SetImePosition(int x, int y) {}
    
    bool IsTouchDevice() {
        return true; // Assume non-Win32 might be mobile in this project context
    }
}
#endif
