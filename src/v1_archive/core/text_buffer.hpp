#pragma once
#include <string>

struct TextBuffer {
    std::string text;
    std::string preedit;
    void insert(const std::string& s) { text += s; }
    void remove_back() { 
        if (!text.empty()) {
             while(!text.empty() && (text.back() & 0xC0) == 0x80) text.pop_back();
             if(!text.empty()) text.pop_back();
        }
    }
    void clear_preedit() { preedit.clear(); }
    void set_preedit(const std::string& s) { preedit = s; }
};
