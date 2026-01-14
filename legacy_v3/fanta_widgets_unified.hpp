#pragma once
#include "fanta_core_unified.hpp"

namespace fanta {

struct WidgetCommon {
    float w=-1,h=-1,p=0;
    float blur=0; bool backdrop=false;
    ResolvedStyle style_mod;
    bool has_style=false;

    void apply(NodeHandle& n) {
        n.constraint().w = w;
        n.constraint().h = h;
        n.constraint().p = p;
        n.render().blur = blur;
        n.render().backdrop = backdrop;
        if(has_style) n.style() = style_mod;
    }
};

template<typename T>
struct Builder {
    WidgetCommon c;
    bool built=false;
    virtual ~Builder() { if(!built) static_cast<T*>(this)->build(); }
    virtual void build() = 0;
    
    T& width(float v) { c.w=v; return *static_cast<T*>(this); }
    T& height(float v) { c.h=v; return *static_cast<T*>(this); }
    T& blur(float v) { c.blur=v; return *static_cast<T*>(this); }
    T& backdrop(bool v) { c.backdrop=v; return *static_cast<T*>(this); }
    T& bg(Color col) { c.style_mod.bg=col; c.has_style=true; return *static_cast<T*>(this); }
};

struct Label : Builder<Label> {
    std::string text;
    Label(std::string t) : text(t) {}
    void build() override;
};

struct Button : Builder<Button> {
    std::string text;
    Button(std::string t) : text(t) {}
    void build() override;
};

} // namespace fanta
