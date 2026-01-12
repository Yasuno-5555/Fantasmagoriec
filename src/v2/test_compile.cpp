// Fantasmagorie v2 - Compile Test
// Verifies that core module compiles correctly
#include "core/types.hpp"
#include "core/node.hpp"
#include "core/layout.hpp"
#include "core/context.hpp"

#include <iostream>

int main() {
    using namespace fanta;
    
    // Test Types
    Color c = Color::Hex(0xFF5500FF);
    Vec2 v(100, 200);
    Rect r(0, 0, 800, 600);
    Transform t = Transform::Scale(2.0f);
    
    // Test NodeStore
    NodeStore store;
    store.add_node(1);
    store.add_node(2);
    store.tree[1].children.push_back(2);
    store.tree[2].parent = 1;
    
    // Test NodeHandle
    NodeHandle h(&store, 1);
    h.constraint().width = 100;
    h.style().bg = Color::Hex(0x333333FF);
    
    // Test Layout
    LayoutEngine::solve(store, 1, 800, 600);
    
    // Test UIContext
    UIContext ctx;
    ctx.begin_frame();
    ctx.begin_node("root");
    ctx.begin_node("child");
    ctx.end_node();
    ctx.end_node();
    ctx.end_frame();
    
    std::cout << "Fantasmagorie v2 Core: Compile OK!" << std::endl;
    std::cout << "  NodeStore nodes: " << store.nodes.size() << std::endl;
    std::cout << "  Color test: 0x" << std::hex << c.u32() << std::endl;
    
    return 0;
}
