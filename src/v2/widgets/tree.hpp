// Fantasmagorie v2 - TreeView Widget
// Hierarchical tree with expand/collapse
#pragma once

#include "builder_base.hpp"
#include <functional>

namespace fanta {

struct TreeNode {
    const char* label;
    std::vector<TreeNode> children;
    bool expanded = false;
    void* user_data = nullptr;
};

class TreeViewBuilder : public BuilderBase<TreeViewBuilder> {
public:
    explicit TreeViewBuilder(const char* id) : id_(id) {}
    
    TreeViewBuilder& root(const TreeNode& node) { root_ = node; return *this; }
    TreeViewBuilder& indent(float i) { indent_ = i; return *this; }
    TreeViewBuilder& on_select(std::function<void(TreeNode*)> fn) { on_select_ = fn; return *this; }
    
    void build();
    ~TreeViewBuilder() { if (!built_) build(); }
    
private:
    const char* id_;
    TreeNode root_;
    float indent_ = 16.0f;
    std::function<void(TreeNode*)> on_select_;
    
    void build_node(TreeNode& node, int depth);
};

inline TreeViewBuilder TreeView(const char* id) {
    return TreeViewBuilder(id);
}

} // namespace fanta
