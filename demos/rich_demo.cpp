#include "fanta.h"
#include <iostream>
#include <string>

static int g_btn_id = 0;
static int g_card_id = 0;

// Helper to create a button
void Button(fanta::Element& parent, const char* label, fanta::Color bg_color, fanta::Color text_color) {
    std::string id = std::string("Btn_") + std::to_string(g_btn_id++);
    fanta::Element btn(id.c_str());
    btn.size(120, 40)
       .bg(bg_color)
       .rounded(8)
       .elevation(2)
       .label(label)
       .color(text_color)
       .textStyle(fanta::TextToken::Callout);
    parent.child(btn);
}

void MakeCard(fanta::Element& parent, const char* title, const char* content) {
    std::string card_id = std::string("Card_") + std::to_string(g_card_id++);
    fanta::Element card(card_id.c_str());
    card.width(300).bg(fanta::Color::SecondarySystemBackground())
        .rounded(12).elevation(4).padding(16).column().gap(8);
        
    std::string title_id = card_id + "_title";
    fanta::Element t(title_id.c_str());
    t.label(title).height(0).textStyle(fanta::TextToken::Headline).color(fanta::Color::Label());
    
    std::string body_id = card_id + "_body";
    fanta::Element b(body_id.c_str());
    b.label(content).height(0).textStyle(fanta::TextToken::Body).color(fanta::Color::SecondaryLabel());
    
    card.child(t).child(b);
    
    // Add some "Active" elements
    std::string row_id = card_id + "_actions";
    fanta::Element row(row_id.c_str());
    row.row().gap(10).height(40);
    Button(row, "View", fanta::Color::Accent(), fanta::Color::White());
    Button(row, "Delete", fanta::Color::Destructive(), fanta::Color::White());
    
    card.child(row);
    parent.child(card);
}

int main() {
    fanta::Init(1024, 768, "Fantasmagorie Crystal - Theme Showcase");
    fanta::SetTheme(fanta::Theme::Dark());

    fanta::Run([]() {
        // --- Root Layout ---
        fanta::Element root("Root");
        root.size(1024, 768).bg(fanta::Color::Background()).row();

        // --- Sidebar ---
        {
            fanta::Element sidebar("Sidebar");
            sidebar.size(240, 768).bg(fanta::Color::TertiarySystemBackground()) // Distinct panel color
                   .padding(20).column().gap(10);
            
            fanta::Element title("AppTitle");
            title.label("Fantasmagorie").textStyle(fanta::TextToken::Title2).color(fanta::Color::Label());
            sidebar.child(title);
            
            fanta::Element sep("Sep");
            sep.line().width(200).height(1).bg(fanta::Color::Separator());
            sidebar.child(sep);
            
            const char* items[] = {"Dashboard", "Projects", "Settings", "Profile"};
            for(auto* i : items) {
                fanta::Element item(i);
                item.size(200, 36).label(i).textStyle(fanta::TextToken::Body)
                    .color(fanta::Color::Label())
                    .hoverBg(fanta::Color::Fill()) // Interactive
                    .rounded(6);
                sidebar.child(item);
            }
            root.child(sidebar);
        }

        // --- Main Content ---
        {
            fanta::Element main("Main");
            main.size(784, 768).padding(40).column().gap(30);
            
            fanta::Element h1("PageTitle");
            h1.label("Design System").textStyle(fanta::TextToken::LargeTitle).color(fanta::Color::Label());
            main.child(h1);
            
            // Grid of Cards
            fanta::Element grid("Grid");
            grid.row().wrap(true).gap(20).width(704); // Wrap content
            
            MakeCard(grid, "Theme Awareness", "Components automatically adapt to the active theme context.");
            MakeCard(grid, "Elevation", "Shadows utilize semantic layers to create depth.");
            MakeCard(grid, "Typography", "System fonts are scaled using dynamic type tokens.");
            MakeCard(grid, "Interaction", "Hover and click states use physics-based feedback.");
            
            main.child(grid);
            root.child(main);
        }

        // --- Screenshot Tour Logic ---
        static int frame = 0;
        frame++;
        
        if (frame == 10) {
            fanta::SetTheme(fanta::Theme::Dark());
            fanta::CaptureScreenshot("rich_dark.png");
        } 
        else if (frame == 20) {
            fanta::SetTheme(fanta::Theme::Light());
            fanta::CaptureScreenshot("rich_light.png");
        }
        else if (frame == 30) {
            fanta::SetTheme(fanta::Theme::AppleHIG());
            fanta::CaptureScreenshot("rich_apple.png");
        }
        else if (frame == 40) {
            fanta::SetTheme(fanta::Theme::Fantasmagorie());
            fanta::CaptureScreenshot("rich_fanta.png");
        }
        else if (frame == 50) {
            exit(0);
        }
    });

    fanta::Shutdown();
    return 0;
}
