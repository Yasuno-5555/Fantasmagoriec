#include "ime_win32.hpp"

#pragma comment(lib, "imm32.lib")

std::string to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int sz = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string r(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), r.data(), sz, NULL, NULL);
    return r;
}

void ImeHandler::init(HWND h, InputDispatcher* input) {
    hwnd = h;
    input_dispatcher = input;
}

bool ImeHandler::handle_message(UINT msg, WPARAM wp, LPARAM lp, TextBuffer& buf) {
    if (msg == WM_IME_STARTCOMPOSITION) { composing = true; return false; }
    if (msg == WM_IME_ENDCOMPOSITION) { composing = false; buf.clear_preedit(); return false; }
    if (msg == WM_IME_COMPOSITION) {
        HIMC himc = ImmGetContext(hwnd);
        if (!himc) return false;
        if (lp & GCS_RESULTSTR) {
            DWORD sz = ImmGetCompositionStringW(himc, GCS_RESULTSTR, NULL, 0);
            if (sz > 0) {
                std::vector<wchar_t> b(sz / sizeof(wchar_t));
                ImmGetCompositionStringW(himc, GCS_RESULTSTR, b.data(), sz);
                if(input_dispatcher) input_dispatcher->push_text(to_utf8({b.begin(), b.end()}));
            }
        }
        if (lp & GCS_COMPSTR) {
            DWORD sz = ImmGetCompositionStringW(himc, GCS_COMPSTR, NULL, 0);
            if (sz > 0) {
                std::vector<wchar_t> b(sz / sizeof(wchar_t));
                ImmGetCompositionStringW(himc, GCS_COMPSTR, b.data(), sz);
                buf.set_preedit(to_utf8({b.begin(), b.end()}));
            } else { buf.clear_preedit(); }
        }
        ImmReleaseContext(hwnd, himc);
        return true;
    }
    if (msg == WM_CHAR && wp >= 32 && wp != 127) {
        if(input_dispatcher) input_dispatcher->push_text(to_utf8(std::wstring(1, (wchar_t)wp)));
        return true;
    }
    return false;
}
