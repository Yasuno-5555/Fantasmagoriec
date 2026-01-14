#pragma once

// The Single Truth.
// Fantasmagorie Crystal Architecture (v3.1)
// Thin Header. No implementation details.

#include <cstdint>
#include <functional>

namespace fanta {

// --- Opaque Handles (The PIMPL pattern) ---
struct ElementHandle; 

// --- Foundational Types (Public) ---
struct Color { 
    float r,g,b,a; 
    static Color Hex(uint32_t c); 
    static Color White();
    static Color Black();
    Color alpha(float a) const;
};

struct Theme {
    Color background;
    Color surface;
    Color primary;
    Color accent;
    Color text;
    Color text_muted;
    Color border;

    static Theme Dark();
    static Theme Light();
};

// --- The Builder API ---
struct Element {
    ElementHandle* handle = nullptr;

    Element(const char* id_or_label);
    
    // Fluent Modifiers
    Element& label(const char* text); // New: Override label
    Element& size(float w, float h);
    Element& width(float w);
    Element& height(float h);
    
    // Layout
    Element& row(); // Switch to Row Layout
    Element& column(); // Switch to Column Layout
    Element& padding(float p);
    Element& gap(float g);
    
    // Styling
    Element& bg(Color c);
    Element& color(Color c); 
    Element& fontSize(float size);
    Element& rounded(float radius);
    Element& elevation(float e);
    Element& line(); // Treat as Line primitive
    
    // Phase 7: Node Graph
    Element& canvas(float zoom, float pan_x, float pan_y);
    Element& node(float x, float y);
    Element& wire(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float thickness=2.0f);
    Element& scrollable(bool vertical=true, bool horizontal=false); // Phase 8
    Element& markdown(const char* md_text); // Phase 8
    Element& popup(); // Phase 8
    
    // Effects
    Element& blur(float radius);
    Element& backdrop(bool enable = true);

    // Interaction
    Element& onClick(std::function<void()> callback);

    // Composition
    Element& child(const Element& e);
};

// --- Global Context Control ---
void Init(int width, int height, const char* title);
void Shutdown();
void SetTheme(const Theme& theme);
Theme GetTheme();
bool Run(std::function<void()> app_loop);

// Input Helpers
void GetMousePos(float& x, float& y);
bool IsMouseDown();
bool IsMouseJustPressed();
bool IsKeyDown(int key); // ASCII for letters, VK_* for others on Windows

// Frame Control
void BeginFrame();
void EndFrame();

// Phase 5.2: Screenshot
bool CaptureScreenshot(const char* filename);

} // namespace fanta
