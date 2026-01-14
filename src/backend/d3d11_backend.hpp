#pragma once

#ifdef FANTA_USE_D3D11

// --- Standard Headers First ---
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cmath>

// Windows SDK headers MUST come first to avoid macro conflicts with GLFW
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wrl/client.h>

// Project headers after Windows headers
#include "backend/backend.hpp"

namespace fanta {

using Microsoft::WRL::ComPtr;

class D3D11Backend : public Backend {
    // Window Handle (HWND)
    void* hwnd = nullptr; 

    // D3D11 Resources
    ComPtr<ID3D11Device> d3d_device;
    ComPtr<ID3D11DeviceContext> d3d_context;
    ComPtr<IDXGISwapChain> swap_chain;
    
    // D2D Resources (Interop)
    ComPtr<ID2D1Factory> d2d_factory;
    ComPtr<ID2D1RenderTarget> d2d_target;
    ComPtr<ID2D1SolidColorBrush> solid_brush;
    
    // DirectWrite
    ComPtr<IDWriteFactory> dwrite_factory;

    // State
    int width = 0;
    int height = 0;

    bool init_d3d(void* native_window);
    void cleanup_d3d();
    void create_render_target();

public:
    D3D11Backend();
    ~D3D11Backend();

    bool init(int w, int h, const char* title) override;
    void shutdown() override;
    bool is_running() override; // Removed const to match Backend interface
    void begin_frame(int w, int h) override;
    void end_frame() override;
    void render(const internal::DrawList& draw_list) override;
    void get_mouse_state(float& x, float& y, bool& down) override;
    Capabilities capabilities() const override { return { false }; } // No native blur yet
    
    // Phase 5.2: Screenshot capture
    bool capture_screenshot(const char* filename);
};

} // namespace fanta

#endif // FANTA_USE_D3D11
