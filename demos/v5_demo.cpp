// V5 Demo - The Great Masquerade (Fluent API)
// Demonstrates: Fluent API, Implicit Parent Stack, Stateless AST

#include "view/api.hpp"
#include <iostream>
#include <functional>
#include <string>

// V5 Core API (minimal forward declarations)
namespace fanta {
    void Init(int width, int height, const char* title);
    int GetScreenWidth();
    int GetScreenHeight();
    bool RunWithUI(std::function<void(::fanta::internal::DrawList&)> ui_loop);
}

int main() {
    using namespace fanta;
    using namespace fanta::ui;
    
    Init(1280, 800, "Fantasmagorie V5: The Great Masquerade");
    
    int click_count = 0;
    float scroll_offset = 0;
    
    RunWithUI([&](internal::DrawList& dl) {
        // Build View AST using "The Lie" (Fluent builders + Implicit Stack)
        auto root = Column()
            .width((float)GetScreenWidth())
            .height((float)GetScreenHeight())
            .padding(20)
            .bg({0.06f, 0.06f, 0.08f, 1.0f}); 
        {
            // Floating Header (Elevated + Squircle)
            Row().height(60).padding(12).margin(8)
                .bg({0.15f, 0.18f, 0.25f, 1.0f})
                .radius(15).squircle().shadow(15);
            {
                Text("Phase C: Visual Excellence").size(24).color({0.9f, 0.95f, 1.0f, 1.0f});
            }
            End(); 
            
            // Content Area
            Scroll().grow(1).margin(10).padding(20).bg({0.08f, 0.08f, 0.1f, 1.0f}).radius(12);
            {
                // Glassy Interactive Cards
                for (int i = 0; i < 5; ++i) {
                    std::string label = "Elevated Card #" + std::to_string(i);
                    
                    Box().height(100).margin(10).padding(15)
                        .bg({0.12f, 0.12f, 0.15f, 1.0f})
                        .radius(20).squircle().shadow(10)
                        .wobble(0.02f, 0.01f); // Liquid Glass effect
                    {
                        if (Button(AllocString(label.c_str())).margin(5).height(40).squircle()) {
                            click_count++;
                        }
                        Text("This card explores SDF Squircle and 2-pass elevation.").size(12).color({0.5f, 0.5f, 0.6f, 1.0f});
                    }
                    End();
                }
            }
            End(); // Close Scroll
            
            // Footer
            Box().height(30).bg({0.05f, 0.05f, 0.06f, 1.0f});
            {
                Text("Design: The Friendly Lie -> The Strict Truth").size(12);
            }
            End();
        }
        End(); // Close Root
        
        // 2-Pass: ComputeLayout -> Render -> DrawList
        RenderUI(root, (float)GetScreenWidth(), (float)GetScreenHeight(), dl);
    });
    
    return 0;
}
