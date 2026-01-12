# Fantasmagorie v2

Next-generation immediate-mode UI framework for C++, designed for modularity, extensibility, and modern application development.

## 🚀 Overview

Fantasmagorie v2 is a complete rewrite of the original framework, adopting a strictly modular architecture while retaining the **Structure of Arrays (SoA)** data model for performance. It introduces a fluent **Builder Pattern API**, integrated scripting, accessibility support, and a plugin system.

## ✨ Key Features

### Core & Architecture
- **SoA Data Model**: High-performance component-based storage (No fat `Widget` objects).
- **Flexbox Layout**: Native support for Row/Column, Grow/Shrink, and alignment.
- **Builder Pattern API**: Fluent, declarative syntax for UI construction.
- **Modular Design**: Zero coupling between core, renderer, and platform layers.

### Advanced Capabilities (Killer Features)
- **📜 Scripting**: Built-in lightweight Lua-like script engine for logic embedding.
- **♿ Accessibility**: First-class support for Screen Readers, Keyboard Nav, and High Contrast.
- **🔌 Plugins**: Dynamic loading of Widgets and Themes via DLL/dylib/.so.
- **📊 Profiler**: Integrated frame timing and performance bottleneck detection.
- **📱 Mobile Ready**: Touch gesture recognition (Tap, Swipe, Pinch) and DPI-aware scaling.

## 📦 Modules (`src/v2/`)

| Module | Description |
|--------|-------------|
| `core` | Types, NodeStore, Layout Engine, UIContext, Undo/Redo |
| `widgets` | Button, Label, Slider, Window, Table, Tree, Markdown |
| `render` | Abstract Backend, DrawList (Layers/Clipping), OpenGL ES 3.0 |
| `style` | Theme System (Minimal, Material, Glass), System Sync |
| `platform` | Native Integration (Dialogs, Clipboard) & Mobile Utils |
| `script` | Embedded Script Engine |
| `a11y` | Accessibility & Focus Management |
| `plugin` | Dynamic Library Loader |
| `input` | Gesture Recognition |

## 💻 Usage Example

```cpp
#include "fantasmagorie.hpp"

// Fluent Builder API
fanta::Window("Settings")
    .size(400, 300)
    .draggable()
    .children([]{
        
        fanta::Row().children([]{
            fanta::Label("Volume").bold().build();
            fanta::Slider("vol", &volume, 0, 100)
                .accessible("Master Volume Control")
                .undoable()
                .build();
        });

        fanta::Button("Save Configuration")
            .primary()
            .script("print('Config saved')")
            .on_click([]{ save_config(); })
            .build();
            
        // Embed Markdown Content
        fanta::Markdown(
            "# Release Notes\n"
            "- Added **bold** text support\n"
            "- Fixed layout bugs"
        ).build();
    });
```

## 🛠️ Build

v2 is header-heavy but requires C++17.
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## ⚠️ Legacy Code

The original v1 codebase has been archived to `src/v1_archive/` for reference. All active development happens in `src/v2/`.
