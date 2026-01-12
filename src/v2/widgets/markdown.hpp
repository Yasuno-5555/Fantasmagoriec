// Fantasmagorie v2 - Markdown Widget
// Lightweight block-level Markdown renderer
#pragma once

#include "builder_base.hpp"
#include <string>

namespace fanta {

class MarkdownBuilder : public BuilderBase<MarkdownBuilder> {
public:
    explicit MarkdownBuilder(const char* text) : text_(text) {
        // Default constraints for a document view
        width_ = -1; // Auto/Fill
    }

    // Custom styling overrides could go here
    // MarkdownBuilder& code_font(Font* font); 
    
    void build();
    ~MarkdownBuilder() { if (!built_) build(); }
    
private:
    const char* text_;
    
    void parse_and_build(const char* text);
    void render_header(int level, const std::string& line);
    void render_list_item(const std::string& line);
    void render_code_block(const std::vector<std::string>& lines);
    void render_paragraph(const std::string& line);
};

inline MarkdownBuilder Markdown(const char* text) {
    return MarkdownBuilder(text);
}

} // namespace fanta
