# Fantasmagorie V5 Crystal: The Great Masquerade 💎🐍

Fantasmagorie V5 is a high-performance, stateless UI framework for C++ and Python. It combines the precision of an SDF rendering engine with the flexibility of a reactive Python development environment.

## 💎 Core Architecture: Stateless Excellence
V5 operates on the principle of a **Stateless AST**. Every frame, the UI tree is reconstructed from scratch (using a high-speed `FrameArena`).
- **Philosophy I**: The Lie (Complex Builders) → The Strict Truth (Simple POD AST).
- **Philosophy II**: Build-Time Recursion, Render-Time Precision.
- **Philosophy III**: State lives in C++ (PersistentStore), Logic lives in Python.

## 🎨 The Universal Masquerade (Fluent API)
Thanks to a unified `ViewHeader`, every widget in the engine supports a consistent fluent API:
```cpp
// C++
Text("Hello").bg(Color::Red()).shadow(10).radius(5).wobble(0.1, 0.1);
```
```python
# Python
fanta.Button("Click Me").width(200).height(50).radius(10).bg(fanta.Color(0,0.5,1))
```
- **Supported Visuals**: Shadows (Elevation), Squircle corners, Backdrop Blur, Background Colors, Padding/Margin, and Flex-grow.

## 🐍 Project "PyFanta" (The Serpent)
A first-class Python bridge via `pybind11`.
- **Hybrid Power**: Write your UI in Python, render it with C++ OpenGL at 60+ FPS.
- **Ouroboros Hot-Reload**: Save your Python code in VS Code and watch the UI update instantly without restarting the engine.
- **Crash-Proof**: Python errors are caught and rendered as a high-fidelity "Red Screen of Death" directly in the engine, keeping the C++ backend alive.

## 🛠️ Pro Debugging Infrastructure
Built-in tools for precision UI engineering:
- **Step Mode (F5/F6)**: Pause the engine and step frame-by-frame.
- **Hit-Test Visualizer**: Live magenta frames highlight exactly which element is hovered.
- **Rainbow Borders**: Color-coded nesting depth for instant layout structural analysis.
- **State Dump**: Full engine state export via JSON.

## 🚀 Quick Start (Python)
Navigate to the `build/` directory and run:
```powershell
python ../src/python/hot_reload_runner.py
```
Then edit `src/python/test_ui.py` to see the magic.

---
*Design: The Friendly Lie -> The Strict Truth.*
