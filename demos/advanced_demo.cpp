#include "fanta.h"
#include <vector>
#include <string>
#include <iostream>

using namespace fanta;

// Static toggle for popup
static bool g_popup_open = true; // Enabled for verification screenshot

void AdvancedUi() {
    // Root container
    Element root("Root");
    root.size(1280, 720).bg(Color::Hex(0x111111FF)).row().label("");
    
    // --- Sidebar (Scrollable List) ---
    Element sidebar("Sidebar");
    sidebar.width(300).height(720).bg(Color::Hex(0x1A1A1AFF)).label("").column();
    {
        Element title_container("TitleContainer");
        title_container.width(300).height(60).padding(10).label("");
        {
            Element title("SidebarTitle");
            title.label("SCROLLABLE LIST").fontSize(18).height(40).color(Color::Hex(0xBBBBBBFF));
            title_container.child(title);
        }
        sidebar.child(title_container);
        
        // Scrollable Area
        Element scroll("ScrollArea");
        scroll.width(300).height(660).scrollable(true, false).bg(Color::Hex(0x131313FF)).label("");
        {
            // Many cards to force scrolling
            for (int i = 0; i < 15; ++i) {
                std::string card_id = "Card_" + std::to_string(i);
                std::string card_label = "Interactive Item #" + std::to_string(i);
                Element card(card_id.c_str());
                card.width(260).height(80).bg(Color::Hex(0x252525FF))
                    .rounded(8).elevation(2).label(card_label.c_str());
                scroll.child(card);

                // Add a small spacer
                Element spacer("");
                spacer.height(10).label("");
                scroll.child(spacer);
            }
        }
        sidebar.child(scroll);
    }
    root.child(sidebar);
        
    // --- Main Content ---
    Element main("Main");
    main.width(980).height(720).bg(Color::Hex(0x151515FF)).label("").column().padding(40);
    {
        Element content("Content");
        content.width(900).height(680).label("").column(); // inner
        {
            // Markdown Header
            Element header("Header");
            header.markdown("# Phase 8: Advanced Widgets\n**Crystal Architecture** Implementation").height(120).label("");
            content.child(header);
            
            Element summary("Summary");
            summary.markdown("## Features Implemented:\n- **Push/Pop Clip**\n- **Scrollable Container**\n- **Markdown Parser**\n- **Overlay System**").height(240).label("");
            content.child(summary);
            
            // Interaction button
            Element btn("Btn");
            btn.width(220).height(50).bg(Color::Hex(0x3B82F6FF)).rounded(4).elevation(4)
                .label("TOGGLE POPUP");
            content.child(btn);
            
            // Spacer
            Element spacer2(""); spacer2.height(20).label("");
            content.child(spacer2);

            // Canvas inside a panel
            Element panel("CanvasContainer");
            panel.width(600).height(300).bg(Color::Black()).rounded(12).elevation(10).label("");
            {
                 Element canvas("NestedCanvas");
                 canvas.canvas(1.5f, -50, -30).size(600, 300).label("");
                 {
                      Element node1("Node1");
                      node1.node(50, 50).size(150, 100).bg(Color::Hex(0x444466FF)).elevation(5).label("Node 1");
                      canvas.child(node1);
                      
                      Element node2("Node2");
                      node2.node(250, 150).size(150, 100).bg(Color::Hex(0x664444FF)).elevation(5).label("Node 2");
                      canvas.child(node2);

                      Element w("Wire1");
                      w.wire(125, 100, 175, 100, 225, 200, 275, 200, 3.0f);
                      canvas.child(w);
                 }
                 panel.child(canvas);
            }
            content.child(panel);
        }
        main.child(content);
    }
    root.child(main);

    // --- Popup (Rendered as Overlay) ---
    if (g_popup_open) {
        Element popup("PopupOverlay");
        popup.popup().node(440, 260).size(400, 240).bg(Color::Hex(0x222222FA)).rounded(16).elevation(100).label("").column().padding(20);
        {
            Element p_title("PopupTitle");
            p_title.label("MODAL DIALOG").fontSize(22).color(Color::White()).height(40).column();
            popup.child(p_title);
            
            Element p_body("PopupBody");
            p_body.label("This is a root-level overlay.").color(Color::Hex(0xCCCCCCFF)).height(100).column();
            popup.child(p_body);

            Element close("CloseBtn");
            close.width(120).height(40).bg(Color::Hex(0xEF4444FF)).rounded(4).label("CLOSE");
            popup.child(close);
        }
    }
}

int main(int argc, char** argv) {
    bool screenshot_mode = false;
    std::string out_file = "advanced_result.png";

    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--screenshot") screenshot_mode = true;
        if (std::string(argv[i]) == "--output" && i + 1 < argc) out_file = argv[i+1];
    }

    Init(1280, 720, "Fantasmagorie Advanced Demo");
    
    int frame_count = 0;
    Run([&]() {
        AdvancedUi();

        if (screenshot_mode && frame_count == 10) {
            CaptureScreenshot(out_file.c_str());
        }
        if (screenshot_mode && frame_count > 15) {
            // Exit after screenshot
            exit(0);
        }
        frame_count++;
    });

    Shutdown();
    return 0;
}
