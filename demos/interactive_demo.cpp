#include "view/api.hpp"
#include "core/contexts_internal.hpp"
#include <iostream>
#include <functional>
#include <string>

namespace fanta {
    void Init(int width, int height, const char* title);
    int GetScreenWidth();
    int GetScreenHeight();
    bool RunWithUI(std::function<void(::fanta::internal::DrawList&)> ui_loop);
}

int main() {
    using namespace fanta;
    using namespace fanta::ui;
    
    Init(1280, 800, "Fantasmagorie V5: Interactive Core Demo");
    
    char name_buffer[64] = "Fanta User";
    bool dark_mode = true;
    float volume = 75.0f;
    int click_count = 0;
    bool screenshot_done = false;
    
    RunWithUI([&](internal::DrawList& dl) {
        static int frame_count = 0;
        if (frame_count < 10) {
            std::cout << "[Demo] Frame " << frame_count << " running..." << std::endl;
        }
        
#ifndef NDEBUG
        if (!screenshot_done) {
            if (frame_count > 5) {
                if (internal::GetEngineContext().world.backend->capture_screenshot("interactive_demo.png")) {
                    std::cout << "[Demo] Screenshot captured to interactive_demo.png" << std::endl;
                } else {
                    std::cerr << "[Demo] Failed to capture screenshot!" << std::endl;
                }
                screenshot_done = true;
            }
        }
#endif
        frame_count++;
        auto root = Column()
            .width((float)GetScreenWidth())
            .height((float)GetScreenHeight())
            .padding(40)
            .bg({0.08f, 0.08f, 0.1f, 1.0f});
        {
            Text("Settings").size(32).color({1,1,1,1});
            
            Box().height(20); End(); // Spacer
            
            // Text Input
            Column().padding(10).bg({0.12f, 0.12f, 0.15f, 1.0f}).radius(10);
            {
                Text("Your Name:").size(14).color({0.6f, 0.6f, 0.7f, 1.0f});
                TextInput(name_buffer, 64).placeholder("Enter name...").height(40).margin(5);
            }
            End();
            
            Box().height(20); End(); // Spacer
            
            // Toggle
            Row().padding(10).bg({0.12f, 0.12f, 0.15f, 1.0f}).radius(10).height(50);
            {
                Toggle("Dark Mode", dark_mode).grow(1);
            }
            End();
            
            Box().height(20); End(); // Spacer
            
            // Slider
            Column().padding(10).bg({0.12f, 0.12f, 0.15f, 1.0f}).radius(10);
            {
                std::string vol_label = "Volume: " + std::to_string((int)volume) + "%";
                Text(AllocString(vol_label.c_str())).size(14).color({0.6f, 0.6f, 0.7f, 1.0f});
                Slider("Volume", volume).range(0.0f, 100.0f).height(40).margin(5);
            }
            End();
            
            Box().height(40); End(); // Spacer
            
            // Button
            if (Button("Save Changes").height(50).radius(10).squircle()) {
                click_count++;
                std::cout << "Saved! Count: " << click_count << " Name: " << name_buffer << std::endl;
            }
            
            Row().margin(20);
            {
                Text("Click count: ").size(16);
                Text(AllocString(std::to_string(click_count).c_str())).size(16).color({0.3f, 0.7f, 1.0f, 1.0f});
            }
            End();
        }
        End();
        
        RenderUI(root, (float)GetScreenWidth(), (float)GetScreenHeight(), dl);
    });
    
    return 0;
}
