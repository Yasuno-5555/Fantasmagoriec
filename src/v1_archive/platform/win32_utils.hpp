#pragma once
#include <string>

namespace platform {
    bool Win32_OpenFileDialog(const char* title, const char* filter, std::string& out_path);
    
    // Clipboard
    void Win32_SetClipboardText(const char* text);
    std::string Win32_GetClipboardText();
}
