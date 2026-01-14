#ifdef FANTA_USE_D3D11

#include "backend/d3d11_backend.hpp"
#include "core/types_internal.hpp"
#include "backend/drawlist.hpp"
#include <iostream>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace fanta {

// Simple Win32 Window for Preview
static float g_mouse_wheel_accum = 0; // Phase 8
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_MOUSEWHEEL) {
        g_mouse_wheel_accum += (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

D3D11Backend::D3D11Backend() {}
D3D11Backend::~D3D11Backend() { shutdown(); }

bool D3D11Backend::init(int w, int h, const char* title) {
    width = w; height = h;
    
    // 1. Create Window
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "FantaPreview";
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowA("FantaPreview", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, wc.hInstance, NULL);
        
    if (!hwnd) {
        std::cerr << "Win32 CreateWindow failed\n";
        return false;
    }
    this->hwnd = (void*)hwnd;
    
    return init_d3d((void*)hwnd);
}

bool D3D11Backend::init_d3d(void* native_window) {
    HRESULT hr;
    
    // 1. D3D11 Device & Context
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; // Required for D2D
    
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = (HWND)native_window;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
        nullptr, 0, D3D11_SDK_VERSION, &sd, &swap_chain, &d3d_device, nullptr, &d3d_context);
        
    if (FAILED(hr)) {
        std::cerr << "D3D11 Create Failed: " << std::hex << hr << "\n";
        return false;
    }
    
    // 2. D2D Factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d_factory.GetAddressOf());
    if (FAILED(hr)) return false;
    
    // 3. DirectWrite Factory
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(dwrite_factory.GetAddressOf()));
    if (FAILED(hr)) return false;
    
    create_render_target();
    return true;
}

void D3D11Backend::create_render_target() {
    ComPtr<IDXGISurface> surface;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&surface)); // Get BackBuffer
    
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    
    d2d_factory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, &d2d_target);
    d2d_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &solid_brush);
}

void D3D11Backend::shutdown() {
    // Release ComPtrs handled automatically
}

bool D3D11Backend::is_running() {
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true; // Keep running
}

void D3D11Backend::begin_frame(int w, int h) {
    if (!d2d_target) return;
    d2d_target->BeginDraw();
    d2d_target->Clear(D2D1::ColorF(0.07f, 0.07f, 0.07f)); // Dark gray bg
}

void D3D11Backend::end_frame() {
    if (!d2d_target) return;
    d2d_target->EndDraw();
    swap_chain->Present(1, 0); // VSync
}

