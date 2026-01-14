#include "fanta.h"
#include "fanta_views.h"
#include <iostream>

// V5 Usage Demo
// This replaces the old benchmark or acts as a testbed.

using namespace fanta;

int main() {
    Init(1280, 800, "Fantasmagorie V5 Demo");
    
    // Main Loop
    while (true) {
        BeginFrame();
        
        // V5 Declarative UI
        // Allocates nodes on FrameArena
        
        // Root Container
        auto root = Column({
            // Header
            Row({
                Label("Fantasmagorie V5"),
                Box({ .width = 20, .height = 1 }), // Spacer
                Label("Refactor Check")
            }),
            
            // Content
            Box({ .height = 20 }),
            
            Label("Welcome to the FrameArena era."),
            
            // Buttons
            Row({
                TextButton("Cancel", { .func = [](void*){ std::cout << "Cancel\n"; } }),
                Box({ .width = 10 }),
                TextButton("Submit", { .func = [](void*){ std::cout << "Submit\n"; } })
            })
        });
        
        // In V4, we built Elements. In V5, we build Nodes.
        // Phase 4 will introduce "Lowering" (View -> Element/Node IR).
        // For now, these nodes are just created and destroyed. 
        // We can't see them yet because RenderTree doesn't know about them.
        // But if this runs without crashing, memory allocation works.
        
        EndFrame();
    }
    
    Shutdown();
    return 0;
}
