#include "ui/api.hpp"
#include "core/job_system.hpp"
#include <string>
#include <vector>

int main() {
    // 1. Core API
    if (ui::BeginMenuBar()) {
        ui::EndMenuBar();
    }
    
    // 2. Images (Phase Visuals)
    ui::DrawImage(nullptr, 100, 100);
    
    // 3. Animation (Button)
    ui::Button("Anim");
    
    // 4. Markdown (Phase 2)
    ui::Markdown("# Header\nLink [Go](http)");
    
    // 5. Localization (Phase 3)
    ui::Button(ui::Loc("Welcome"));
    ui::Button("キャンセル"); // UTF-8 Verify
    
    // 6. Job System (Phase 4)
    ui::Spinner();
    
    // 7. Essential Widgets (Phase 1 Ext)
    static bool chk = false;
    ui::Checkbox("Toggle", &chk);
    static float val = 0.5f;
    ui::Slider("Value", &val, 0.0f, 1.0f);
    static ui::Color col = {1,1,1,1};
    ui::ColorEdit("Color", &col);
    
    // 8. Phase 2: Structure
    ui::PushStyleColor(ui::StyleColor::Text, {1,0,0,1}); 
    ui::Button("Red Text");
    ui::PopStyleColor();
    
    static float sx=0, sy=0;
    ui::BeginScrollArea("Scroll", 100, 100, &sx, &sy);
    ui::Button("Inside");
    ui::EndScrollArea();
    
    // 9. Phase 3: Complex
    if(ui::TreeNode("Root", nullptr)) {
        ui::Button("Child");
        ui::TreePop();
    }
    ui::SetClipboardText("Copy");
    std::string clip = ui::GetClipboardText();
    
    // 10. Phase 4: Docking
    ui::DockSpace("Dock");
    bool win_open = true;
    ui::BeginWindow("Window", &win_open);
    ui::EndWindow();
    
    return 0;
}
