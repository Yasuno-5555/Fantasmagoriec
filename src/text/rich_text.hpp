#pragma once
#include "types_internal.hpp"
#include <string>
#include <vector>

namespace fanta {
namespace internal {

    struct TextSpan {
        std::string text;
        ColorF color;
        float font_size = 16.0f;
        TextToken token = TextToken::None;
        bool bold = false;
        bool italic = false;
    };

    struct RichText {
        std::vector<TextSpan> spans;
        
        static RichText Parse(const std::string& markup); // Supports basic tag like <color=#FF0000>text</color>
    };

} // namespace internal
} // namespace fanta
