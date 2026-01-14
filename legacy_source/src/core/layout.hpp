// Fantasmagorie v2 - Layout Engine
// Flexbox-like layout solver
#pragma once

#include "core/node.hpp"

namespace fanta {

class LayoutEngine {
public:
    // Solve layout for entire tree
    static void solve(NodeStore& store, NodeID root, float width, float height);
    
private:
    static void solve_node(NodeStore& store, NodeID id, float x, float y, float w, float h);
    static void measure_children(NodeStore& store, NodeID id);
};

} // namespace fanta


