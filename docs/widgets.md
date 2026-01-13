# Widget Catalog

Fantasmagorie v2 provides a rich set of widgets, all accessible via the fluent **Builder Pattern**.

## Common Properties (`BuilderBase`)
All widgets support these methods:

| Method | Description |
|--------|-------------|
| `.width(f)`, `.height(f)` | Explicit dimensions in dps. |
| `.grow(f)` | Flexbox grow factor (default 0). |
| `.padding(f)` | Internal spacing. |
| `.id(str)` | Explicit unique ID for state persistence. |
| `.undoable()` | Enable automatic undo/redo for value changes. |
| `.script(lua)` | Attach code to be executed by the widget. |
| `.accessible(desc)` | Provide a name for screen readers. |

---

## 1. Basic Widgets

### `fanta::Label(text)`
Displays static or dynamic text.
- `.bold()`: Use bold weights.
- `.color(hex)`: Text color override.

### `fanta::Button(label)`
Standard push button.
- `.primary()`: Use theme's primary color.
- `.on_click(fn)`: Callback for click events.

### `fanta::Slider(label, val_ptr, min, max)`
Numeric input.
- `.bind(ptr)`: Bind a variable directly.
- `.range(min, max)`: Set limits.

### `fanta::Checkbox(label, bool_ptr)`
Toggle switch.
- `.bind(ptr)`: Bind a boolean variable.

---

## 2. Advanced Widgets

### `fanta::Window(title)`
Draggable, closable top-level container.
- `.size(w, h)`: Initial size.
- `.closable(bool)`: Show close button.
- `.draggable(bool)`: Allow moving.

### `fanta::Table(id)`
Data grid with columns.
- `.columns({{"Name", 150}, {"Price", 80}})`: Define headers.
- `.row_count(n)`: Number of rows.
- `.on_cell(fn)`: Logic to render individual cells.

### `fanta::TreeView(id)`
Hierarchical collapsible tree.
- `.root(node_data)`: Set the root of the tree.
- `.on_select(fn)`: Node selection callback.

### `fanta::Markdown(text)`
Rich text renderer using block-level parsing.
- Support for Headers, Lists, and Code blocks.

---

## 3. Layout Helpers

### `fanta::Row()` / `fanta::Column()`
Sets the layout direction for the current node.

### `fanta::Spacer(size = 0)`
- `size > 0`: Fixed pixel spacer.
- `size == 0`: Flexible spacer (spring) using `grow(1)`.

### `fanta::BeginPanel(id)` / `fanta::EndPanel()`
Creates a visual group with a background and rounded corners.
