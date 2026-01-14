# API Reference (fanta::Element)

This is a concise reference for the fluent Builder API used in `fanta::Element`.

## Layout & Sizing

| Method | Description |
|--------|-------------|
| `.size(w, h)` | Sets fixed width and height. |
| `.width(w)` / `.height(h)` | Sets explicit dimension. |
| `.row()` | Sets layout direction to Horizontal. |
| `.column()` | Sets layout direction to Vertical (Default). |
| `.grow(n)` | Sets Flex Grow factor (e.g., `1.0` to fill remaining space). |
| `.gap(n)` | Sets gap between children (like CSS gap). |
| `.padding(n)` | Sets uniform padding. |
| `.align(Align)` | Cross-axis alignment (`Start`, `Center`, `End`, `Stretch`). |
| `.justify(Justify)` | Main-axis alignment (`Start`, `Center`, `SpaceBetween`). |

## Styling

| Method | Description |
|--------|-------------|
| `.bg(Color)` | Sets background color. |
| `.rounded(r)` | Sets corner radius. |
| `.elevation(n)` | Sets shadow depth (0-24). |
| `.backdropBlur(sigma)` | Enables background blur (Glassmorphism). |
| `.vibrancy(amount)` | Boosts saturation of background content. |
| `.material(Type)` | Applies preset material style (`UltraThin`, `Chrome`). |

## Typography

| Method | Description |
|--------|-------------|
| `.label(text)` | Sets the text content. |
| `.color(Color)` | Sets text color. |
| `.fontSize(size)` | Sets font size in logical pixels. |
| `.textStyle(Token)` | Sets semantic style (`Title1`, `Body`, `Caption`). |

## Interaction

| Method | Description |
|--------|-------------|
| `.onClick(callback)` | Registers a click handler `std::function<void()>`. |
| `.hoverBg(Color)` | Sets background color when hovered. |
| `.activeBg(Color)` | Sets background color when pressed. |
| `.focusable(bool)` | Makes the element capable of receiving keyboard focus. |

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
