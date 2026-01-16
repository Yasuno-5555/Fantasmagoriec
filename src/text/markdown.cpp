#include "text/markdown.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace fanta {
namespace markdown {

// --- Helper: Trim whitespace ---
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// --- Helper: Starts with ---
static bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

// --- Helper: Count leading chars ---
static int count_leading(const std::string& s, char c) {
    int count = 0;
    for (char ch : s) {
        if (ch == c) count++;
        else break;
    }
    return count;
}

// --- Detect Block Type ---
BlockType MarkdownParser::detect_block_type(const std::string& line, int& out_level) {
    out_level = 0;
    std::string trimmed = trim(line);
    
    if (trimmed.empty()) return BlockType::Empty;
    
    // Headings: # to ######
    if (trimmed[0] == '#') {
        int level = count_leading(trimmed, '#');
        if (level >= 1 && level <= 6 && trimmed.size() > (size_t)level && trimmed[level] == ' ') {
            out_level = level;
            return static_cast<BlockType>(static_cast<int>(BlockType::Heading1) + level - 1);
        }
    }
    
    // Horizontal Rule: ---, ***, ___
    if ((starts_with(trimmed, "---") || starts_with(trimmed, "***") || starts_with(trimmed, "___")) && 
        trimmed.find_first_not_of("-*_ ") == std::string::npos && trimmed.size() >= 3) {
        return BlockType::HorizontalRule;
    }
    
    // Bullet List: -, *, +
    if ((trimmed[0] == '-' || trimmed[0] == '*' || trimmed[0] == '+') && 
        trimmed.size() > 1 && trimmed[1] == ' ') {
        // Count leading spaces for indent
        out_level = 0;
        for (char c : line) {
            if (c == ' ') out_level++;
            else if (c == '\t') out_level += 4;
            else break;
        }
        out_level /= 2; // Rough indent level
        return BlockType::BulletList;
    }
    
    // Numbered List: 1. 2. etc.
    if (std::isdigit(trimmed[0])) {
        size_t dot_pos = trimmed.find('.');
        if (dot_pos != std::string::npos && dot_pos < 4) {
            bool all_digits = true;
            for (size_t i = 0; i < dot_pos; i++) {
                if (!std::isdigit(trimmed[i])) { all_digits = false; break; }
            }
            if (all_digits && dot_pos + 1 < trimmed.size() && trimmed[dot_pos + 1] == ' ') {
                return BlockType::NumberedList;
            }
        }
    }
    
    // Code Block (fenced): ```
    if (starts_with(trimmed, "```")) {
        return BlockType::CodeBlock;
    }
    
    // Block Quote: >
    if (trimmed[0] == '>') {
        return BlockType::BlockQuote;
    }
    
    return BlockType::Paragraph;
}

// --- Trim Block Prefix ---
std::string MarkdownParser::trim_block_prefix(const std::string& line, BlockType type, int level) {
    std::string trimmed = trim(line);
    
    switch (type) {
        case BlockType::Heading1:
        case BlockType::Heading2:
        case BlockType::Heading3:
        case BlockType::Heading4:
        case BlockType::Heading5:
        case BlockType::Heading6:
            return trimmed.substr(level + 1); // Skip "# " or "## " etc.
        
        case BlockType::BulletList:
            return trimmed.substr(2); // Skip "- " or "* "
        
        case BlockType::NumberedList: {
            size_t dot_pos = trimmed.find('.');
            if (dot_pos != std::string::npos && dot_pos + 2 <= trimmed.size()) {
                return trimmed.substr(dot_pos + 2); // Skip "1. "
            }
            return trimmed;
        }
        
        case BlockType::BlockQuote:
            return trimmed.size() > 1 ? trim(trimmed.substr(1)) : "";
        
        default:
            return trimmed;
    }
}

// --- Parse Inline Styles ---
std::vector<InlineSpan> MarkdownParser::parse_inline(const std::string& text) {
    std::vector<InlineSpan> spans;
    
    // Simple state machine for bold (**), italic (*), code (`), links ([](url))
    std::string current_text;
    InlineStyle current_style = InlineStyle::None;
    
    size_t i = 0;
    while (i < text.size()) {
        // Bold: **text**
        if (i + 1 < text.size() && text[i] == '*' && text[i+1] == '*') {
            if (!current_text.empty()) {
                spans.push_back({current_text, current_style});
                current_text.clear();
            }
            // Toggle bold
            if ((static_cast<uint8_t>(current_style) & static_cast<uint8_t>(InlineStyle::Bold)) != 0) {
                current_style = static_cast<InlineStyle>(static_cast<uint8_t>(current_style) & ~static_cast<uint8_t>(InlineStyle::Bold));
            } else {
                current_style = current_style | InlineStyle::Bold;
            }
            i += 2;
            continue;
        }
        
        // Italic: *text* (single asterisk, not followed by another)
        if (text[i] == '*' && (i + 1 >= text.size() || text[i+1] != '*')) {
            if (!current_text.empty()) {
                spans.push_back({current_text, current_style});
                current_text.clear();
            }
            // Toggle italic
            if ((static_cast<uint8_t>(current_style) & static_cast<uint8_t>(InlineStyle::Italic)) != 0) {
                current_style = static_cast<InlineStyle>(static_cast<uint8_t>(current_style) & ~static_cast<uint8_t>(InlineStyle::Italic));
            } else {
                current_style = current_style | InlineStyle::Italic;
            }
            i += 1;
            continue;
        }
        
        // Code: `text`
        if (text[i] == '`') {
            if (!current_text.empty()) {
                spans.push_back({current_text, current_style});
                current_text.clear();
            }
            // Find closing backtick
            size_t end = text.find('`', i + 1);
            if (end != std::string::npos) {
                spans.push_back({text.substr(i + 1, end - i - 1), InlineStyle::Code});
                i = end + 1;
                continue;
            }
        }
        
        // Link: [text](url)
        if (text[i] == '[') {
            size_t close_bracket = text.find(']', i);
            if (close_bracket != std::string::npos && close_bracket + 1 < text.size() && text[close_bracket + 1] == '(') {
                size_t close_paren = text.find(')', close_bracket + 2);
                if (close_paren != std::string::npos) {
                    if (!current_text.empty()) {
                        spans.push_back({current_text, current_style});
                        current_text.clear();
                    }
                    std::string link_text = text.substr(i + 1, close_bracket - i - 1);
                    std::string link_url = text.substr(close_bracket + 2, close_paren - close_bracket - 2);
                    InlineSpan link_span;
                    link_span.text = link_text;
                    link_span.style = InlineStyle::Link;
                    link_span.link_url = link_url;
                    spans.push_back(link_span);
                    i = close_paren + 1;
                    continue;
                }
            }
        }
        
        current_text += text[i];
        i++;
    }
    
    if (!current_text.empty()) {
        spans.push_back({current_text, current_style});
    }
    
    // If no spans, add empty paragraph placeholder
    if (spans.empty()) {
        spans.push_back({"", InlineStyle::None});
    }
    
    return spans;
}

// --- Main Parse ---
MarkdownDocument MarkdownParser::parse(const std::string& source) {
    MarkdownDocument doc;
    std::istringstream stream(source);
    std::string line;
    
    bool in_code_block = false;
    std::string code_language;
    std::string code_content;
    
    while (std::getline(stream, line)) {
        // Handle Code Blocks
        if (starts_with(trim(line), "```")) {
            if (!in_code_block) {
                // Start code block
                in_code_block = true;
                code_language = trim(line).substr(3);
                code_content.clear();
            } else {
                // End code block
                in_code_block = false;
                MarkdownBlock block;
                block.type = BlockType::CodeBlock;
                block.code_language = code_language;
                block.spans.push_back({code_content, InlineStyle::Code});
                doc.blocks.push_back(block);
            }
            continue;
        }
        
        if (in_code_block) {
            if (!code_content.empty()) code_content += "\n";
            code_content += line;
            continue;
        }
        
        // Detect block type
        int level = 0;
        BlockType type = detect_block_type(line, level);
        
        if (type == BlockType::Empty) {
            // Empty lines are kept as separators
            MarkdownBlock block;
            block.type = BlockType::Empty;
            doc.blocks.push_back(block);
            continue;
        }
        
        if (type == BlockType::HorizontalRule) {
            MarkdownBlock block;
            block.type = BlockType::HorizontalRule;
            doc.blocks.push_back(block);
            continue;
        }
        
        // Parse content
        std::string content = trim_block_prefix(line, type, level);
        
        MarkdownBlock block;
        block.type = type;
        block.indent_level = level;
        block.spans = parse_inline(content);
        doc.blocks.push_back(block);
    }
    
    return doc;
}

} // namespace markdown
} // namespace fanta
