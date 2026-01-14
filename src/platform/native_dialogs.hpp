#pragma once

#include <string>

namespace fanta {

// Native File Dialogs (Platform-specific implementation)

// Opens a native file open dialog
// filter: e.g. "Text Files\0*.txt\0All Files\0*.*\0"
// Returns empty string if cancelled
std::string OpenFileDialog(const char* filter = nullptr, const char* title = "Open File");

// Opens a native file save dialog
std::string SaveFileDialog(const char* filter = nullptr, const char* title = "Save File");

// Opens a native folder picker dialog
std::string OpenFolderDialog(const char* title = "Select Folder");

} // namespace fanta
