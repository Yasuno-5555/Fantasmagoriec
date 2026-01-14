# Fantasmagorie Crystal v3.3

**Crystal Architecture**: A state-of-the-art C++ immediate-mode UI framework designed for high-performance DSP software, node-based editors, and polished creative tools.

---

## ✨ Principles

Fantasmagorie Crystal follows a strict architectural protocol:
- **Thin Header**: `fanta.h` is a minimal, Pimpl-based interface. Zero third-party or platform headers leak into your project.
- **State Separation**: UI logic, layout, and cross-platform rendering are strictly decoupled.
- **Durable Identity**: Widgets maintain persistent state (animations, scroll positions) across frames using a unique ID-based `StateStore`.

---

## 🎨 Key Features

### 1. High-Performance Rendering
- **Dual Backends**: Native support for **OpenGL** and **Direct3D 11 (Direct2D)**.
- **SDF-Text**: Resolution-independent typography powered by FreeType and Signed Distance Fields.
- **Vector Primitives**: High-quality rounded rectangles, anti-aliased circles, and Bezier curves.

### 2. Interaction & Animation
- **Spring Physics**: Native bounciness and organic feedback for all interactive elements (Elevation, Scaling).
- **Advanced Input**: Mouse wheel integration, keyboard state tracking, and sub-pixel mouse positioning.

### 3. Restoration Suite (Legacy Parity)
- **Node Graph**: Declarative Canvas API with Pan/Zoom support and Bezier wire drawing.
- **Markdown**: Built-in parser and renderer for rich documentation within your UI.
- **Scrollable Containers**: Clipping-aware scroll areas with inertial momentum.
- **Modal Popups**: Comprehensive overlay pass for menus, dialogs, and tooltips.

### 4. Semantic Theming
- One-click toggling between **Dark** and **Light** modes.
- Role-based colors (`primary`, `surface`, `accent`) ensure visual consistency across the entire application.

---

## 🚀 Quick Start

```cpp
#include "fanta.h"

int main() {
    fanta::Init(1280, 720, "My App");
    
    fanta::Run([]() {
        fanta::Element root("Main");
        root.size(1280, 720).bg(fanta::GetTheme().background).padding(40);
        {
            fanta::Element title("Title");
            title.label("Hello Crystal!").fontSize(32).color(fanta::GetTheme().primary);
            root.child(title);
            
            fanta::Element btn("ClickMe");
            btn.width(200).height(50).bg(fanta::GetTheme().surface).rounded(8).elevation(4)
               .label("PRESS ME");
            root.child(btn);
        }
    });
    
    fanta::Shutdown();
    return 0;
}
```

---

## 🛠️ Build Requirements

- **C++20** (for design patterns and performance)
- **CMake 3.16+**
- **Dependencies**: GLFW, FreeType (managed via CMake)

---

## ⚠️ Repository Structure

- `include/`: Public facade headers.
- `src/core/`: Internal types, animation solvers, and input management.
- `src/backend/`: Rendering implementation detail (OpenGL/D3D11).
- `src/element/`: Layout engine and element lifecycle.
- `demos/`: Verification and reference implementations.
