#include "fanta.h"
#include <iostream>
#include <string>

using namespace fanta;

static bool g_is_dark = false;

void ThemeUi() {
    Theme current = GetTheme();

    // Use a large root container with the theme background
    Element root("Root");
    root.size(1280, 720).bg(current.background).column().padding(40).gap(20).label("");
    {
        Element title("Title");
        title.label(g_is_dark ? "DARK THEME" : "LIGHT THEME")
             .fontSize(32).color(current.primary).height(50);
        root.child(title);

        Element hint("Hint");
        hint.label("Press 'T' to toggle theme").color(current.text_muted).height(30);
        root.child(hint);

        // A card-like surface
        Element surface("Surface");
        surface.width(600).height(400).bg(current.surface).rounded(12).elevation(4).padding(20).gap(15).label("");
        {
             Element md("Intro");
             md.markdown("# Theme Support\nVisual elements now follow a **Semantic Theme** system.\n- [Visit Website](https://fantasmagorie.io)\n- Support for dynamic switching.");
             surface.child(md);

             Element btn("Button");
             btn.width(200).height(50).bg(current.primary).rounded(6).elevation(2).label("THEMED BUTTON");
             surface.child(btn);
             
             Element accent_box("AccentBox");
             accent_box.width(200).height(50).bg(current.accent).rounded(6).label("ACCENT COLOR");
             surface.child(accent_box);
        }
        root.child(surface);
    }
}

int main(int argc, char** argv) {
    bool screenshot_mode = false;
    std::string out_file = "theme_result.png";

    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--screenshot") screenshot_mode = true;
        if (std::string(argv[i]) == "--output" && i + 1 < argc) out_file = argv[i+1];
    }

    Init(1280, 720, "Fantasmagorie Theme Demo");
    SetTheme(Theme::Light());
    
    int frame_count = 0;
    static bool t_pressed_last = false;

    Run([&]() {
        bool t_pressed = IsKeyDown('T');
        if (t_pressed && !t_pressed_last) {
            g_is_dark = !g_is_dark;
            SetTheme(g_is_dark ? Theme::Dark() : Theme::Light());
        }
        t_pressed_last = t_pressed;

        ThemeUi();

        if (screenshot_mode && frame_count == 10) {
            CaptureScreenshot(out_file.c_str());
        }
        if (screenshot_mode && frame_count > 15) {
            exit(0);
        }
        frame_count++;
    });

    Shutdown();
    return 0;
}
