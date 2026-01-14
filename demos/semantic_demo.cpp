#include "fanta.h"
#include <iostream>

int main() {
    fanta::Init(1280, 720, "Fantasmagorie Crystal - Semantic Colors Demo");
    fanta::SetTheme(fanta::Theme::AppleHIG()); // Use Apple HIG theme
    
    fanta::Run([]() {
        fanta::Element root("Main");
        root.size(1280, 720).bg(fanta::GetTheme().background)
            .padding(fanta::Spacing::XL()).column().gap(fanta::Spacing::M());
        {
            fanta::Element title("Title");
            // Use Color::Label() instead of GetTheme().text
            title.label("Apple HIG Semantic Colors")
                .textStyle(fanta::TextToken::LargeTitle) // Phase 12.7 Typography
                .color(fanta::Color::Label());
            root.child(title);
            
            fanta::Element subtitle("Subtitle");
            // Use Color::SecondaryLabel()
            subtitle.label("Using structural semantic color tokens").textStyle(fanta::TextToken::Title1).color(fanta::Color::SecondaryLabel());
            root.child(subtitle);

            fanta::Element desc_new("Description");
            desc_new.label("This demo uses semantic tokens for colors and spacing.")
                .textStyle(fanta::TextToken::Body)
                .color(fanta::Color::SecondaryLabel());
            root.child(desc_new);
            
            fanta::Element desc("Desc");
            // Use Color::TertiaryLabel()
            desc.label("Tertiary label for less important info").fontSize(14).color(fanta::Color::TertiaryLabel());
            root.child(desc);

            fanta::Element separator("Sep");
            separator.line().width(1200).height(1).bg(fanta::Color::Separator());
            root.child(separator);

            // Row for semantic fills
            fanta::Element row("FillRow");
            row.row().gap(fanta::Spacing::L()).height(200);
            {
                fanta::Element card1("Card1");
                card1.size(300, 180).rounded(16).padding(fanta::Spacing::L())
                     .bg(fanta::Color::Fill()) // SecondarySystemBackground
                     .elevation(2);
                {
                    fanta::Element t1("T1");
                    t1.label("Fill Color").color(fanta::Color::Label());
                    card1.child(t1);
                }
                row.child(card1);
                
                fanta::Element card2("Card2");
                card2.size(300, 180).rounded(16).padding(20)
                     .bg(fanta::Color::SecondaryFill()) // TertiarySystemBackground
                     .elevation(4);
                {
                    fanta::Element t2("T2");
                    t2.label("Secondary Fill").color(fanta::Color::Label());
                    card2.child(t2);
                    
                    fanta::Element sep2("Sep2");
                    sep2.line().width(260).height(1).bg(fanta::Color::Separator());
                    card2.child(sep2);
                }
                row.child(card2);
                
                fanta::Element card3("Card3");
                card3.size(300, 180).rounded(16).padding(20)
                     .bg(fanta::Color::Fill())
                     .elevation(2);
                {
                     fanta::Element btn("ActionBtn");
                     btn.size(120, 44).rounded(8).bg(fanta::Color::Accent()) // SystemGreen
                        .label("Accent Action").color(fanta::Color::White())
                        .hoverLift(4);
                     card3.child(btn);

                     // Disabled Button
                     fanta::Element btn_disabled("DisabledBtn");
                     btn_disabled.size(120, 44).rounded(8).bg(fanta::Color::Destructive())
                        .label("Disabled").color(fanta::Color::White())
                        .disabled(true); // Phase 2.2
                     card3.gap(10).child(btn_disabled);
                }
                row.child(card3);
            }
            root.child(row);
        }
    });
    
    fanta::Shutdown();
    return 0;
}
