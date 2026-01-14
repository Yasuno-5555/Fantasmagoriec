# Getting Started with Fantasmagorie Crystal

## Prerequisites

- **Operaing System**: Windows 10/11, macOS 12+, or Modern Linux (Ubuntu 22.04+).
- **Compiler**: C++20 compliant compiler.
    - Windows: MSVC (Visual Studio 2022 v17.0+)
    - Linux: GCC 11+ or Clang 14+
    - macOS: Apple Clang 14+ (Xcode 14)
- **Build System**: CMake 3.20+
- **Tools**: Git

## 1. Cloning the Repository

```bash
git clone --recursive https://github.com/Yasuno-5555/Fantasmagoriec.git
cd Fantasmagoriec
```

> **Note**: The `--recursive` flag is critical to pull in dependencies like GLFW and pure-cpp-math libraries.

## 2. Building

Fantasmagorie works best with standard CMake workflows.

### Windows (Visual Studio)

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux / macOS (Unix Makefiles)

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

## 3. Creating Your First App

Create a new `.cpp` file (e.g., `myapp.cpp`) and link it against `fanta`:

```cpp
#include "fanta.h"

int main() {
    // 1024x768 window titled "Hello"
    fanta::Init(1024, 768, "Hello");
    fanta::SetTheme(fanta::Theme::Dark());

    fanta::Run([]() {
        fanta::Element root("root");
        root.size(1024, 768).bg(fanta::Color::SystemBackground());
        
        fanta::Element text("msg");
        text.label("Hello World").color(fanta::Color::Label()).fontSize(32);
        
        root.child(text);
    });

    fanta::Shutdown();
}
```

## 4. Running Demos

The project comes with several demos to showcase capabilities:

- **Rich Demo** (`rich_demo`): Showcases specialized UI cards, Glassmorphism, and Themes.
- **Wire Demo** (`wire_demo`): Demonstrates the Node Graph and Wire connection system.
