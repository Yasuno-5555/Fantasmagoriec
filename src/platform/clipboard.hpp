#pragma once

#include <string>

namespace fanta {

// Clipboard Operations (using GLFW internally)

// Set clipboard text content
void SetClipboardText(const char* text);

// Get clipboard text content
// Returns empty string if clipboard is empty or not text
std::string GetClipboardText();

// Check if clipboard has text content
bool HasClipboardText();

} // namespace fanta
