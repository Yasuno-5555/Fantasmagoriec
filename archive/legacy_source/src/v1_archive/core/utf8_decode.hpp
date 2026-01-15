#pragma once
#include <cstdint>

// Simple UTF-8 Decoder
// Returns Codepoint and advances pointer
inline uint32_t DecodeNextUTF8(const char*& str) {
    uint32_t c = (unsigned char)*str;
    if (c == 0) return 0;
    
    str++;
    if (c < 0x80) return c; // ASCII
    
    uint32_t result = 0;
    int extra = 0;
    
    if ((c & 0xE0) == 0xC0) {
        result = c & 0x1F;
        extra = 1;
    } else if ((c & 0xF0) == 0xE0) {
        result = c & 0x0F;
        extra = 2;
    } else if ((c & 0xF8) == 0xF0) {
        result = c & 0x07;
        extra = 3;
    } else {
        return '?'; // Error
    }
    
    for (int i=0; i<extra; ++i) {
        uint32_t n = (unsigned char)*str;
        if ((n & 0xC0) != 0x80) return '?'; // Error
        result = (result << 6) | (n & 0x3F);
        str++;
    }
    return result;
}
