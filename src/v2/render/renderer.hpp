// Fantasmagorie v2 - Renderer
// Standard render pass implementation (NodeStore -> DrawList)
#pragma once

#include "../core/node.hpp"
#include "draw_list.hpp"

namespace fanta {

class Renderer {
public:
    // Submit entire node tree to draw list
    // Respects Visual Layer System order: Shadow -> Background -> Content -> Border -> Children
    static void submit(NodeStore& store, NodeID root, DrawList& list);

private:
    static void submit_node(NodeStore& store, NodeID id, DrawList& list);
};

} // namespace fanta
