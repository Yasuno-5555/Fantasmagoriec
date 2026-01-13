// Fantasmagorie v2 - DX Verification Demo
// This file tests the ergonomics of the new Refactored Widget API.

#include "../core/context.hpp"
#include "../widgets/button.hpp"
#include "../widgets/slider.hpp"
#include "../widgets/checkbox.hpp"
#include "../widgets/label.hpp"
#include "../widgets/icon.hpp" // Added
#include "../widgets/scroll_panel.hpp" // Phase 9
#include "../widgets/widget_base.hpp" // For PanelScope if I want to use it, or manual

namespace fanta {

// Mock application state
struct AppState {
    float volume = 0.5f;
    bool enabled = true;
    bool show_settings = false;
};

static AppState app;

void build_demo_ui() {
    using namespace fanta; // Assumption: we are inside fanta namespace anyway
    
    // 1. Window/Panel Setup
    // Using RAII PanelScope from widget_base.hpp
    PanelScope win("Main Window", [](WidgetCommon& c){
        c.width = 400;
        c.height = -1; // Auto
        c.padding = 20;
    });
    
    // Manual style override if needed (until PanelScope supports it better)
    {
        NodeHandle n = g_ctx->get(win.id());
        n.layout().dir = LayoutDir::Column;
        n.style().bg = Color::Hex(0x303030FF);
    }
    
    // 2. Simple Header
    Label("DX Verification Demo")
        .bold()
        .color(0xFFD700FF) // Gold
        .height(40)
        .build();
        
    // 3. Control Group
    {
        PanelScope group("Controls", [](WidgetCommon& c){
            c.padding = 10;
            c.gap = 8; // Testing Gap Support
        });
        
        NodeHandle g = g_ctx->get(group.id());
        g.layout().dir = LayoutDir::Column;
        g.style().bg = Color::Hex(0x404040FF);
        g.style().corner_radius = 8;
        
        // Checkbox binding
        Checkbox("Enable Audio Engine")
            .bind(&app.enabled)
            .animate_on(InteractionState::Hover, [](ResolvedStyle& s){
                s.bg = Color::Hex(0x505050FF); // Lighter on hover
                s.scale = 1.02f; // Slight zoom
            })
            // Tactile feedback on click? 
            // Checkbox doesn't use Active state usually, but let's try
            .animate_on(InteractionState::Active, [](ResolvedStyle& s){
                s.scale = 0.95f; // Shrink on press
            })
            .build();
            
        // Conditional UI
        if (app.enabled) {
            Label("Volume Control").height(24).build();
            
            // Slider with chained constraints and logic
            Slider("Master Vol")
                .range(0.0f, 1.0f)
                .bind(&app.volume)
                .width(250)
                .undoable()
                .animate_on(InteractionState::Active, [](ResolvedStyle& s){
                    s.scale = 0.98f;
                })
                .build();
        }
    } // End Controls
    
    // 4. Action Buttons
    {
        PanelScope actions("Actions", [](WidgetCommon& c){
            c.padding = 10;
            c.gap = 10; // Testing Gap in Row
        });
        
        NodeHandle a = g_ctx->get(actions.id());
        a.layout().dir = LayoutDir::Row;
        // a.layout().justify = Align::End; // Right align -> Gap handles spacing now
        
        // Button with click handler
        if (Button("Reset")
            .width(100)
            .animate_on(InteractionState::Active, [](ResolvedStyle& s){ s.scale = 0.95f; })
            .build_clicked()) 
        {
            app.volume = 0.5f;
            app.enabled = true;
        }
        
        // Primary button
        Button("Save")
            .primary()
            .width(100)
            .animate_on(InteractionState::Active, [](ResolvedStyle& s){ s.scale = 0.95f; })
            .on_click([](){ 
                // Save logic 
            })
            .build();
            
    } // End Actions
    
    // 5. Glass Panel Demo (Phase 7)
    {
        PanelScope glass("Glass Panel", [](WidgetCommon& c){
            c.width = 300;
            c.padding = 20;
            c.gap = 10;
        });
        
        NodeHandle n = g_ctx->get(glass.id());
        n.style().bg = Color::Hex(0x80808020); // Semi-transparent grey
        n.style().corner_radius = 12;
        
        // Biased Border
        n.style().border.width = 2.0f;
        n.style().border.color_light = Color::Hex(0xFFFFFF60); // Bright Top/Left
        n.style().border.color_dark = Color::Hex(0x00000060);  // Dark Bot/Right
        
        Label("Fantasmagorie v2 DX Verification").color(0xAAAAAAFF).build();
        Label("Press TAB to navigate focus!").color(0xFB8B24FF).build();
        Button("In Glass").width(120).build();
    }
    
    // 6. Icon Demo (Phase 7)
    {
        PanelScope icons("Icons", [](WidgetCommon& c){
            c.padding = 20;
            c.gap = 15;
        });
        
        NodeHandle n = g_ctx->get(icons.id());
        n.layout().dir = LayoutDir::Row;
        
        // Include Icon header implicitly? No, dx_test needs it.
        // But it's included via widget_base? No.
        // I need to add #include "../widgets/icon.hpp" to dx_test.cpp
        
        // Usage (assuming header included)
        Icon(IconType::Check).size(24).color(Color::Hex(0x00FF00FF)).build();
        Icon(IconType::Close).size(24).color(Color::Hex(0xFF0000FF)).build();
        Icon(IconType::Menu).size(24).color(Color::White()).build();
        Icon(IconType::Search).size(24).color(Color::Hex(0xAAAAFFFF)).build();
        Icon(IconType::Settings).size(24).color(Color::Hex(0xCCCCCCFF)).build();
    }
    
    // 7. Scroll Demo (Phase 9)
    {
        Label("Scroll Demo").bold().color(0xFFD700FF).height(30).build();
        
        ScrollPanel("MyScrollArea")
            .width(350).height(150)
            .padding(8)
            .begin();
        
        for (int i = 0; i < 15; ++i) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Scroll Item %d", i + 1);
            Button(buf)
                .width(300)
                .height(28)
                .animate_on(InteractionState::Hover, [](ResolvedStyle& s){
                    s.bg = Color::Hex(0x505050FF);
                })
                .build();
        }
        
        g_ctx->end_node();
    }

    // End Main Window (Handled by PanelScope destructor)
}

} // namespace fanta
