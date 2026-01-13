# Fantasmagorie

Next-generation immediate-mode UI framework for C++, designed for modularity, extensibility, and modern application development.

---

## 🆕 Fantasmagorie v3 (Latest)

**v3** is the production-ready rendering backend with strict header separation and commercial-grade architecture.

### Key Features
- **Pimpl Pattern**: Zero OpenGL/platform headers leak into public API
- **GLAD Loader**: Modern OpenGL function loading
- **SDF Rendering**: Signed Distance Field shaders for resolution-independent shapes and text
- **FreeType Typography**: Full Unicode support including Japanese, Chinese, emoji
- **Triple VBO Buffering**: GPU-safe text rendering with buffer orphaning

### Quick Start
```bash
cmake -S . -B build
cmake --build build
./build/bin/Debug/fanta_v3_demo.exe
```

### Demo Output
- 10,000 small rectangles (stress test grid)
- Rounded rectangle with elevation shadow
- Multi-language text (English + 日本語)
- Real-time glyph atlas statistics

### v3 File Structure
```
src/v3/
├── core/          # Types, fonts, shaders, text
├── platform/      # OpenGL implementation (Pimpl)
│   └── glad/      # GLAD loader
├── shaders/       # GLSL SDF shaders
└── demo.cpp       # Demonstration app
```

### Iron Rules (Header Safety)
1. **No `windows.h`/`glad.h`/`glfw3.h` in `.hpp` files**
2. Use `#define NOMINMAX` at top of `.cpp` files
3. Use `#undef` guards for common Windows macros
4. Forward declarations + Pimpl for platform types

---

## Fantasmagorie v2

The immediate-mode UI layer with Builder Pattern API.

### Features
- **SoA Data Model**: High-performance component storage
- **Flexbox Layout**: Row/Column, Grow/Shrink, alignment
- **Builder Pattern API**: Fluent, declarative syntax
- **Scripting**: Embedded Lua-like engine
- **Accessibility**: Screen reader, keyboard nav support

### Facade Headers
| Header | Description |
|--------|-------------|
| `fanta_core.hpp` | Types, NodeStore, Layout, UIContext |
| `fanta_widgets.hpp` | Button, Label, Slider, Window, etc. |
| `fanta_render.hpp` | DrawList, OpenGL backend |
| `fantasmagorie.hpp` | All-in-one include |

### Usage
```cpp
#include "fantasmagorie.hpp"

fanta::Window("Settings")
    .size(400, 300)
    .children([]{
        fanta::Label("Volume").bold().build();
        fanta::Slider("vol", &volume, 0, 100).build();
    });
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
- v2 active in `src/v2/`
- **v3 is the recommended backend** (`src/v3/`)
