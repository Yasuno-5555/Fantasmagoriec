#pragma once

#include "../core/types_internal.hpp"

namespace fanta {
namespace internal {

// TreeNode State
struct TreeNodeState {
    bool expanded = false;
    int depth = 0;
    bool has_children = false;
    
    void toggle() { expanded = !expanded; }
};

// TreeNode Logic
struct TreeNodeLogic {
    static void Update(ElementState& el, InputContext& input, const ComputedLayout& layout);
    static float GetIndent(int depth) { return depth * 20.0f; }
    static float GetArrowSize() { return 12.0f; }
};

} // namespace internal
} // namespace fanta
