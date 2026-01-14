# Fantasmagorie Crystal 💎

**Zero-Overhead Native Immediate-Mode Cross-Platform Hybrid**

Fantasmagorie Crystal is a high-performance UI engine designed to combine the **pixel-perfect rendering of Flutter** with the **extreme development speed of Immediate-Mode** and the **raw power of Native C++**.

It leverages **SDF (Signed Distance Field)** rendering to ensure your UI remains crisp at any scale, any viewport, and any DPI, without the heft of a web browser.

---

## 🚀 The Vision
- **Zero Overhead**: No Virtual Machines, no Garbage Collection, no JavaScript bridge. Just pure C++ performance.
- **Immediate Mode**: Build UI at the speed of thought. State management is implicit and trivial.
- **Pixel-Perfect**: Modern SDF-based rendering for sharp, beautiful visuals on any screen.
- **Native Hygiene**: Deep OS integration where it counts (IME, Accessibility, Native Dialogs).
- **Web & Native**: WebGPU/Vulkan/Metal backends for unified powerhouse performance.

---

## 🎨 Key Features

### 1. High-Fidelity Rendering (SDF)
- **Resolution Independent**: Rectangles, circles, and rounded corners are mathematically perfect primitives, not textured quads.
- **Glassmorphism**: Real-time highly optimized background blur (`context.backdropBlur()`) with vibrancy support.
- **Advanced Shadows**: Dual-layer shadow system (Ambient + Key) provides realistic depth (`elevation`).
- **Typography**: SDF-based text rendering ensures crisp fonts even when zoomed or transformed.

### 2. Powerful Layout Engine
- **Flexbox-Inspired**: Familiar `Row` / `Column` layout paradigms.
- **Rich Alignment**: `Start`, `Center`, `End`, `SpaceBetween` justification.
- **Virtualization**: Built-in support for massive lists (`virtualList`), rendering only what's visible.

### 3. Comprehensive Widget Suite
- **Interactive**: Sliders, Toggles, Text Input (with cursor & selection), Buttons.
- **Structural**: Groups, Tables, Scrollable Areas, Tree Nodes.
- **Navigation**: Menu Bars, Context Menus, and Popups.
- **Node Graph**: First-class support for Infinite Canvas, Nodes, Ports, and Bezier Wires.

### 4. Semantic Theming
- **Token System**: Define colors and typography continuously without hardcoding values (e.g. `Color::Accent`, `TextToken::Title1`).
- **Theme Awareness**: Instant switching between Light, Dark, High Contrast, and custom themes.
- **Material Support**: Apple-inspired materials like `UltraThin` or `Chrome`.

### 5. Input & Gestures
- **Code-Driven Input**: APIs like `IsClicked()`, `IsHovered()` work directly with the element tree.
- **Touch Ready**: Simulation for Swipe and Pinch gestures.
- **Focus Management**: Full Tab-based keyboard navigation support.

---

## 🚀 Quick Start

```cpp
#include "fanta.h"
#include <string>

int main() {
    // 1. Initialize the Engine
    fanta::Init(1280, 720, "Crystal Demo");
    
    // 2. Set a Theme (e.g., Apple Human Interface Guidelines style)
    fanta::SetTheme(fanta::Theme::AppleHIG());

    // 3. Application Loop
    fanta::Run([]() {
        // Root Container
        fanta::Element root("Root");
        root.size(1280, 720).bg(fanta::Color::SystemBackground()).row();

        // Sidebar
        {
            fanta::Element sidebar("Sidebar");
            sidebar.width(250).bg(fanta::Color::SecondarySystemBackground())
                   .padding(20).column().gap(12);

            fanta::Element title("Title");
            title.label("Fantasmagorie").textStyle(fanta::TextToken::Title1).color(fanta::Color::Label());
            sidebar.child(title);
            
            // Interactive Items
            const char* menu[] = {"Dashboard", "Settings", "Profile"};
            for(auto* val : menu) {
                 fanta::Element item(val);
                 item.height(40).label(val).textStyle(fanta::TextToken::Body)
                     .rounded(8).hoverBg(fanta::Color::Fill()) // Hover effect
                     .onClick([=](){ fanta::Native::ShowToast(val); });
                 sidebar.child(item);
            }
            root.child(sidebar);
        }

        // Main Content
        {
            fanta::Element content("Main");
            content.flexGrow(1.0f).padding(40).column().gap(20);
            
            fanta::Element h1("Header");
            h1.label("Welcome Back").textStyle(fanta::TextToken::LargeTitle);
            content.child(h1);
            
            // "Glass" Card
            fanta::Element card("Card");
            card.size(400, 200).rounded(16).elevation(10)
                .bg(fanta::Color(255, 255, 255, 20)) // Semi-transparent
                .backdropBlur(30).vibrancy(0.5)      // Glass effect
                .padding(20);
                
            card.child(fanta::Element("CardTxt").label("This is a glass card with realtime blur."));
            content.child(card);
            
            root.child(content);
        }
    });
    
    // 4. Cleanup
    fanta::Shutdown();
    return 0;
}
```

---

## 🛠️ Build Requirements

- **C++20**: Required for modern language features.
- **CMake 3.16+**: Build system.
- **Dependencies**: GLFW, FreeType (Fetched automatically via CMake/Vendor).

---

## 📂 Repository Structure

- `include/`: **Public API**. The only header you need is `fanta.h`.
- `src/core/`: **Engine Core**. State management, inputs, and animation logic.
- `src/element/`: **Layout**. Flexbox solver and widget logic builders.
- `src/backend/`: **Rendering**. Backend implementations (OpenGL, D3D11).
- `src/platform/`: **OS Bridge**. Windows/Linux/Mac specific integrations.
- `demos/`: **Examples**. Reference implementations demonstrating various features.
- `docs/`: **Documentation**. Architecture deep dives and references.
