#include "fanta.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    fanta::Init(800, 600, "Fantasmagorie Crystal - Multi-selection Demo");
    fanta::SetTheme(fanta::Theme::Dark());

    fanta::Run([]() {
        fanta::Element root("Background");
        root.size(800, 600)
            .bg(fanta::Color::Hex(0x1a1a1aFF))
            .column();
        
        {
            fanta::Element label("Instruction");
            label.padding(20).label("Click to select, Drag to Marquee, Shift+Click to multi-select")
                 .color(fanta::Color::Label());
            root.child(label);
        }

        // Nodes
        for (int i = 0; i < 5; ++i) {
            std::string id = "Node_" + std::to_string(i);
            fanta::Element node(id.c_str());
            node.node(50.0f + i * 140.0f, 150.0f) // Absolute positioning
                .size(120, 100)
                .bg(fanta::Color::Hex(0x333333FF))
                .rounded(8)
                .padding(10)
                .focusable()
                .elevation(2)
                .label(id.c_str())
                .hoverLift(4)
                .activeScale(-0.05f);
            
            root.child(node);
        }

        // Automated verification screenshots
        static int frame = 0;
        frame++;
        if (frame == 30) {
            // No selection yet
            fanta::CaptureScreenshot("interaction_init.png");
        } else if (frame == 60) {
            // We can't easily simulate marquee from here without more hacks, 
            // but we can at least check it builds and runs.
            // I'll end it here for now.
            exit(0);
        }
    });

    fanta::Shutdown();
    return 0;
}
