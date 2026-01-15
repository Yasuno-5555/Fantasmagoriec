#pragma once
#include <cstdint>
#include <string>

namespace fanta {
namespace internal {

    // Simple UTF-8 Decoder
    inline uint32_t NextUTF8(const char*& p) {
        if (!p || !*p) return 0;
        
        uint32_t codepoint = 0;
        unsigned char c = static_cast<unsigned char>(*p);
        
        if (c < 0x80) {
            // 1-byte
            codepoint = c;
            p++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-bytes
            codepoint = c & 0x1F;
            p++;
            if ((*p & 0xC0) == 0x80) codepoint = (codepoint << 6) | (*p++ & 0x3F);
        } else if ((c & 0xF0) == 0xE0) {
            // 3-bytes
            codepoint = c & 0x0F;
            p++;
            if ((*p & 0xC0) == 0x80) codepoint = (codepoint << 6) | (*p++ & 0x3F);
            if ((*p & 0xC0) == 0x80) codepoint = (codepoint << 6) | (*p++ & 0x3F);
        } else if ((c & 0xF8) == 0xF0) {
            // 4-bytes
            codepoint = c & 0x07;
            p++;
            if ((*p & 0xC0) == 0x80) codepoint = (codepoint << 6) | (*p++ & 0x3F);
            if ((*p & 0xC0) == 0x80) codepoint = (codepoint << 6) | (*p++ & 0x3F);
            if ((*p & 0xC0) == 0x80) codepoint = (codepoint << 6) | (*p++ & 0x3F);
        } else {
            // Invalid - skip or use replacement char
            codepoint = 0xFFFD;
            p++;
        }
        return codepoint;
    }

}
}
