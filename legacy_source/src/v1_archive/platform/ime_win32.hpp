#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <imm.h>

#include "core/text_buffer.hpp"
#include "input/input_dispatch.hpp"

// Convert output string to UTF-8
std::string to_utf8(const std::wstring& wstr);

class ImeHandler {
public:
    HWND hwnd = nullptr;
    InputDispatcher* input_dispatcher = nullptr;
    bool composing = false;

    void init(HWND h, InputDispatcher* input);
    bool handle_message(UINT msg, WPARAM wp, LPARAM lp, TextBuffer& buf);
};
