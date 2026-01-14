# Fantasmagorie V4.5 Dependency Graph

## Conceptual Flow
`Input` -> `fanta.cpp` -> `ElementState` -> `LayoutEngine` -> `RenderTree` -> `DrawList`

## Class Dependencies

### Global Context
- `EngineContext` (God Object)
  - `StateStore` (Data)
  - `RuntimeState` (Transient)
  - `InputContext` (Input)
  - `Backend` (Interface)

### Data Structures
- **[ElementState]** (The "Hub")
  - Used by: `LayoutEngine`, `RenderTree`, `fanta.cpp` (Builder), `StateStore`
  - Dependencies: `Theme` (for Resolving Props), `AccessibilityRole`

- **[StateStore]**
  - Owning: `std::vector<ElementState>`
  - Owning: `std::map<ID, PersistentState>`

### Subsystems

#### [LayoutEngine]
- **Input**: `std::vector<ElementState>`
- **Output**: `std::map<ID, ComputedLayout>`
- **Coupling**:
  - `TextLayout` (Structural dependency for text measurement)
  - `Theme` (via `GetThemeCtx` for fonts)

#### [RenderTree]
- **Input**:
  - `ElementState` (Tree structure)
  - `ComputedLayout` (Geometry)
  - `StateStore` (Animation/Interaction state)
  - `InputContext` (Hit testing)
- **Output**: `DrawList` (Commands)
- **Coupling**:
  - Directly modifies `StateStore.persistent_states` (Elastic scroll, Animation logic)
  - Directly reads `InputContext` (HitTest logic currently mixed in Render)

## Critical Coupling Points (Refactor Targets)
1. **RenderTree <-> StateStore**: RenderTree currently calculates physics (springs) and updates persistent state. This violates "View = Result".
   - *Fix*: Move physics to an `Update` phase before Render.
2. **ElementState**: Contains both POD data (props) and Runtime flags (is_hovered, is_active).
   - *Fix*: Split into `ViewNode` (POD) and `WidgetState` (Runtime).
3. **LayoutEngine -> TextLayout**: Hard dependency on concrete text measuring.
   - *Fix*: Abstract `MeasureText` interface.
