# Fantasmagorie

Next-generation immediate-mode UI framework for C++, designed for modularity, extensibility, and modern application development.

---

## 🚀 Fantasmagorie v3.0 (Unified)

Version 3.0 unifies the v2 UI layer and v3 rendering backend into a single, cohesive framework.

### Key Features
- **Immediate-Mode UI**: Builder Pattern API for declarative, intuitive widget construction
- **SDF Rendering**: Signed Distance Field shaders for resolution-independent shapes and text
- **FreeType Typography**: Full Unicode support including Japanese, Chinese, emoji
- **Pimpl Pattern**: Zero OpenGL/platform headers leak into public API
- **Path System**: SVG-like path builder with adaptive Bezier tessellation
- **Animation System**: Spring physics and Tween-based animations
- **Extensible**: Built-in scripting, accessibility, plugin, and profiler addons

### Quick Start
```bash
cmake -S . -B build
cmake --build build
./build/bin/Debug/fanta_demo.exe
```

### Project Structure
```
src/
├── core/          # Types, Path, Animation, Layout, NodeStore
├── render/        # PaintList, FontManager, Shaders (SDF backend)
├── platform/      # OpenGL implementation (Pimpl + GLAD)
├── shaders/       # GLSL SDF shaders
├── widgets/       # Button, Slider, Window, Table, etc.
├── a11y/          # Accessibility support
├── input/         # Gesture recognition
├── plugin/        # Dynamic plugin loading
├── profiler/      # Performance overlay
├── script/        # Embedded scripting
├── style/         # Theming
├── demos/         # Demo applications
└── *.hpp          # Facade headers
```

### Facade Headers
| Header | Description |
|--------|-------------|
| `fanta_core.hpp` | Types, Path, Animation, NodeStore, Layout |
| `fanta_render.hpp` | PaintList, FontManager, RenderBackend |
| `fanta_widgets.hpp` | Button, Slider, Window, Table, etc. |
| `fantasmagorie.hpp` | All-in-one include |

### Usage Example
```cpp
#include "fantasmagorie.hpp"

// Using v3 render backend
fanta::RenderBackend backend;
backend.init();

fanta::PaintList paint;
paint.rect({100, 100, 200, 100}, fanta::Color::Hex(0x1E1E1EFF));
paint.text({120, 150}, "Hello, Fantasmagorie!", 24.0f);

backend.execute(paint);
```

---

## 🛠️ Build

Requires **C++17** and **CMake 3.16+**.
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## ⚠️ Legacy

- v1 archived in `src/v1_archive/`
