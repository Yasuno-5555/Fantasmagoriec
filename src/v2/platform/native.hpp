// Fantasmagorie v2 - Native Platform APIs
// Cross-platform native dialogs and system integration
#pragma once

#include <string>
#include <functional>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#endif

namespace fanta {
namespace native {

// ============================================================================
// File Dialogs
// ============================================================================

inline std::string open_file_dialog(const char* title, const char* filter = "*.*") {
#ifdef _WIN32
    char filename[MAX_PATH] = {0};
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrTitle = title;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
#endif
    return "";
}

inline std::string save_file_dialog(const char* title, const char* filter = "*.*") {
#ifdef _WIN32
    char filename[MAX_PATH] = {0};
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrTitle = title;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    
    if (GetSaveFileNameA(&ofn)) {
        return std::string(filename);
    }
#endif
    return "";
}

// ============================================================================
// Message Boxes
// ============================================================================

enum class MessageBoxType { Info, Warning, Error, Question };

inline int show_message(const char* title, const char* message, MessageBoxType type = MessageBoxType::Info) {
#ifdef _WIN32
    UINT flags = MB_OK;
    switch (type) {
        case MessageBoxType::Info: flags |= MB_ICONINFORMATION; break;
        case MessageBoxType::Warning: flags |= MB_ICONWARNING; break;
        case MessageBoxType::Error: flags |= MB_ICONERROR; break;
        case MessageBoxType::Question: flags = MB_YESNO | MB_ICONQUESTION; break;
    }
    return MessageBoxA(nullptr, message, title, flags);
#endif
    return 0;
}

// ============================================================================
// System Actions
// ============================================================================

inline void open_url(const char* url) {
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
#endif
}

inline void open_folder(const char* path) {
#ifdef _WIN32
    ShellExecuteA(nullptr, "explore", path, nullptr, nullptr, SW_SHOWNORMAL);
#endif
}

// ============================================================================
// Clipboard
// ============================================================================

inline void set_clipboard(const char* text) {
#ifdef _WIN32
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        size_t len = strlen(text) + 1;
        HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len);
        if (mem) {
            memcpy(GlobalLock(mem), text, len);
            GlobalUnlock(mem);
            SetClipboardData(CF_TEXT, mem);
        }
        CloseClipboard();
    }
#endif
}

inline std::string get_clipboard() {
#ifdef _WIN32
    std::string result;
    if (OpenClipboard(nullptr)) {
        HANDLE data = GetClipboardData(CF_TEXT);
        if (data) {
            char* text = (char*)GlobalLock(data);
            if (text) result = text;
            GlobalUnlock(data);
        }
        CloseClipboard();
    }
    return result;
#endif
    return "";
}

// ============================================================================
// Notifications (Windows Toast - simplified)
// ============================================================================

inline void show_notification(const char* title, const char* message) {
#ifdef _WIN32
    // Simplified: use message box as fallback
    // Real implementation would use Windows.UI.Notifications
    MessageBoxA(nullptr, message, title, MB_OK | MB_ICONINFORMATION);
#endif
}

} // namespace native
} // namespace fanta
