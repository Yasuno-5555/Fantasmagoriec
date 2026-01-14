#include "text/rich_text.hpp"
#include <sstream>

namespace fanta {
namespace internal {

RichText RichText::Parse(const std::string& markup) {
    RichText rich;
    // Simple state machine for parsing tags like <color=#RGBA>, <b>, <i>
    // For now, let's just do a naive implementation.
    
    std::string current_text;
    ColorF current_color = {1,1,1,1};
    bool bold = false;
    bool italic = false;
    
    for (size_t i = 0; i < markup.length(); ++i) {
        if (markup[i] == '<') {
            // Flush current span
            if (!current_text.empty()) {
                rich.spans.push_back({current_text, current_color, 16.0f, TextToken::None, bold, italic});
                current_text.clear();
            }
            
            size_t end = markup.find('>', i);
            if (end != std::string::npos) {
                std::string tag = markup.substr(i + 1, end - i - 1);
                if (tag == "b") bold = true;
                else if (tag == "/b") bold = false;
                else if (tag == "i") italic = true;
                else if (tag == "/i") italic = false;
                else if (tag.rfind("color=", 0) == 0) {
                    // Parse hex color #RRGGBBAA
                    std::string hex = tag.substr(6);
                    if (hex[0] == '#') hex = hex.substr(1);
                    uint32_t rgba = std::stoul(hex, nullptr, 16);
                    current_color.a = (rgba & 0xFF) / 255.0f;
                    current_color.b = ((rgba >> 8) & 0xFF) / 255.0f;
                    current_color.g = ((rgba >> 16) & 0xFF) / 255.0f;
                    current_color.r = ((rgba >> 24) & 0xFF) / 255.0f;
                } else if (tag == "/color") {
                    current_color = {1,1,1,1};
                }
                i = end;
                continue;
            }
        }
        current_text += markup[i];
    }
    
    if (!current_text.empty()) {
        rich.spans.push_back({current_text, current_color, 16.0f, TextToken::None, bold, italic});
    }
    
    return rich;
}

} // namespace internal
} // namespace fanta
