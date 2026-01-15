# API Reference (V5 Crystal)

This reference covers the Universal Masquerade API available in both C++ and Python.

## 📐 Layout (LayoutConfig)

| Method | Description |
|--------|-------------|
| `.width(w)` / `.height(h)` | Sets explicit pixel dimension. |
| `.size(w, h)` | Sets both width and height. |
| `.grow(n)` / `.shrink(n)` | Flexbox growth/shrink factors. |
| `.padding(n)` | Sets internal padding. |
| `.margin(n)` | Sets external margin. |
| `.align(Align)` | Cross-axis alignment (`Start`, `Center`, `End`, `Stretch`). |

## 🎨 Styling (StyleConfig)

| Method | Description |
|--------|-------------|
| `.bg(Color)` | Sets background color. |
| `.color(Color)` | Sets foreground/text color. |
| `.radius(r)` | Sets corner radius (Squircle supported). |
| `.shadow(depth)` | Sets elevation depth. |
| `.blur(sigma)` | Sets backdrop blur amount. |
| `.wobble(x, y)` | Adds interactive micro-vibration. |
| `.size(fontSize)` | Sets font size (Text/Button only). |

## 📦 Widgets (View Builders)

| Function | Description |
|----------|-------------|
| `Box()` | Generic container. |
| `Row()` / `Column()` | Flex containers. |
| `Text("str")` | Text display. |
| `Button("label")` | Interactive button. returns `bool` in loop. |
| `Scroll()` | Scrollable container. |
| `Splitter(ratio)` | Resizable split container. |
| `End()` | Closes the current layout group. |

## 🐍 Python Specifics
In Python, all methods are available on the widget objects returned by the builder functions:
```python
fanta.Text("Hello").size(32).color(fanta.Color(1,1,1))
```
Interactive widgets can be checked directly:
```python
if fanta.Button("Save").bg(fanta.Color(0,1,0)):
    save_data()
```

## Node Graph

| Method | Description |
|--------|-------------|
| `.canvas(zoom, px, py)` | Marks element as an infinite pannable canvas. |
| `.node(x, y)` | Marks element as a positionable node on the canvas. |
| `.port()` | Marks element as a connection socket. |
| `.wire(...)` | Draws a Bezier wire between points. |

## Advanced

| Method | Description |
|--------|-------------|
| `.virtualList(...)` | Efficiently renders large lists. |
| `.bind(Observable)` | Two-way binding for Sliders, Toggles, and Inputs. |
| `.capture(id)` | Captures all mouse input to this element. |
