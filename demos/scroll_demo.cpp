#include "fanta.h"
#include <iostream>
#include <string>

int main() {
    fanta::Init(400, 800, "Fantasmagorie Crystal - Virtualized List Demo");
    fanta::SetTheme(fanta::Theme::AppleHIG());

    fanta::Run([]() {
        fanta::Element root("Main");
        root.size(400, 800)
            .bg(fanta::Color::Background())
            .column(); // Vertical Stack
        
        {
            fanta::Element header("Header");
            header.size(400, 60).bg(fanta::Color::SecondarySystemBackground())
                  .padding(16).elevation(4);
            header.label("Virtualized Scroll (1000 items)").textStyle(fanta::TextToken::Headline)
                  .color(fanta::Color::Label());
            root.child(header);
        }

        // Scroll Container
        fanta::Element list("List");
        list.size(400, 740) // Fill remaining space
            .scrollable(true, false) // Enable Vertical Scroll
            .column(); // Children in column
        
        // Populate 1000 items
        for (int i = 0; i < 1000; ++i) {
            std::string id = "Item_" + std::to_string(i);
            fanta::Element item(id.c_str());
            
            // Alternating colors
            auto bg = (i % 2 == 0) ? fanta::Color::SecondarySystemBackground() : fanta::Color::Background();
            
            item.size(400, 50)
                .bg(bg)
                .padding(12)
                .color(fanta::Color::Label())
                .label(("Item " + std::to_string(i)).c_str())
                .textStyle(fanta::TextToken::Body)
                .hoverBg(fanta::Color::Accent()) // Highlight on hover
                .onClick([i]{ std::cout << "Clicked item " << i << std::endl; });
            
            // Add separator
            if (i < 999) {
                fanta::Element line(("Line_" + std::to_string(i)).c_str());
                line.line().width(384).height(1).bg(fanta::Color::Separator());
               // item.child(line); // No, sibling or child? List expects items
               // We'll just stick to simple items
            }

            list.child(item);
        }
        
        root.child(list);

        static int frame = 0;
        frame++;

        // Theme Screenshot Tour
        if (frame == 20) {
            fanta::SetDebugHitTest(true);
            fanta::CaptureScreenshot("hit_test_debug.png");
            std::cout << "Captured Hit Test Debug" << std::endl;
        }
        else if (frame == 40) {
            // End Tour
            exit(0);
        }
    });

    fanta::Shutdown();
    return 0;
}
