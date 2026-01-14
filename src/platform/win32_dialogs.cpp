#include <fanta.h>
#include <windows.h>
#include <shobjidl.h>
#include <string>
#include <iostream>

namespace fanta {

static std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide[0], (int)wide.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wide[0], (int)wide.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

static std::string ShowFileDialog(CLSID clsid, const char* filter, const char* title, bool folder_mode) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    std::string result = "";

    IFileDialog* pDialog = NULL;
    hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));

    if (SUCCEEDED(hr)) {
        if (title) {
            std::wstring wtitle(title, title + strlen(title));
            pDialog->SetTitle(wtitle.c_str());
        }

        if (folder_mode) {
            DWORD dwOptions;
            pDialog->GetOptions(&dwOptions);
            pDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        // TODO: Implement file filters logic if needed

        hr = pDialog->Show(NULL); // Null parent for now

        if (SUCCEEDED(hr)) {
            IShellItem* pItem;
            hr = pDialog->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr)) {
                    result = WideToUtf8(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pDialog->Release();
    }

    // CoUninitialize(); // Might break other things if called too aggressively
    return result;
}

std::string OpenFileDialog(const char* filter, const char* title) {
    return ShowFileDialog(CLSID_FileOpenDialog, filter, title, false);
}

std::string SaveFileDialog(const char* filter, const char* title) {
    return ShowFileDialog(CLSID_FileSaveDialog, filter, title, false);
}

std::string OpenFolderDialog(const char* title) {
    return ShowFileDialog(CLSID_FileOpenDialog, nullptr, title, true);
}

} // namespace fanta
