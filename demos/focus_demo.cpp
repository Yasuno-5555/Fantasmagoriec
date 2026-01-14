#include "fanta.h"
#include <iostream>

int main() {
    fanta::Init(1280, 720, "Fantasmagorie Crystal - Focus Demo");
    
    fanta::Run([]() {
        fanta::Element root("Main");
        root.size(1280, 720).bg(fanta::GetTheme().background).padding(40).column().gap(20);
        {
            fanta::Element title("Title");
            title.label("Focus & Keyboard Navigation").fontSize(40).color(fanta::GetTheme().primary);
            root.child(title);
            
            fanta::Element subtitle("Subtitle");
            subtitle.label("Press TAB to cycle focus. Click to select.").fontSize(20).color(fanta::GetTheme().text_muted);
            root.child(subtitle);

            // Row for focusable buttons
            fanta::Element row("ButtonRow");
            row.row().gap(20).height(100);
            {
                for (int i = 1; i <= 4; ++i) {
                    char id[32];
                    snprintf(id, sizeof(id), "FocusBtn%d", i);
                    char label[32];
                    snprintf(label, sizeof(label), "Button %d", i);
                    
                    fanta::Element btn(id);
                    btn.width(180).height(60).rounded(8).elevation(2)
                       .bg(fanta::GetTheme().surface)
                       .label(label)
                       .focusable()
                       .focusBg(fanta::GetTheme().primary)
                       .focusLift(8.0f)
                       .hoverBg(fanta::Color::Hex(0x444444FF))
                       .hoverLift(4.0f)
                       .activeBg(fanta::GetTheme().accent)
                       .activeScale(-0.08f);
                    row.child(btn);
                }
            }
            root.child(row);
            
            fanta::Element info("InfoBox");
            info.size(400, 80).bg(fanta::GetTheme().surface).rounded(12).elevation(1)
               .label("The focused button gets a ring!")
               .focusable()
               .focusBg(fanta::Color::Hex(0x3366CCFF))
               .focusLift(12.0f);
            root.child(info);
        }
    });
    
    fanta::Shutdown();
    return 0;
}
