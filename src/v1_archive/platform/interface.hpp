#pragma once
#include <string>

namespace platform {
    // Dialogs
    bool OpenFileDialog(const char* title, const char* filter, std::string& out_path);
    
    // System Integration
    void SetClipboardText(const char* text);
    std::string GetClipboardText();
    void SetImePosition(int x, int y);
    
    // Device Capabilities
    bool IsTouchDevice(); // True on iOS/Android, usually false on Desktop
}
