#include "win32_utils.hpp"
#include <windows.h>
#include <commdlg.h>
#include <string>

namespace platform {
    // Existing OpenFileDialog (Simplified mock or real)
    bool Win32_OpenFileDialog(const char* title, const char* filter, std::string& out_path) {
        OPENFILENAMEA ofn;
        char szFile[260];
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            out_path = ofn.lpstrFile;
            return true;
        }
        return false;
    }

    void Win32_SetClipboardText(const char* text) {
        if (!OpenClipboard(NULL)) return;
        EmptyClipboard();
        int len = strlen(text) + 1;
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
        if (hMem) {
            memcpy(GlobalLock(hMem), text, len);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
        CloseClipboard();
    }

    std::string Win32_GetClipboardText() {
        if (!OpenClipboard(NULL)) return "";
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData == NULL) {
            CloseClipboard();
            return "";
        }
        char* pszText = static_cast<char*>(GlobalLock(hData));
        std::string result = pszText ? pszText : "";
        GlobalUnlock(hData);
        CloseClipboard();
        return result;
    }
}
