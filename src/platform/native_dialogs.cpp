#include "native_dialogs.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "ole32.lib")

namespace fanta {

std::string OpenFileDialog(const char* filter, const char* title) {
    char filename[MAX_PATH] = {0};
    
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter ? filter : "All Files\0*.*\0";
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
    return "";
}

std::string SaveFileDialog(const char* filter, const char* title) {
    char filename[MAX_PATH] = {0};
    
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter ? filter : "All Files\0*.*\0";
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    
    if (GetSaveFileNameA(&ofn)) {
        return std::string(filename);
    }
    return "";
}

std::string OpenFolderDialog(const char* title) {
    // Use IFileDialog for modern folder picker
    std::string result;
    
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileDialog* pfd = nullptr;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, 
                              IID_IFileDialog, reinterpret_cast<void**>(&pfd));
        
        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            pfd->GetOptions(&dwOptions);
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
            
            if (title) {
                wchar_t wTitle[256];
                MultiByteToWideChar(CP_UTF8, 0, title, -1, wTitle, 256);
                pfd->SetTitle(wTitle);
            }
            
            hr = pfd->Show(nullptr);
            if (SUCCEEDED(hr)) {
                IShellItem* psi = nullptr;
                hr = pfd->GetResult(&psi);
                if (SUCCEEDED(hr)) {
                    PWSTR pszPath = nullptr;
                    hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                    if (SUCCEEDED(hr)) {
                        char path[MAX_PATH];
                        WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, path, MAX_PATH, nullptr, nullptr);
                        result = path;
                        CoTaskMemFree(pszPath);
                    }
                    psi->Release();
                }
            }
            pfd->Release();
        }
        CoUninitialize();
    }
    
    return result;
}

} // namespace fanta

#else
// Linux/macOS stub - TODO: implement with zenity/kdialog or native APIs

namespace fanta {

std::string OpenFileDialog(const char* filter, const char* title) {
    (void)filter; (void)title;
    return ""; // Not implemented
}

std::string SaveFileDialog(const char* filter, const char* title) {
    (void)filter; (void)title;
    return ""; // Not implemented
}

std::string OpenFolderDialog(const char* title) {
    (void)title;
    return ""; // Not implemented
}

} // namespace fanta

#endif