void D3D11Backend::render(const internal::DrawList& draw_list) {
    if (!d2d_target) return;
    
    // Phase 7: Transform Stack
    std::vector<D2D1_MATRIX_3X2_F> transform_stack;
    transform_stack.push_back(D2D1::Matrix3x2F::Identity());
    
    // Ensure identity at start
    d2d_target->SetTransform(D2D1::Matrix3x2F::Identity());

    for (const auto& cmd : draw_list.commands) {
        if (cmd.type == internal::DrawCmdType::Rectangle) {
            D2D1_RECT_F r = D2D1::RectF(
                cmd.rectangle.pos_x, cmd.rectangle.pos_y,
                cmd.rectangle.pos_x + cmd.rectangle.size_x,
                cmd.rectangle.pos_y + cmd.rectangle.size_y
            );
            solid_brush->SetColor(D2D1::ColorF(
                cmd.rectangle.color_r, cmd.rectangle.color_g, cmd.rectangle.color_b, cmd.rectangle.color_a));
            d2d_target->FillRectangle(r, solid_brush.Get());
            
        } else if (cmd.type == internal::DrawCmdType::RoundedRect) {
            D2D1_ROUNDED_RECT rr;
            rr.rect = D2D1::RectF(
                cmd.rounded_rect.pos_x, cmd.rounded_rect.pos_y,
                cmd.rounded_rect.pos_x + cmd.rounded_rect.size_x,
                cmd.rounded_rect.pos_y + cmd.rounded_rect.size_y
            );
            rr.radiusX = rr.radiusY = cmd.rounded_rect.radius;
            
            solid_brush->SetColor(D2D1::ColorF(
                cmd.rounded_rect.color_r, cmd.rounded_rect.color_g, cmd.rounded_rect.color_b, cmd.rounded_rect.color_a));
            d2d_target->FillRoundedRectangle(rr, solid_brush.Get());
            
        } else if (cmd.type == internal::DrawCmdType::Circle) {
             D2D1_ELLIPSE ell;
             ell.point = D2D1::Point2F(cmd.circle.center_x, cmd.circle.center_y);
             ell.radiusX = ell.radiusY = cmd.circle.r;
             
             solid_brush->SetColor(D2D1::ColorF(
                 cmd.circle.color_r, cmd.circle.color_g, cmd.circle.color_b, cmd.circle.color_a));
             d2d_target->FillEllipse(ell, solid_brush.Get());
             
             solid_brush->SetColor(D2D1::ColorF(
                 cmd.circle.color_r, cmd.circle.color_g, cmd.circle.color_b, cmd.circle.color_a));
             d2d_target->FillEllipse(ell, solid_brush.Get());
             
        } else if (cmd.type == internal::DrawCmdType::Line) {
             D2D1_POINT_2F p0 = D2D1::Point2F(cmd.line.p0_x, cmd.line.p0_y);
             D2D1_POINT_2F p1 = D2D1::Point2F(cmd.line.p1_x, cmd.line.p1_y);
             
             solid_brush->SetColor(D2D1::ColorF(
                 cmd.line.color_r, cmd.line.color_g, cmd.line.color_b, cmd.line.color_a));
                 
             d2d_target->DrawLine(p0, p1, solid_brush.Get(), cmd.line.thickness);
             
        } else if (cmd.type == internal::DrawCmdType::Text) {
            const auto& run = draw_list.text_runs[cmd.text.run_index];
            if (run.original_text.empty()) continue;
            
            // On-the-fly layout for preview (Inefficient but fine for debugging)
            ComPtr<IDWriteTextFormat> fmt;
            dwrite_factory->CreateTextFormat(
                L"Segoe UI", NULL, 
                DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                run.font_size, L"en-us", &fmt);
            
            // This is "Preview" mode - We just dump the text at the position
            // We need the string from TextRun.
            // Requirement update: TextRun needs to store 'std::string original_text' for this backend!
            // 'gl' backend uses Quads. 'd2d' backend uses String.
            // Let's assume TextRun has it. Wait, I need to check font_types.hpp or add it.
            
            std::wstring wtext(run.original_text.begin(), run.original_text.end());
            
            D2D1_RECT_F layoutRect = D2D1::RectF(run.bounds.x, run.bounds.y, run.bounds.x + 1000, run.bounds.y + 1000); // unbounded
            
            solid_brush->SetColor(D2D1::ColorF(cmd.text.color_r, cmd.text.color_g, cmd.text.color_b, cmd.text.color_a));
            d2d_target->DrawText(
                wtext.c_str(), (UINT32)wtext.length(),
                fmt.Get(),
                layoutRect,
                solid_brush.Get()
            );
        } else if (cmd.type == internal::DrawCmdType::PushTransform) {
             D2D1_MATRIX_3X2_F current = transform_stack.back();
             // Scale then Translate
             D2D1_MATRIX_3X2_F local = D2D1::Matrix3x2F::Scale(
                 D2D1::Size(cmd.transform.scale, cmd.transform.scale), D2D1::Point2F(0,0)) *
                 D2D1::Matrix3x2F::Translation(cmd.transform.tx, cmd.transform.ty);
             
             D2D1_MATRIX_3X2_F next = local * current;
             transform_stack.push_back(next);
             d2d_target->SetTransform(next);
             
        } else if (cmd.type == internal::DrawCmdType::PopTransform) {
             if (transform_stack.size() > 1) transform_stack.pop_back();
             d2d_target->SetTransform(transform_stack.back());
             
        } else if (cmd.type == internal::DrawCmdType::Bezier) {
             solid_brush->SetColor(D2D1::ColorF(cmd.bezier.color_r, cmd.bezier.color_g, cmd.bezier.color_b, cmd.bezier.color_a));
             
             D2D1_POINT_2F p0 = {cmd.bezier.p0_x, cmd.bezier.p0_y};
             D2D1_POINT_2F p1 = {cmd.bezier.p1_x, cmd.bezier.p1_y};
             D2D1_POINT_2F p2 = {cmd.bezier.p2_x, cmd.bezier.p2_y};
             D2D1_POINT_2F p3 = {cmd.bezier.p3_x, cmd.bezier.p3_y};
             
             D2D1_POINT_2F prev = p0;
             const int segments = 20;
             for(int i=1; i<=segments; ++i) {
                 float t = i / (float)segments;
                 float u = 1.0f - t;
                 float x = u*u*u*p0.x + 3*u*u*t*p1.x + 3*u*t*t*p2.x + t*t*t*p3.x;
                 float y = u*u*u*p0.y + 3*u*u*t*p1.y + 3*u*t*t*p2.y + t*t*t*p3.y;
                 D2D1_POINT_2F next = {x, y};
                 d2d_target->DrawLine(prev, next, solid_brush.Get(), cmd.bezier.thickness);
                 prev = next;
             }
        } else if (cmd.type == internal::DrawCmdType::PushClip) {
             d2d_target->PushAxisAlignedClip(
                 D2D1::RectF(cmd.clip.x, cmd.clip.y, cmd.clip.x + cmd.clip.w, cmd.clip.y + cmd.clip.h),
                 D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
             );
        } else if (cmd.type == internal::DrawCmdType::PopClip) {
             d2d_target->PopAxisAlignedClip();
        }
    }
}

