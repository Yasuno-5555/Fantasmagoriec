// Fantasmagorie v3 - Unified Widgets (Rich Builder API)
#pragma once
#include "fanta_core_unified.hpp"

namespace fanta {

struct WidgetCommon {
    float w=-1, h=-1, p=0, g=0;
    float blur_r=0; bool backdrop=false;
    ResolvedStyle custom_style;
    bool has_custom_style = false;

    void apply(NodeHandle& n) const {
        auto& c = n.constraint();
        c.width = w; c.height = h; c.padding = p; c.gap = g;
        auto& r = n.render();
        r.blur_radius = blur_r; r.backdrop_blur = backdrop;
        if (has_custom_style) n.style() = custom_style;
    }
};

template<typename T>
class Builder {
protected:
    WidgetCommon common;
    bool built = false;
public:
    virtual ~Builder() { if(!built) static_cast<T*>(this)->build(); }
    
    // Standard Constraints
    T& width(float v) { common.w=v; return *static_cast<T*>(this); }
    T& height(float v) { common.h=v; return *static_cast<T*>(this); }
    T& padding(float v) { common.p=v; return *static_cast<T*>(this); }
    
    // Composite Effects (Phase 6)
    T& blur(float r) { common.blur_r=r; return *static_cast<T*>(this); }
    T& backdrop(bool b=true) { common.backdrop=b; return *static_cast<T*>(this); }
    
    // Rich Styling (Deep Operation)
    T& bg(Color c) { common.custom_style.bg=c; common.has_custom_style=true; return *static_cast<T*>(this); }
    T& radius(CornerRadius r) { common.custom_style.radius=r; common.has_custom_style=true; return *static_cast<T*>(this); }
    T& shadow(Shadow s) { common.custom_style.shadow=s; common.has_custom_style=true; return *static_cast<T*>(this); }
    T& border(Color c, float w=1.0f) { common.custom_style.border_color=c; common.custom_style.border_width=w; common.has_custom_style=true; return *static_cast<T*>(this); }

    virtual void build() = 0;
};

// --- Standard Widgets ---

class Label : public Builder<Label> {
    std::string text; bool bold=false;
public:
    Label(std::string t) : text(t) {}
    Label& is_bold(bool b=true) { bold=b; return *this; }
    void build() override;
};

class Button : public Builder<Button> {
    std::string text; std::function<void()> click_fn;
public:
    Button(std::string t) : text(t) {}
    Button& on_click(std::function<void()> f) { click_fn=f; return *this; }
    void build() override;
};

class Slider : public Builder<Slider> {
    std::string label; float* val; float min_v, max_v;
public:
    Slider(std::string l, float* v, float mi, float ma) : label(l), val(v), min_v(mi), max_v(ma) {}
    void build() override;
};

}
