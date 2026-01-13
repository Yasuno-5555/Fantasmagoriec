# Architecture Deep Dive

Fantasmagorie v2 is built on a performance-first, decoupled architecture. Unlike traditional OO UI frameworks, it treats UI elements as sets of components.

## 1. Structure of Arrays (SoA) Model

The core of the framework is the `NodeStore`. Instead of a single `Widget` class with dozens of members, data is partitioned into parallel arrays:

```cpp
struct NodeStore {
    std::unordered_map<NodeID, NodeTree>          tree;        // Parent/Child refs
    std::unordered_map<NodeID, LayoutConstraints> constraints; // Input for layout
    std::unordered_map<NodeID, LayoutData>        layout;      // Output of layout
    std::unordered_map<NodeID, ResolvedStyle>     style;       // Colors, borders
    std::unordered_map<NodeID, InputState>        input;       // Click/Hover flags
    // ... other component streams
};
```

### Why SoA?
- **Cache Efficiency**: The Layout Engine only touches `constraints` and `layout`, keeping the CPU cache warm with relevant data.
- **De-coupling**: You can write a new Renderer that only reads `layout` and `style` without knowing about the input logic.
- **Transient vs Persistent**: Some data (like `layout`) is recalculated every frame, while others (like `scroll_offset`) persist.

## 2. Frame Lifecycle

A typical frame in Fantasmagorie v2 flows through 4 distinct phases:

1. **Input Phase**: OS events (Mouse/Touch) are mapped to `NodeID`s based on the previous frame's layout results.
2. **Build Phase**: The user code runs. `fanta::Button()...` calls create or retrieve nodes and update their `constraints` and `style`.
3. **Layout Phase**: The `LayoutEngine` solves the Flexbox constraints for all active nodes.
4. **Render Phase**: The `DrawList` is populated by iterating through resolved nodes and converted into GPU commands.

## 3. NodeID and Reification

Nodes are identified by a 64-bit `NodeID`. By default, these IDs are generated based on the string labels or explicit IDs provided in the Builder API.
This allows widgets to "reify" across frames—finding their persistent state (like scroll position) even as the hierarchy is built dynamically.

## 4. Coordinate Systems

- **Local Coordinates**: Every node operates in its own coordinate space `(0, 0)` at its top-left.
- **Global Coordinates**: The final screen position after applying the parent stack's transforms.
- **DPI Scaling**: The platform layer provides a scale factor. All builder units are in "logical pixels" (dp), which are converted to physical pixels before rendering.
