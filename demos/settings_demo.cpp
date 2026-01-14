#include "fanta.h"
#include <string>
#include <vector>
#include <cmath>

using namespace fanta;

// --- Functional Components (Widgets as Functions) ---

Element SectionHeader(const char* text) {
    return Element("Header")
        .width(0) // Fill
        .height(40)
        .bg(Color::Hex(0x333333FF))
        .child(Element(text).color(Color::Hex(0xAAAAAAFF)).height(40)); // Text vertically centered-ish
}

Element Slider(const char* id, float value) {
    // Visualizing a slider using two bars stacked (since we only have vertical)
    // Actually, we can use a "Track" background and a "Thumb" on top?
    // No, layout is strictly non-overlapping column.
    // So we simulate a slider as a "Progress Bar" style.
    // Filled Part (Top) ? No, that looks like a vertical bar.
    // We will just draw a horizontal bar that is 100% wide, and rely on the Backend to draw it?
    // No, backend only draws Rects.
    
    // Workaround for Vertical-Only Layout to make a "Horizontal-ish" Slider:
    // We can't.
    // So we just make a bar that changes color brightness based on value.
    return Element(id)
        .height(30)
        .bg(Color::Hex(0x007AFF00).alpha(value)) // Blue opacity = value
        .rounded(15)
        .child(Element("Value").color(Color::White()));
}

Element Toggle(const char* id, bool isOn) {
    return Element(id)
        .height(40)
        .bg(isOn ? Color::Hex(0x34C759FF) : Color::Hex(0x8E8E93FF)) // Green vs Grey
        .rounded(20)
        .child(
            Element(isOn ? "ON" : "OFF").color(Color::White())
        )
        // Mimic "Knob" by adding a child? 
        // It will appear BELOW the text.
        .child(Element("Knob").height(10).bg(Color::White()).rounded(5));
}

int main() {
    Init(400, 800, "Settings (Vertical Layout)");

    Run([]{
        static float volume = 0.7f;
        static float brightness = 0.4f;
        static bool wifi = true;
        static bool bluetooth = false;

        Element("Root")
            .bg(Color::Hex(0x000000FF))
            
            .child(Element("TitleBar").height(60).bg(Color::Hex(0x1C1C1EFF))
                .child(Element("Settings").color(Color::White()))
            )
            
            // --- Group 1 ---
            .child(SectionHeader("CONNECTIVITY"))
            
            .child(Element("WiFiRow").height(80).bg(Color::Hex(0x1C1C1EFF)).rounded(10)
                .child(Element("WiFi Label").color(Color::White()))
                .child(Toggle("WiFiToggle", wifi))
            )
            
            .child(Element("Spacer").height(10)) // Gap
            
            .child(Element("BTRow").height(80).bg(Color::Hex(0x1C1C1EFF)).rounded(10)
                .child(Element("Bluetooth Label").color(Color::White()))
                .child(Toggle("BTToggle", bluetooth))
            )
            
            // --- Group 2 ---
            .child(SectionHeader("DISPLAY & SOUND"))
            
            .child(Element("VolRow").height(80).bg(Color::Hex(0x1C1C1EFF)).rounded(10)
                .child(Element("Volume").color(Color::White()))
                .child(Slider("VolSlider", volume))
            )
            
            .child(Element("Spacer").height(10))
            
            .child(Element("BriRow").height(80).bg(Color::Hex(0x1C1C1EFF)).rounded(10)
                 .child(Element("Brightness").color(Color::White()))
                 .child(Slider("BriSlider", brightness))
            );
            
            // Simulation of input (auto-animate)
            static int frame = 0;
            frame++;
            if (frame % 120 == 0) wifi = !wifi;
            if (frame % 80 == 0) bluetooth = !bluetooth;
            volume = (std::sin(frame * 0.05f) + 1.0f) * 0.5f;
    });

    return 0;
}
