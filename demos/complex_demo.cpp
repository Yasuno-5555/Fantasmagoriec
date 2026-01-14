#include "fanta.h"
#include <vector>
#include <string>

using namespace fanta;

// Reusable component pattern (Functional Composition)
Element Card(const char* title, const char* value) {
    return Element("Card")
        .width(220).height(100)
        .bg(Color::Hex(0x333333FF))
        .rounded(8)
        .child(Element(title).color(Color::Hex(0xAAAAAAFF)).height(30))
        .child(Element(value).color(Color::White()).height(40)); // value is the label
}

int main() {
    Init(1280, 800, "Crystal Complex Demo (Deep Nesting)");

    Run([]{
        // Level 1: System Root
        Element("Root")
            .size(1280, 800)
            .bg(Color::Hex(0x111111FF))
            
            // Level 2: Main Layout Container
            .child(Element("Container")
                .width(1000).height(780)
                
                // Level 3: Glass Header
                .child(Element("Header")
                    .height(80)
                    .bg(Color::Hex(0xFFFFFF10)) // Low alpha
                    .blur(20.0f)                // Native Blur
                    .backdrop(true)
                    .rounded(12)
                    
                    // Level 4: Header Content
                    .child(Element("AppTitle").color(Color::White()).height(40))
                    .child(Element("Subtitle (""Frozen API"")").color(Color::Hex(0x888888FF)).height(20))
                )

                // Level 3: Body Column
                .child(Element("Body")
                    .width(1000).height(680)
                    
                    // Level 4: Sidebar
                    .child(Element("Sidebar")
                        .width(200).height(680)
                        
                        // Level 5: Menu List
                        .child(Element("Menu").bg(Color::Hex(0x181818FF)).rounded(8)
                            // Level 6: Items
                            .child(Element("Dashboard").height(40))
                            .child(Element("Analytics").height(40))
                            .child(Element("Settings").height(40))
                        )
                    )
                    
                    // Level 4: Content Grid
                    .child(Element("Content")
                        .width(780).height(680)
                        
                        // Level 5: Stats Row
                        .child(Element("StatsRow")
                            .height(120)
                            // Level 6: Cards
                            .child(Card("CPU Usage", "45%"))
                            .child(Card("Memory \n(Allocated)", "2.4 GB"))
                            .child(Card("Active Nodes", "1,240"))
                        )

                        // Level 5: Nested Glass Panel inside Opaque
                        .child(Element("DeepGlassPanel")
                            .width(780).height(300)
                            .bg(Color::Hex(0x00000080)) // Semi-transparent black
                            .rounded(16)
                            
                            // Level 6: The Glass Window
                            .child(Element("GlassWindow")
                                .width(600).height(200)
                                .bg(Color::Hex(0xFFFFFF05))
                                .blur(15.0f)
                                .backdrop(true)
                                .child(Element("Deepest Text").color(Color::Hex(0xFF00FFFF)))
                            )
                        )
                    )
                )
            );
    });

    return 0;
}
