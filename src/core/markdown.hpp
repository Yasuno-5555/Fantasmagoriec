#pragma once
#include <string>
#include <vector>

namespace fanta {
namespace internal {

    struct TextSpan {
        std::string text;
        bool bold = false;
        bool italic = false;
        bool is_link = false;
        bool is_heading = false;
        int heading_level = 0;
        std::string link_url;
    };

    inline std::vector<TextSpan> ParseMarkdown(const std::string& input) {
        std::vector<TextSpan> spans;
        size_t i = 0;
        size_t n = input.size();
        
        bool in_bold = false;
        bool in_italic = false;
        std::string buffer;
        
        auto flush = [&]() {
            if (!buffer.empty()) {
                TextSpan s;
                s.text = buffer;
                s.bold = in_bold;
                s.italic = in_italic;
                spans.push_back(s);
                buffer.clear();
            }
        };

        // Heading check
        int h_level = 0;
        if (input.starts_with("## ")) { h_level = 2; i = 3; }
        else if (input.starts_with("# ")) { h_level = 1; i = 2; }

        while (i < n) {
            // Unescape
            if (input[i] == '\\' && i + 1 < n) {
                buffer += input[i+1];
                i += 2;
                continue;
            }
            // Bold **
            if (i + 1 < n && input[i] == '*' && input[i+1] == '*') {
                flush();
                in_bold = !in_bold;
                i += 2;
                continue;
            }
            // Italic *
            if (input[i] == '*') {
                flush();
                in_italic = !in_italic;
                i += 1;
                continue;
            }
            // Link [text](url)
            if (input[i] == '[') {
                size_t close_bracket = input.find("](", i);
                if (close_bracket != std::string::npos) {
                    size_t close_paren = input.find(")", close_bracket);
                    if (close_paren != std::string::npos) {
                        flush();
                        TextSpan s;
                        s.text = input.substr(i + 1, close_bracket - (i + 1));
                        s.is_link = true;
                        s.link_url = input.substr(close_bracket + 2, close_paren - (close_bracket + 2));
                        s.bold = in_bold;
                        s.italic = in_italic;
                        spans.push_back(s);
                        i = close_paren + 1;
                        continue;
                    }
                }
            }
            buffer += input[i];
            i++;
        }
        flush();

        if (h_level > 0) {
            for (auto& s : spans) {
                s.is_heading = true;
                s.heading_level = h_level;
            }
        }
        return spans;
    }

} // namespace internal
} // namespace fanta
