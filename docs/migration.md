# Migration Guide: v1 to v2

Fantasmagorie v2 is a breaking rewrite. This guide helps you port your existing codebases.

## 1. Namespace Changes
All core APIs have moved from `ui::` to `fanta::`.

- **v1**: `ui::Button("Label")`
- **v2**: `fanta::Button("Label").build()` (See Builder Pattern below)

## 2. Builder Pattern vs Procedure
v1 used separate global state calls for constraints. v2 uses a fluent chaining API.

### Sizing
- **v1**: 
  ```cpp
  ui::SetNextWidth(200);
  ui::Button("Submit");
  ```
- **v2**:
  ```cpp
  fanta::Button("Submit").width(200).build();
  ```

### Callbacks
- **v1**:
  ```cpp
  if (ui::Button("Submit")) { ... }
  ```
- **v2**:
  ```cpp
  fanta::Button("Submit")
      .on_click([]{ ... })
      .build();
  ```

## 3. Render Backends
The renderer has been abstracted. If you were accessing `Renderer` directly:

- Replace `Renderer` instance with `fanta::RenderBackend*`.
- Use `fanta::OpenGLBackend` specifically if you need OpenGL ES 3.0.

## 4. Include Strategy
Instead of many small files, use the new facade headers:

- `#include "fanta_widgets.hpp"` instead of individual widget headers.
- `#include "fantasmagorie.hpp"` for a "catch-all" inclusion.

## 5. Scope Guards
`ui::StyleScope` and `ui::TransformScope` have been replaced by more integrated features in the Builder Pattern or the `PanelScope` helper.

## 6. Archived Code
If you need to peek at the old implementations, they are preserved in:
`src/v1_archive/`