// Phase 6.1
void D3D11Backend::get_mouse_state(float& x, float& y, bool& down, float& wheel) {
    if (!hwnd) { x=0; y=0; down=false; wheel=0; return; }
    POINT p;
    if (::GetCursorPos(&p)) {
        ::ScreenToClient((HWND)hwnd, &p);
        x = (float)p.x;
        y = (float)p.y;
    } else {
        x = 0; y = 0;
    }
    // Check left mouse button
    // 0x8000 = Currently pressed
    down = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    
    wheel = g_mouse_wheel_accum;
    g_mouse_wheel_accum = 0;
} 

// Phase 5.2: Screenshot implementation
// Needs stb_image_write (assumed available from global include or needs re-include)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool D3D11Backend::capture_screenshot(const char* filename) {
    if (!swap_chain || !d3d_device || !d3d_context) return false;

    // 1. Get BackBuffer
    ComPtr<ID3D11Texture2D> back_buffer;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

    D3D11_TEXTURE2D_DESC desc;
    back_buffer->GetDesc(&desc);

    // 2. Create Staging Texture (CPU Readable)
    D3D11_TEXTURE2D_DESC staging_desc = desc;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    staging_desc.BindFlags = 0;
    staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> staging_texture;
    if (FAILED(d3d_device->CreateTexture2D(&staging_desc, nullptr, &staging_texture))) {
        std::cerr << "[Screenshot] Failed to create staging texture" << std::endl;
        return false;
    }

    // 3. Copy BackBuffer to Staging
    d3d_context->CopyResource(staging_texture.Get(), back_buffer.Get());

    // 4. Map Staging Texture
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (FAILED(d3d_context->Map(staging_texture.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
        std::cerr << "[Screenshot] Failed to map staging texture" << std::endl;
        return false;
    }

    // 5. Convert BGRA (DXGI) to RGBA (stb)
    std::vector<uint8_t> rgba_data(desc.Width * desc.Height * 4);
    uint8_t* src_row = (uint8_t*)mapped.pData;
    uint8_t* dst_row = rgba_data.data();

    // D3D11 reads top-left to bottom-right (no Y-flip needed unlike OpenGL)
    // But DXGI format is usually B8G8R8A8_UNORM
    
    for (UINT y = 0; y < desc.Height; ++y) {
        for (UINT x = 0; x < desc.Width; ++x) {
            uint8_t b = src_row[x * 4 + 0];
            uint8_t g = src_row[x * 4 + 1];
            uint8_t r = src_row[x * 4 + 2];
            uint8_t a = src_row[x * 4 + 3];

            dst_row[x * 4 + 0] = r;
            dst_row[x * 4 + 1] = g;
            dst_row[x * 4 + 2] = b;
            dst_row[x * 4 + 3] = a; // alpha 255 ideally
        }
        src_row += mapped.RowPitch;
        dst_row += desc.Width * 4;
    }

    d3d_context->Unmap(staging_texture.Get(), 0);

    // 6. Write PNG
    int result = stbi_write_png(filename, desc.Width, desc.Height, 4, rgba_data.data(), desc.Width * 4);
    
    if (result) {
        std::cout << "[Screenshot] Saved to: " << filename << " (" << desc.Width << "x" << desc.Height << ")" << std::endl;
    } else {
        std::cerr << "[Screenshot] Failed to save: " << filename << std::endl;
    }
    
    return result != 0;
}

} // namespace fanta

#endif
