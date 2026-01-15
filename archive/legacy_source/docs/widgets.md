# API Reference

## PaintList (Immediate Drawing)

The core drawing API. Add commands to a list, then execute on the backend.

### Basic Drawing
```cpp
PaintList paint;
paint.rect(Rect{x, y, w, h}, Color);      // Rectangle
paint.circle(Vec2{cx, cy}, radius, Color); // Circle
paint.line(Vec2{from}, Vec2{to}, Color, thickness);
paint.text(Vec2{pos}, "string", fontSize, Color);
```

### Rectangle Properties
```cpp
auto& r = paint.rect({0, 0, 200, 100}, Color::Hex(0x1E1E1EFF));
r.radius = CornerRadius(8);              // Rounded corners
r.shadow = ElevationShadow::Level2();    // Drop shadow
r.border_color = Color::Hex(0x333333FF);
r.border_width = 1.0f;
```

### Elevation Levels
| Level | Usage |
|-------|-------|
| `Level1()` | Cards (2dp) |
| `Level2()` | Dialogs (6dp) |
| `Level3()` | Modals (12dp) |
| `Level4()` | Drawers (24dp) |

---

## Widgets (Builder Pattern)

High-level UI components from v2.

### Button
```cpp
fanta::Button("Click Me")
    .width(150)
    .primary()
    .on_click([]{ /* handler */ })
    .build();
```

### Slider
```cpp
fanta::Slider("Volume", &volume, 0, 100)
    .width(200)
    .build();
```

### Window
```cpp
fanta::Window("Settings")
    .size(400, 300)
    .closable(true)
    .children([]{ /* content */ });
```

### Table
```cpp
fanta::Table("data")
    .columns({{"Name", 150}, {"Value", 100}})
    .row_count(items.size())
    .on_cell([](int row, int col){ /* render */ });
```

---

## Animation

### Spring (Physics-based)
```cpp
AnimatedFloat value;
value.target = 1.0f;

Spring spring = Spring::wobbly();
spring.update(value, delta_time);

float current = value.current;
```

### Tween (Time-based)
```cpp
Tween tween;
tween.duration(0.5f)
     .easing(easing::ease_out_elastic)
     .on_complete([]{ /* done */ })
     .start();

float t = tween.update(dt); // 0.0 to 1.0
```

---

## Path

### Construction
```cpp
Path p;
p.move_to(0, 0)
 .line_to(100, 0)
 .quad_to({150, 50}, {100, 100})
 .cubic_to({50, 150}, {0, 100}, {0, 50})
 .close();
```

### Convenience Shapes
```cpp
Path::rounded_rect(Rect, CornerRadius);
Path::circle(center, radius);
Path::polygon(center, radius, sides);
Path::star(center, outerR, innerR, points);
```
