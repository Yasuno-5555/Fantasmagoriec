// Fantasmagorie v2 - Markdown Implementation
#include "markdown.hpp"
#include "label.hpp"
#include "widgets.hpp" // For Spacer, layout helpers
#include <sstream>
#include <vector>

namespace fanta {

void MarkdownBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    // Container for the whole markdown document
    NodeID id = g_ctx->begin_node("markdown_doc");
    NodeHandle n = g_ctx->get(id);
    
    apply_constraints(n);
    n.layout().dir = LayoutDir::Column;
    
    // Simple state machine for parsing
    parse_and_build(text_);
    
    g_ctx->end_node();
}

void MarkdownBuilder::parse_and_build(const char* text) {
    if (!text) return;
    
    std::stringstream ss(text);
    std::string line;
    std::vector<std::string> code_buffer;
    bool in_code_block = false;
    
    while (std::getline(ss, line)) {
        // Handle CRLF
        if (!line.empty() && line.back() == '\r') line.pop_back();
        
        // Code blocks
        if (line.substr(0, 3) == "```") {
            if (in_code_block) {
                render_code_block(code_buffer);
                code_buffer.clear();
                in_code_block = false;
            } else {
                in_code_block = true;
            }
            continue;
        }
        
        if (in_code_block) {
            code_buffer.push_back(line);
            continue;
        }
        
        // Skip empty lines (could add spacing)
        if (line.empty()) {
            Spacer(8.0f);
            continue;
        }
        
        // Headers
        if (line[0] == '#') {
            int level = 0;
            while (level < (int)line.size() && line[level] == '#') level++;
            render_header(level, line.substr(level));
        }
        // List items
        else if (line.size() >= 2 && line[0] == '-' && line[1] == ' ') {
            render_list_item(line.substr(2));
        }
        // Normal paragraph
        else {
            render_paragraph(line);
        }
    }
}

void MarkdownBuilder::render_header(int level, const std::string& line) {
    // Trim leading space
    size_t first = line.find_first_not_of(" \t");
    std::string content = (first == std::string::npos) ? "" : line.substr(first);
    
    // Scale size based on level (H1=32, H2=24, H3=20...)
    // Note: Assuming default font, we can't easily change font size per-node without 
    // full style support in renderer, but let's simulate intent or use scale transform?
    // For now, use Label with bold. In a real engine, we'd set font size style.
    
    Label(content.c_str())
        .bold() // Hypothetical API extension or just bold style
        // .size(32.0f - level * 4.0f) 
        .build();
        
    // Underline for H1/H2
    if (level <= 2) {
        NodeID div = g_ctx->begin_node("divider");
        NodeHandle h = g_ctx->get(div);
        h.constraint().height = 1.0f;
        h.constraint().width = -1; // Fill
        h.style().bg = Color::Hex(0x555555FF);
        g_ctx->end_node();
        Spacer(8.0f);
    }
}

void MarkdownBuilder::render_list_item(const std::string& line) {
    // Row: Bullet + Text
    NodeID row = g_ctx->begin_node("list_item");
    g_ctx->get(row).layout().dir = LayoutDir::Row;
    
    // Bullet
    Label(u8"•").build(); // Unicode bullet
    Spacer(8.0f);
    
    // Text
    render_paragraph(line);
    
    g_ctx->end_node();
}

void MarkdownBuilder::render_code_block(const std::vector<std::string>& lines) {
    NodeID panel = BeginPanel("code_block");
    NodeHandle n = g_ctx->get(panel);
    n.style().bg = Color::Hex(0x00000040); // Darker background
    n.layout().dir = LayoutDir::Column;
    
    for (const auto& l : lines) {
        Label(l.c_str())
            .color(0xAAAAAAFF) // Grey text
            .build();
    }
    
    EndPanel();
    Spacer(8.0f);
}

void MarkdownBuilder::render_paragraph(const std::string& line) {
    // TODO: support wrapping or inline bold/italic parsing here.
    // For now, simple text node.
    Label(line.c_str()).build();
}

} // namespace fanta
