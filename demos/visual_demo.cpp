#include "fanta.h"
#include <string>

using namespace fanta;

// Aesthetic Constants (Apple/Skia-like)
const Color APP_BG = Color::Hex(0x121212FF); // Almost black
const Color CARD_BG_1 = Color::Hex(0x1E1E1EFF);
const Color CARD_BG_2 = Color::Hex(0x252525FF);
const Color CARD_BG_3 = Color::Hex(0x2C2C2CFF);
const Color ACCENT_BLUE = Color::Hex(0x4B5EFFFF);
const Color ACCENT_ORANGE = Color::Hex(0xFF9F0AFF);
const Color ACCENT_GREEN = Color::Hex(0x30D158FF);
const Color TEXT_PRIMARY = Color::Hex(0xFFFFFFFF);
const Color TEXT_SECONDARY = Color::Hex(0xEBEBF599); // 60% alpha

Element Spacer(const char* id, float h) {
    return Element(id).height(h).label("");
}

Element Card(const char* id, const char* title, const char* desc, Color bg) {
    std::string pid = id;
    return Element(id).label("")
        .width(360).height(120) 
        .bg(bg)
        .rounded(18)
        .elevation(10.0f)
        .child(Spacer((pid + "_sp").c_str(), 15))
        .child(Element((pid + "_t").c_str()).label(title).color(TEXT_PRIMARY).height(20))
        .child(Element((pid + "_d").c_str()).label(desc).color(TEXT_SECONDARY).height(20));
}

Element CircleButton(const char* id, Color c) {
    return Element(id).label("")
        .size(60, 60)
        .bg(c)
        .rounded(30)
        .elevation(5.0f);
}

Element Pill(const char* id, const char* label) {
    std::string pid = id;
    return Element(id).label("")
        .width(100).height(34)
        .bg(Color::Hex(0xFFFFFF20))
        .rounded(17)
        .blur(10.0f).backdrop(true)
        .child(Element((pid + "_l").c_str()).label(label).color(TEXT_PRIMARY));
}

// Screenshot mode flag
static bool g_screenshot_mode = false;
static std::string g_screenshot_filename = "screenshot.png";
static int g_frame_count = 0;

int main(int argc, char* argv[]) {
    // Check for arguments
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--screenshot") {
            g_screenshot_mode = true;
        } else if (std::string(argv[i]) == "--output" && i + 1 < argc) {
            g_screenshot_filename = argv[++i];
        }
    }
    
    Init(450, 900, "Rich Visual Demo (Crystal v3.1)");

    Run([]{
        // Screenshot mode: capture after a few frames for rendering to settle
        if (g_screenshot_mode) {
            if (g_frame_count == 3) {
                CaptureScreenshot(g_screenshot_filename.c_str());
            }
            if (g_frame_count == 4) {
                 // Wait one more frame for capture to complete (EndFrame processes it)
                 // Since there is no RequestExit API yet, we just force exit here
                 // But ideally we should add RequestExit() to fanta.h
                 exit(0);
            }
        }
        g_frame_count++;

        Element("Root").label("")
            .bg(APP_BG)
            
            // Header
            .child(Element("Header").label("").height(100)
                .child(Spacer("H_S1", 40))
                .child(Element("Title").label("Fantasmagorie v3").color(TEXT_PRIMARY))
                .child(Element("Subtitle").label("SDF Rendering & Typography").color(TEXT_SECONDARY))
            )
            
            // Phase 5.3: Divider Line
            .child(Element("Divider").line().width(360).height(1).bg(Color::Hex(0xFFFFFF33))) // 20% white
            
            .child(Spacer("HS_2", 20))

            // Cards Stack
            .child(Card("C1", "Subtle Elevation", "Elevation Layer 1", CARD_BG_1))
            .child(Spacer("S1", 20))
            .child(Card("C2", "Medium Elevation", "Elevation Layer 2", CARD_BG_2))
            .child(Spacer("S2", 20))
            .child(Card("C3", "High Elevation", "Elevation Layer 3", CARD_BG_3))

            .child(Spacer("S3", 40))

            // Action Cluster
            .child(Element("Actions").label("").height(200)
                .child(CircleButton("CB1", ACCENT_BLUE))
                .child(Spacer("AS1", 10))
                .child(CircleButton("CB2", ACCENT_ORANGE))
                .child(Spacer("AS2", 10))
                .child(CircleButton("CB3", ACCENT_GREEN))
            )

            .child(Spacer("S4", 20))
            
            // Spring to push footer down
            .child(Element("Spring").label("").height(0))

            // Typography / Pills
            .child(Element("Footer").label("").height(80).row() // Row Layout!
                .child(Pill("P1", "System"))
                .child(Spacer("FS1", 5))
                .child(Pill("P2", "Typography"))
            );
    });

    return 0;
}
