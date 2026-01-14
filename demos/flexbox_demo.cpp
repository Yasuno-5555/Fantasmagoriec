#include "fanta.h"
#include <iostream>

int main() {
    fanta::Init(1280, 720, "Fantasmagorie Crystal - Flexbox Wrap Demo");
    fanta::SetTheme(fanta::Theme::AppleHIG()); // Use Apple HIG theme
    
    fanta::Run([]() {
        fanta::Element root("Main");
        root.size(1280, 720).bg(fanta::GetTheme().background).padding(40).column().gap(20);
        {
            fanta::Element title("Title");
            title.label("Flexbox Wrap Demo").fontSize(32).color(fanta::GetTheme().text);
            root.child(title);
            
            fanta::Element subtitle("Subtitle");
            subtitle.label("Resize window to see wrapping behavior").fontSize(16).color(fanta::GetTheme().text_muted);
            root.child(subtitle);

            // Wrap container - row with wrap enabled
            fanta::Element wrapContainer("WrapContainer");
            wrapContainer.size(1200, 500).bg(fanta::GetTheme().surface).rounded(16).padding(16)
                         .row().wrap().gap(12);
            {
                // Create many small boxes to demonstrate wrap
                for (int i = 1; i <= 20; ++i) {
                    char id[32];
                    snprintf(id, sizeof(id), "Box%d", i);
                    char label[16];
                    snprintf(label, sizeof(label), "%d", i);
                    
                    fanta::Element box(id);
                    box.size(100, 80).rounded(12).elevation(2)
                       .bg(fanta::GetTheme().primary.alpha(0.3f + (i % 5) * 0.14f))
                       .label(label).color(fanta::GetTheme().text)
                       .hoverBg(fanta::GetTheme().primary)
                       .hoverLift(8.0f)
                       .activeScale(-0.05f);
                    wrapContainer.child(box);
                }
            }
            root.child(wrapContainer);
        }
    });
    
    fanta::Shutdown();
    return 0;
}
