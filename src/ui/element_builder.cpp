#include "../fanta.h"
#include "../core/types_internal.hpp"
#include "../core/contexts_internal.hpp"
#include "../core/markdown.hpp"
#include <string>
#include <vector>
#include <functional>

namespace fanta {

// --- Element Handle Implementation ---
static size_t ToIndex(ElementHandle* h) { return reinterpret_cast<size_t>(h) - 1; }
static ElementHandle* ToHandle(size_t idx) { return reinterpret_cast<ElementHandle*>(idx + 1); }

// Helper to access state safely
static internal::ElementState& GetState(ElementHandle* h) {
    return internal::GetStore().frame_elements[ToIndex(h)];
}

Element::Element(const char* id_or_label) {
    auto& store = internal::GetStore();
    internal::ElementState state;
    state.id = std::hash<std::string>{}(id_or_label);
    state.label = id_or_label;
    
    // Default Styling from Theme (Phase 9)
    state.text_color = internal::GetThemeCtx().current.text;
    
    // Add to current frame
    store.frame_elements.push_back(state);
    
    // Stable Handle: Index + 1
    handle = ToHandle(store.frame_elements.size() - 1);
}

Element& Element::label(const char* text) {
    GetState(handle).label = text; return *this;
}

Element& Element::width(float w) { 
    GetState(handle).w = w; return *this; 
}
Element& Element::height(float h) { 
    GetState(handle).h = h; return *this; 
}

Element& Element::row() {
    GetState(handle).layout_mode = internal::LayoutMode::Row; return *this;
}

Element& Element::column() {
    GetState(handle).layout_mode = internal::LayoutMode::Column; return *this;
}

Element& Element::padding(float p) {
    GetState(handle).p = p; return *this;
}

Element& Element::gap(float g) {
    GetState(handle).gap = g; return *this;
}

Element& Element::blur(float radius) {
    GetState(handle).blur_radius = radius; return *this;
}
Element& Element::backdrop(bool enable) {
    GetState(handle).backdrop_blur = enable; return *this;
}
Element& Element::bg(Color c) {
    internal::ColorF cf = {c.r, c.g, c.b, c.a};
    GetState(handle).bg_color = cf; return *this;
}

Element& Element::line() {
    auto& store = internal::GetStore();
    size_t idx = ToIndex(handle);
    store.frame_elements[idx].is_line = true;
    return *this;
}

Element& Element::size(float w, float h) {
    return width(w).height(h);
}

Element& Element::color(Color c) {
    internal::ColorF cf = {c.r, c.g, c.b, c.a};
    GetState(handle).text_color = cf; return *this;
}

Element& Element::fontSize(float size) {
    GetState(handle).font_size = size; return *this;
}

Element& Element::rounded(float radius) {
    GetState(handle).corner_radius = radius; return *this;
}

Element& Element::scrollable(bool vertical, bool horizontal) {
    auto& el = GetState(handle);
    el.is_scrollable_v = vertical;
    el.is_scrollable_h = horizontal;
    return *this;
}

Element& Element::markdown(const char* md_text) {
    std::string text = md_text;
    std::vector<std::string> lines;
    size_t start = 0;
    size_t end = text.find('\n');
    while (end != std::string::npos) {
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find('\n', start);
    }
    lines.push_back(text.substr(start));

    this->column(); // Use Column to stack lines
    for (const auto& line : lines) {
        if (line.empty()) {
            Element spacer("");
            spacer.height(10);
            this->child(spacer);
            continue;
        }

        Element row_el("");
        row_el.row().height(30);

        auto spans = internal::ParseMarkdown(line);
        for (const auto& span : spans) {
            Element s(span.text.c_str());
            if (span.is_heading) {
                s.fontSize(span.heading_level == 1 ? 28 : 22);
                row_el.height(span.heading_level == 1 ? 40 : 34);
                s.color(GetTheme().text);
            } else if (span.is_link) {
                s.color(GetTheme().primary);
            } else {
                s.color(GetTheme().text);
            }
            row_el.child(s);
        }
        this->child(row_el);
    }
    return *this;
}

Element& Element::popup() {
    GetState(handle).is_popup = true; return *this;
}

Element& Element::elevation(float e) {
    GetState(handle).elevation = e; return *this;
}

Element& Element::canvas(float zoom, float pan_x, float pan_y) {
    auto& s = GetState(handle);
    s.is_canvas = true;
    s.canvas_zoom = zoom;
    s.canvas_pan = {pan_x, pan_y};
    return *this;
}

Element& Element::node(float x, float y) {
    auto& s = GetState(handle);
    s.is_node = true;
    s.node_pos = {x, y};
    return *this;
}

Element& Element::wire(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float thickness) {
    auto& s = GetState(handle);
    s.is_wire = true;
    s.wire_p0 = {x0, y0};
    s.wire_p1 = {x1, y1};
    s.wire_p2 = {x2, y2};
    s.wire_p3 = {x3, y3};
    s.wire_thickness = thickness;
    return *this;
}

Element& Element::onClick(std::function<void()> callback) {
    return *this;
}

Element& Element::child(const Element& e) {
    auto& parent_state = GetState(handle);
    auto& child_state = GetState(e.handle);
    
    parent_state.children.push_back(child_state.id);
    child_state.parent = parent_state.id;
    return *this;
}

Color Color::Hex(uint32_t c) { return {((c>>24)&0xFF)/255.f, ((c>>16)&0xFF)/255.f, ((c>>8)&0xFF)/255.f, (c&0xFF)/255.f}; }
Color Color::White() { return {1,1,1,1}; }
Color Color::Black() { return {0,0,0,1}; }
Color Color::alpha(float a) const { return {r, g, b, a}; }

Theme Theme::Dark() {
    Theme t;
    t.background = Color::Hex(0x111111FF);
    t.surface = Color::Hex(0x1A1A1AFF);
    t.primary = Color::Hex(0x3B82F6FF);
    t.accent = Color::Hex(0x10B981FF);
    t.text = Color::Hex(0xFFFFFFFF);
    t.text_muted = Color::Hex(0xBBBBBBFF);
    t.border = Color::Hex(0x333333FF);
    return t;
}

Theme Theme::Light() {
    Theme t;
    t.background = Color::Hex(0xF3F4F6FF);
    t.surface = Color::Hex(0xFFFFFFFF);
    t.primary = Color::Hex(0x2563EBFF);
    t.accent = Color::Hex(0x059669FF);
    t.text = Color::Hex(0x111827FF);
    t.text_muted = Color::Hex(0x4B5563FF);
    t.border = Color::Hex(0xE5E7EBFF);
    return t;
}

} // namespace fanta
