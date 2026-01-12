// Fantasmagorie v2 - Table Implementation
#include "table.hpp"

namespace fanta {

void TableBuilder::build() {
    if (built_ || !g_ctx) return;
    built_ = true;
    
    NodeID id = g_ctx->begin_node(id_);
    NodeHandle n = g_ctx->get(id);
    
    apply_constraints(n);
    
    n.style().bg = Color::Hex(0x1A1A1AFF);
    n.style().border = Color::Hex(0x333333FF);
    n.style().border_width = 1.0f;
    n.layout().dir = LayoutDir::Column;
    
    float total_width = 0;
    for (const auto& col : columns_) {
        total_width += col.width;
    }
    
    if (width_ <= 0) {
        n.constraint().width = total_width;
    }
    
    // Header row (simplified)
    {
        NodeID hdr_id = g_ctx->begin_node("header");
        NodeHandle hdr = g_ctx->get(hdr_id);
        hdr.constraint().height = header_height_;
        hdr.style().bg = Color::Hex(0x2A2A2AFF);
        hdr.layout().dir = LayoutDir::Row;
        
        for (size_t i = 0; i < columns_.size(); i++) {
            NodeID cell_id = g_ctx->begin_node(columns_[i].header);
            NodeHandle cell = g_ctx->get(cell_id);
            cell.constraint().width = columns_[i].width;
            cell.style().text = Color::Hex(0xCCCCCCFF);
            cell.render().is_text = true;
            cell.render().text = columns_[i].header;
            g_ctx->end_node();
        }
        
        g_ctx->end_node();
    }
    
    // Data rows
    for (int row = 0; row < row_count_; row++) {
        char row_id[32];
        snprintf(row_id, sizeof(row_id), "row_%d", row);
        
        NodeID row_node = g_ctx->begin_node(row_id);
        NodeHandle row_h = g_ctx->get(row_node);
        row_h.constraint().height = row_height_;
        row_h.layout().dir = LayoutDir::Row;
        
        // Striped background
        if (striped_ && row % 2 == 1) {
            row_h.style().bg = Color::Hex(0x222222FF);
        }
        
        for (size_t col = 0; col < columns_.size(); col++) {
            char cell_id[32];
            snprintf(cell_id, sizeof(cell_id), "cell_%d_%zu", row, col);
            
            NodeID cell_node = g_ctx->begin_node(cell_id);
            NodeHandle cell = g_ctx->get(cell_node);
            cell.constraint().width = columns_[col].width;
            cell.input().clickable = true;
            
            // Cell click callback
            if (on_cell_ && is_clicked(cell_node)) {
                on_cell_(row, (int)col);
            }
            
            g_ctx->end_node();
        }
        
        g_ctx->end_node();
    }
    
    g_ctx->end_node();
}

} // namespace fanta
