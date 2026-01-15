# Architecture Overview

Fantasmagorie Crystal is designed around a strict separation of concerns between **State**, **Layout**, and **Rendering**.

## 1. High-Level Diagram

```mermaid
graph TD
    UserCode[User App Code] -->|Calls Fanta API| Builder[Element Builder]
    Builder -->|Writes| FrameState[Frame Elements (Transient)]
    
    subgraph Core Engine
        Scheduler[Scheduler / Loop]
        StateStore[State Store (Persistent)]
        LayoutEngine[Layout Engine]
        RenderTree[Render Tree Walker]
    end
    
    Scheduler -->|BeginFrame| Builder
    Scheduler -->|EndFrame| LayoutEngine
    
    LayoutEngine -->|Read| FrameState
    LayoutEngine -->|Compute| LayoutResults[Layout Results]
    
    RenderTree -->|Read| FrameState
    RenderTree -->|Read| LayoutResults
    RenderTree -->|Read/Write| StateStore
    RenderTree -->|Generates| DrawList[Draw Command List]
    
    DrawList -->|Submit| Backend[Rendering Backend]
    Backend -->|Draw| GPU[Screen]
```

## 2. Core Concepts

### The Stateless AST (The Lies and The Truth)
Fantasmagorie V5 separates the "Developer-facing API" (The Lie) from the "Engine-facing Data" (The Strict Truth).
- **The Lie**: `ViewBuilder` and fluent methods like `.bg()`. These are ephemeral and exist only during the UI construction phase.
- **The Truth**: `ViewHeader` and specialized View PODs. These are simple structs stored in the `FrameArena`.
- **Frame Arena**: Every frame, the entire AST is wiped and rebuilt. This eliminates "zombie" elements and state synchronization bugs.

### The Universal Masquerade
To ensure a consistent API across C++ and Python, all widgets inherit a standardized property set from the `ViewHeader` (backgrounds, shadows, corners, blurs). This allows the renderer to handle 90% of a widget's visual identity automatically.

### The ID System & Persistent State
Every element has a stable `ID`.
- This ID is used to look up **Persistent State** in `EngineContext` (scroll positions, animation timers, interaction state).
- Because the AST is stateless, state survives by being keyed to these deterministic IDs.

## 3. Python Serpent Bridge (New in V5)
V5 Crystal is a hybrid engine.
- **The Core**: High-performance C++ for layout and SDF rendering.
- **The Serpent**: A `pybind11` bridge that allows the entire UI to be defined in Python.
- **Ouroboros**: A reactive hot-reload runner that re-defines the UI tree from Python every frame, enabling instant iteration.

### Layout Engine
The layout engine runs **after** the user code has finished defining the entire tree for the frame. This allows for complex dependency resolution (though currently it's a single-pass Flexbox-like solver).

## 3. Rendering Pipeline

The rendering is deferred.
1. **User Code**: Defines geometry logic.
2. **Layout Step**: Calculates absolute positions (`Rect`).
3. **Render Step**: Walks the tree, combining Layout Rects + Style Info to generate a `DrawList`.
4. **Backend**: Takes the `DrawList` (containing platform-agnostic commands like `DrawRect`, `DrawText`) and executes them (GL/D3D).

### SDF Rendering
Unlike traditional rasterizers, Fantasmagorie uses Signed Distance Fields in the pixel shader.
- **Advantage**: Zooming in 1000x on a rounded corner keeps it perfectly sharp.
- **Advantage**: Shadows and Blurs can be computed analytically or via compute shaders.

## 4. Platform Abstraction

All OS-specific logic is hidden behind `NativePlatformBridge`.
- **Input**: GLFW (or Win32 directly).
- **Dialogs**: Blocking/Async native file pickers.
- **IME**: Text input candidates.
