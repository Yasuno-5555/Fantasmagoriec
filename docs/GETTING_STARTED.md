# Getting Started with Fantasmagorie V5 Crystal

## Prerequisites

- **Python 3.10+**: Recommended for rapid UI iteration.
- **C++20 Compiler**: MinGW (Windows), GCC, or Clang.
- **CMake 3.20+**

## 🐍 The Serpent Path (Recommended)

The fastest way to start is using the Python hot-reload environment.

1. **Build the Engine**:
```bash
mkdir build && cd build
cmake ..
mingw32-make fanta  # or make fanta
```

2. **Run the Ouroboros Runner**:
```bash
python ../src/python/hot_reload_runner.py
```

3. **Design Live**:
Open `src/python/test_ui.py` and start editing. The UI will update the moment you save!

---

## 💎 The Crystal Path (C++ Native)

For high-performance native apps, use the C++ View API.

```cpp
#include "view/api.hpp"

int main() {
    fanta::Init(1280, 800, "V5 Native");

    fanta::RunWithUI([](fanta::internal::DrawList& dl) {
        using namespace fanta::ui;
        
        // Stateless UI Definition
        auto root = Column().width(1280).height(800).bg({0.1, 0.1, 0.1, 1}).padding(20);
        {
            Text("Hello Crystal").size(40).color({1,1,1,1});
            if (Button("Interactive").radius(10).shadow(15)) {
                printf("Button Clicked!\n");
            }
        }
        End(); // Close Column
        
        // Render step
        RenderUI(root, 1280, 800, dl);
    });
}
```

## 🛠 Troubleshooting: Windows DLLs
If `fanta` fails to import in Python, ensure your MinGW `bin` directory is in the DLL search path:
```python
import os
os.add_dll_directory("C:/mingw64/bin")
```
This is handled automatically in the provided `hot_reload_runner.py`.
