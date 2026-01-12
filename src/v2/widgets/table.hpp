// Fantasmagorie v2 - Table Widget
// Data grid with sortable columns
#pragma once

#include "builder_base.hpp"
#include <vector>

namespace fanta {

struct TableColumn {
    const char* header;
    float width = 100.0f;
    bool sortable = false;
};

class TableBuilder : public BuilderBase<TableBuilder> {
public:
    explicit TableBuilder(const char* id) : id_(id) {}
    
    TableBuilder& columns(std::initializer_list<TableColumn> cols) {
        columns_ = cols;
        return *this;
    }
    
    TableBuilder& row_count(int count) { row_count_ = count; return *this; }
    TableBuilder& row_height(float h) { row_height_ = h; return *this; }
    TableBuilder& header_height(float h) { header_height_ = h; return *this; }
    TableBuilder& striped(bool s = true) { striped_ = s; return *this; }
    TableBuilder& on_cell(std::function<void(int row, int col)> fn) { on_cell_ = fn; return *this; }
    
    void build();
    ~TableBuilder() { if (!built_) build(); }
    
private:
    const char* id_;
    std::vector<TableColumn> columns_;
    int row_count_ = 0;
    float row_height_ = 32.0f;
    float header_height_ = 36.0f;
    bool striped_ = true;
    std::function<void(int, int)> on_cell_;
};

inline TableBuilder Table(const char* id) {
    return TableBuilder(id);
}

} // namespace fanta
