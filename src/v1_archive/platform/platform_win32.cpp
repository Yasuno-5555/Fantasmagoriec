#include "interface.hpp"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <string>

namespace platform {
    bool OpenFileDialog(const char* title, const char* filter, std::string& out_path) {
        OPENFILENAMEA ofn;
        char szFile[260] = {0};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL; // Can be linked to window handle later
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter ? filter : "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        ofn.lpstrTitle = title;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            out_path = ofn.lpstrFile;
            return true;
        }
        return false;
    }

    void SetClipboardText(const char* text) {
        if (!OpenClipboard(NULL)) return;
        EmptyClipboard();
        int len = strlen(text);
        HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, len + 1);
        if (h) {
            void* p = GlobalLock(h);
            memcpy(p, text, len + 1);
            GlobalUnlock(h);
            SetClipboardData(CF_TEXT, h);
        }
        CloseClipboard();
    }

    std::string GetClipboardText() {
        if (!OpenClipboard(NULL)) return "";
        HANDLE h = GetClipboardData(CF_TEXT);
        std::string result;
        if (h) {
             char* p = (char*)GlobalLock(h);
             if(p) result = p;
             GlobalUnlock(h);
        }
        CloseClipboard();
        return result;
    }

    void SetImePosition(int x, int y) {
        // Assume ImeHandler or direct IMM call here. 
        // For now stub integration with existing code if needed.
        // Or call `ime.set_pos` if we had access to app instance? 
        // This is low level function.
        // We can use ImmSetCompositionWindow if we have HWND.
        // Stub for now.
    }
    
    bool IsTouchDevice() {
        return false; // Desktop
    }
}
#endif
