# Scripting & Advanced Systems

Fantasmagorie v2 is more than just raw UI; it includes integrated systems for logic, introspection, and community expansion.

## 📜 Script Engine

Widgets can have logic attached using a lightweight, Lua-like syntax.

### Basic Syntax
- **Function Calls**: `print('Hello', 123)`
- **Variables**: `my_var = 10`
- **Events**: Widgets can trigger scripts on interaction.

### Usage in C++
```cpp
fanta::Button("Run")
    .script("print('Executing simulation'); notify_success()")
    .build();
```

---

## ♿ Accessibility (A11y)

v2 is designed to be inclusive from the ground up.

### Key Features
- **Builder Integration**: Attach semantic descriptions with `.accessible("Click to save")`.
- **Keyboard Navigation**: Automatic Tab index calculation and focus ring rendering.
- **Screen Reader Hooks**: Use `fanta::a11y::announce(text)` to speak to the user.
- **High Contrast**: Automatic theme adjustment when the system high contrast mode is detected.

---

## 📊 Performance Profiler

Introspect your UI performance in real-time.

```cpp
#include "fanta_addons.hpp"

// Enable the overlay
fanta::profiler::enable();
```

### Metrics Tracked:
- **Frame Time**: Total time to process a frame.
- **Layout/Render breakdown**: Identifying where the bottleneck lies.
- **Widget Count**: Number of active nodes.
- **Memory Usage**: Current allocation for the NodeStore.

---

## 🔌 Plugin System

Extend the framework without recompiling the main application.

### Loading Plugins
```cpp
fanta::plugin::load("my_custom_theme.dll");
```

### Creating a Plugin:
Plugins must export `fanta_plugin_info` and `fanta_plugin_register`. They can register new widget factories or custom themes to the global registry.
