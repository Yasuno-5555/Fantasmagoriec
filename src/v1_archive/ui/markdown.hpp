#pragma once
#include <string>
#include <vector>
#include <regex>

namespace ui {

    struct TextSpan {
        std::string text;
        bool bold = false;
        bool italic = false;
        bool is_link = false;
        bool is_heading = false;
        int heading_level = 0; // 1 for #, 2 for ##
        std::string link_url;
    };

    inline std::vector<TextSpan> ParseMarkdown(const std::string& input) {
        std::vector<TextSpan> spans;
        
        // Very basic parser: Scan linearly or use regex?
        // Regex is easier for [link](url) but harder for nested state (bold inside italic).
        // Let's implement a simple state machine scanner.
        
        // For simplicity in Phase 1, we handle tags sequentially and non-nested for now, 
        // or just prioritize one.
        // Actually, let's just use strict regex replacement logic for tokenizing.
        
        // Regex approach:
        // Tokenize by special chars: *, [, ]
        
        // Better: Simple scanner
        size_t i = 0;
        size_t n = input.size();
        
        bool in_bold = false;
        bool in_italic = false;
        
        std::string buffer;
        
        auto flush = [&](bool force=false) {
            if (!buffer.empty()) {
                spans.push_back({buffer, in_bold, in_italic, false, false, 0, ""});
                buffer.clear();
            }
        };
        
        // heading check (at start)
        int h_level = 0;
        if (input.rfind("# ", 0) == 0) { h_level = 1; i=2; }
        else if (input.rfind("## ", 0) == 0) { h_level = 2; i=3; }
        
        if (h_level > 0) {
            // Treat whole line as heading for now? Or just styled?
            // Markdown usually implies block level.
            // Let's parse the rest but mark spans as heading too? 
            // Or just return one big heading span for simplicity if no formatting inside.
        }

        while (i < n) {
            // Check for **
            if (i + 1 < n && input[i] == '*' && input[i+1] == '*') {
                flush();
                in_bold = !in_bold;
                i += 2;
                continue;
            }
            // Check for *
            if (input[i] == '*') {
                flush();
                in_italic = !in_italic;
                i += 1;
                continue;
            }
            // Check for link [text](url)
            if (input[i] == '[') {
                // Scan ahead for ](
                size_t close_bracket = input.find("](", i);
                if (close_bracket != std::string::npos) {
                    size_t close_paren = input.find(")", close_bracket);
                    if (close_paren != std::string::npos) {
                        flush();
                        std::string link_text = input.substr(i+1, close_bracket - (i+1));
                        std::string link_url = input.substr(close_bracket+2, close_paren - (close_bracket+2));
                        
                        TextSpan span;
                        span.text = link_text;
                        span.is_link = true;
                        span.link_url = link_url;
                        span.bold = in_bold;
                        span.italic = in_italic;
                        span.is_heading = (h_level > 0);
                        span.heading_level = h_level;
                        spans.push_back(span);
                        
                        i = close_paren + 1;
                        continue;
                    }
                }
            }
            
            buffer += input[i];
            i++;
        }
        flush();
        
        // Apply heading level to all if set (simple logic)
        if (h_level > 0) {
            for(auto& s : spans) {
                s.is_heading = true;
                s.heading_level = h_level;
            }
        }
        
        return spans;
    }
}
