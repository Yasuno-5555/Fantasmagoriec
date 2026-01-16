#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace fanta {
namespace markdown {

// --- Block Types ---
enum class BlockType {
    Paragraph,
    Heading1,
    Heading2,
    Heading3,
    Heading4,
    Heading5,
    Heading6,
    BulletList,
    NumberedList,
    ListItem,
    CodeBlock,
    BlockQuote,
    HorizontalRule,
    Empty
};

// --- Inline Styles ---
enum class InlineStyle : uint8_t {
    None   = 0,
    Bold   = 1 << 0,
    Italic = 1 << 1,
    Code   = 1 << 2,
    Link   = 1 << 3
};

inline InlineStyle operator|(InlineStyle a, InlineStyle b) {
    return static_cast<InlineStyle>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline InlineStyle operator&(InlineStyle a, InlineStyle b) {
    return static_cast<InlineStyle>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

// --- Inline Span ---
struct InlineSpan {
    std::string text;
    InlineStyle style = InlineStyle::None;
    std::string link_url; // For Link style
};

// --- Markdown Block ---
struct MarkdownBlock {
    BlockType type = BlockType::Paragraph;
    std::vector<InlineSpan> spans;
    int indent_level = 0; // For nested lists
    std::string code_language; // For code blocks
};

// --- Parsed Document ---
struct MarkdownDocument {
    std::vector<MarkdownBlock> blocks;
};

// --- Parser ---
class MarkdownParser {
public:
    MarkdownDocument parse(const std::string& source);
    
private:
    BlockType detect_block_type(const std::string& line, int& out_level);
    std::vector<InlineSpan> parse_inline(const std::string& text);
    std::string trim_block_prefix(const std::string& line, BlockType type, int level);
};

// --- Helper: Get font size for block type ---
inline float get_font_size(BlockType type) {
    switch (type) {
        case BlockType::Heading1: return 32.0f;
        case BlockType::Heading2: return 26.0f;
        case BlockType::Heading3: return 22.0f;
        case BlockType::Heading4: return 18.0f;
        case BlockType::Heading5: return 16.0f;
        case BlockType::Heading6: return 14.0f;
        case BlockType::CodeBlock: return 13.0f;
        default: return 16.0f;
    }
}

// --- Helper: Is heading ---
inline bool is_heading(BlockType type) {
    return type >= BlockType::Heading1 && type <= BlockType::Heading6;
}

} // namespace markdown
} // namespace fanta
